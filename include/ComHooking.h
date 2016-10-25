#pragma once

#include <wrl.h>

#include <boost/preprocessor/repeat.hpp>

#include <concurrent_unordered_set.h>

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