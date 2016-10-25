#pragma once

#include <unordered_map>
#include <concurrent_unordered_set.h>
#include <thread>
#include <mutex>

#include <wrl.h>

#include <boost/preprocessor/repeat.hpp>

#pragma comment(lib, "rpcrt4.lib")

using namespace Microsoft::WRL;

namespace Hooking::COM
{
	namespace Detail
	{
		// dummy type to allow reinterpret_cast to work in all cases
		struct DummyType
		{

		};

		struct VmtIndexStub
		{
#define VMT_IMPL_INDEX_FUNC(_, idx, __) virtual int __stdcall GetIndex ## idx(DummyType) { return idx; }
			BOOST_PP_REPEAT(200, VMT_IMPL_INDEX_FUNC, _)
#undef VMT_IMPL_INDEX_FUNC
		};

		template<typename T>
		struct VmtLengthStub : public T
		{
		public:
			virtual void __stdcall __FinalFunctionVmt() {}
		};

		template<class T> struct GetClass_i;

		template<class T, class R>
		struct GetClass_i<R T::*>
		{
			using Type = T;
		};

		template<class T>
		using MemberFunctionType = typename GetClass_i<T>::Type;
	}

	// inspired by http://stackoverflow.com/questions/5635212/detect-the-the-vtable-index-ordinal-of-a-specific-virtual-function-using-visual
	template<typename TFunc>
	int GetVmtOffset(TFunc func)
	{
		using TClass = Detail::MemberFunctionType<TFunc>;
		using TStub = int(__stdcall TClass::*)(Detail::DummyType);

		// instantiate the stub
		static Detail::VmtIndexStub indexingStub;

		// cast the stub to the required type
		TClass* vtInstance = reinterpret_cast<TClass*>(&indexingStub);

		// make the function pointer into a stub with the same-ish offset
		TStub stub = reinterpret_cast<TStub>(func);

		// and invoke the stub
		return (vtInstance->*stub)(Detail::DummyType());
	}

	template<typename T>
	int GetVmtSize()
	{
		return GetVmtOffset(&Detail::VmtLengthStub<T>::__FinalFunctionVmt);
	}

	template<typename TInterface>
	inline void*** GetObjectVmtReference(IUnknown* object)
	{
		ComPtr<IUnknown> objPtr(object);
		ComPtr<TInterface> intPtr;

		if (SUCCEEDED(objPtr.As(&intPtr)))
		{
			return (void***)intPtr.Get();
		}

		{
			GUID interfaceGuid = __uuidof(TInterface);

			RPC_CSTR interfaceGuidString = nullptr;
			UuidToStringA(&interfaceGuid, &interfaceGuidString);

			FuncTrace("Could not get object VMT for object %p interface %s.\n", object, interfaceGuidString);

			RpcStringFreeA(&interfaceGuidString);
		}

		return nullptr;
	}

	template<typename TInterface>
	inline void** GetObjectVmt(IUnknown* object)
	{
		auto vmtRef = GetObjectVmtReference<TInterface>(object);

		if (vmtRef)
		{
			return *vmtRef;
		}

		return nullptr;
	}

	template<typename TInterface>
	inline bool SetObjectVmt(IUnknown* object, void** vmt)
	{
		auto vmtRef = GetObjectVmtReference<TInterface>(object);

		if (vmtRef)
		{
			*vmtRef = vmt;
			return true;
		}

		return false;
	}

	template<typename TInterface>
	inline void** EnsureCustomVmt(IUnknown* object)
	{
		static concurrency::concurrent_unordered_set<void**> objectMap;

		// get the object VMT
		void** vmt = GetObjectVmt<TInterface>(object);

		// sanity check
		if (!vmt)
		{
			return nullptr;
		}

		// if the object VMT is already custom, return that one
		if (objectMap.find(vmt) != objectMap.end())
		{
			return vmt;
		}

		// if not, create a new VMT and copy the existing VMT
		// TODO: clean up on destruction?
		int vmtSize = GetVmtSize<TInterface>() * 16;

		void** newVmt = new void*[vmtSize]; // * 16 is a fudge in case the implementation uses the same VMT as the interface
		memcpy(newVmt, vmt, vmtSize * sizeof(void*));

		// replace the object VMT
		SetObjectVmt<TInterface>(object, newVmt);

		// add the new VMT to the object table
		objectMap.insert(newVmt);

		// and return the new VMT
		return newVmt;
	}

	class __declspec(uuid("F1326FC3-8E5E-450A-ACD1-4E796BA127CE")) IRefHolder : public IUnknown
	{
	public:
		STDMETHOD(GetRefHolder)(void** refHolder) = 0;
	};

	template<typename T>
	struct RefContainer
	{
		class RefHolder : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IRefHolder>
		{
		private:
			T m_refData;

		public:
			RefHolder()
			{
				hadRef = true;
			}

			RefHolder(const T& data)
				: m_refData(data)
			{
				hadRef = true;
			}

			RefHolder(T&& data)
				: m_refData(std::move(data))
			{
				hadRef = true;
			}

			T& GetData()
			{
				return m_refData;
			}

			const T& GetData() const
			{
				return m_refData;
			}

			STDMETHOD(GetRefHolder)(void** refHolder) override
			{
				*refHolder = &m_refData;
				return S_OK;
			}

			bool hadRef;
		};

		struct RefMetaData
		{
			RefContainer container;
			HRESULT(__stdcall* queryInterface)(IUnknown* self, REFIID riid, void** pObject);
			ULONG(__stdcall* addRef)(IUnknown* self);
			ULONG(__stdcall* release)(IUnknown* self);

			RefMetaData()
			{

			}

			RefMetaData(const RefContainer& container)
				: container(container)
			{

			}
		};

		static RefMetaData* GetMetaData(void* object, RefContainer* baseContainer = nullptr)
		{
			static std::unordered_map<void*, RefMetaData> metaDataMap;
			static std::mutex mutex;

			std::unique_lock<std::mutex> lock(mutex);

			auto it = metaDataMap.find(object);

			if (it == metaDataMap.end())
			{
				if (baseContainer)
				{
					it = metaDataMap.insert({ object, RefMetaData(*baseContainer) }).first;
				}
				else
				{
					it = metaDataMap.insert({ object, RefMetaData() }).first;
				}
			}

			return &it->second;
		}

		static HRESULT __stdcall QueryInterfaceStub(IUnknown* self, REFIID riid, void** pObject)
		{
			auto metaData = GetMetaData(self);

			HRESULT hr = metaData->queryInterface(self, riid, pObject);

			if (SUCCEEDED(hr))
			{
				if (!metaData->container.IsAttached((IUnknown*)*pObject))
				{
					metaData->container.Attach((IUnknown*)*pObject);
				}
				
				// AddRef on the meta object - this needs two as some callers are.. weird
				auto metaData = GetMetaData(*pObject);
				auto rc = metaData->container.m_holder->AddRef();
				rc = metaData->container.m_holder->AddRef();
			}

			return hr;
		}

		static ULONG __stdcall AddRefStub(IUnknown* self)
		{
			auto metaData = GetMetaData(self);
			auto rc = metaData->container.m_holder->AddRef();

			return metaData->addRef(self);
		}

		static ULONG __stdcall ReleaseStub(IUnknown* self)
		{
			auto metaData = GetMetaData(self);
			auto rc = metaData->container.m_holder->Release();

			return metaData->release(self);
		}

		RefContainer()
		{
			auto holder = Make<RefHolder>();
			holder.CopyTo(&m_holder);
		}

		RefContainer(const T& data)
		{
			auto holder = Make<RefHolder>(data);
			holder.CopyTo(&m_holder);
		}

		RefContainer(T&& data)
		{
			auto holder = Make<RefHolder>(std::move(data));
			holder.CopyTo(&m_holder);
		}

		RefContainer(const RefContainer& container)
		{
			m_holder = container.m_holder;
		}

		RefContainer(IUnknown* object)
			: RefContainer(GetMetaData(object)->container)
		{

		}

		template<typename TInterface = IUnknown>
		void Attach(IUnknown* object)
		{
			RefMetaData* metaData = GetMetaData(object, this);

			void** vmt = EnsureCustomVmt<TInterface>(object);

			if (vmt[0] != QueryInterfaceStub)
			{
				metaData->queryInterface = (decltype(metaData->queryInterface))vmt[0];
				metaData->addRef = (decltype(metaData->addRef))vmt[1];
				metaData->release = (decltype(metaData->release))vmt[2];
			}

			vmt[0] = QueryInterfaceStub;
			vmt[1] = AddRefStub;
			vmt[2] = ReleaseStub;

			if (m_holder->hadRef)
			{
				m_holder->AddRef();
				m_holder->hadRef = false;
			}
		}

		bool IsAttached(IUnknown* object)
		{
			return (**(void***)object == QueryInterfaceStub);
		}

		T& Get()
		{
			return m_holder->GetData();
		}

		const T& Get() const
		{
			return m_holder->GetData();
		}

	private:
		RefHolder* m_holder;
	};

	template<typename TFunc, typename TRawPtr = void*>
	bool ReplaceVmtFunction(IUnknown* object, TFunc func, TRawPtr replacementFunction, TRawPtr* origFunction = nullptr)
	{
		// make sure a custom VMT is created
		void** vmt = EnsureCustomVmt<Detail::MemberFunctionType<TFunc>>(object);

		if (!vmt)
		{
			return false;
		}

		// get the offset of the function to replace
		int vmtOffset = GetVmtOffset(func);

		// save the original function if need be
		if (origFunction)
		{
			*origFunction = (decltype(*origFunction))vmt[vmtOffset];
		}

		// set the new function
		vmt[vmtOffset] = replacementFunction;

		// return a success condition
		return true;
	}
}