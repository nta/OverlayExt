#pragma once

//
// A platform-independent API for hooking - frontend component.
//
namespace Hooking
{
	namespace Internal
	{
		class IHookBackend;
	}

	//
	// Initializes the backend for the hooking service.
	//
	bool InitializeHooking(Internal::IHookBackend* backend = nullptr);

	//
	// Gets the default hooking backend.
	//
	Internal::IHookBackend* GetDefaultBackend();

	//
	// Looks up an API import, loading the module if needed.
	//
	void* LookupAPIStart(const char* moduleName, const char* functionName);

	//
	// Installs a generic trampoline-style hook, similar to Microsoft Detours.
	//
	bool InstallTrampolineHook(void* hookAddress, void* replacementFunction, void** originalFunction);

	//
	// Installs a trampoline hook on an API.
	//
	template<typename TOrig = void>
	bool InstallAPIHook(const char* moduleName, const char* functionName, void* hookFunction, TOrig* originalFunction = nullptr)
	{
		void* apiAddress = LookupAPIStart(moduleName, functionName);

		if (apiAddress)
		{
			InstallTrampolineHook(apiAddress, hookFunction, (void**)originalFunction);
		}

		return false;
	}
}