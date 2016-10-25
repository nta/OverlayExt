#pragma once

//
// The base interfaces for a hooking backend.
//
namespace Hooking
{
	namespace Internal
	{
		//
		// Example composition tree:
		// AyriaPluginHookBackend <- IHookBackend
		//                        <- MinHookTrampolineBackend <- IHookTrampolineBackend
		//                        <- Win32HookBackend         <- IHookAPIBackend
		//                                                    <- IHookBranchBackend
		//

		class IHookBackend
		{
		public:
			//
			// Destructor.
			//
			virtual ~IHookBackend() = default;

			//
			// Gets the name of the final composed backend service.
			//
			virtual const char* GetName() = 0;
		};

		class IHookAPIBackend
		{
		public:
			//
			// Destructor.
			//
			virtual ~IHookAPIBackend() = default;

			//
			// Backend function for `Hooking::LookupAPIStart`.
			// This is expected to load the module if not loaded already.
			//
			virtual void* LookupAPIStart(const char* moduleName, const char* functionName) = 0;
		};

		class IHookTrampolineBackend
		{
		public:
			//
			// Destructor.
			//
			virtual ~IHookTrampolineBackend() = default;

			//
			// Backend function for `Hooking::InstallTrampolineHook`.
			//
			virtual bool InstallTrampolineHook(void* hookAddress, void* replacementFunction, void** originalFunction) = 0;
		};
	}
}