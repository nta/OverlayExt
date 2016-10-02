#include <StdInc.h>
#include <internal/MinHookHookBackend.h>

#include <MinHook.h>

namespace Hooking::Internal
{
	void* Win32HookBackend::LookupAPIStart(const char* moduleName, const char* functionName)
	{
		// cache a widestring version of the name
		std::wstring moduleNameWide = StringToWide(moduleName);

		// is the image already loaded by this name?
		HMODULE hModule = GetModuleHandleW(moduleNameWide.c_str());

		// if not, try loading it
		if (!hModule)
		{
			hModule = LoadLibraryW(moduleNameWide.c_str());
		}

		// still no module? error out.
		if (!hModule)
		{
			return nullptr;
		}

		return GetProcAddress(hModule, functionName);
	}
}