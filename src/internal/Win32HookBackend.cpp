#include <StdInc.h>
#include <internal/Win32HookBackend.h>

#include <windows.h>

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
            auto lastError = GetLastError();
            FuncTrace("Could not load module %s (error code %d).\n", moduleName, lastError);

			return nullptr;
		}

		void* pFunc = GetProcAddress(hModule, functionName);

        if (!pFunc)
        {
            FuncTrace("Could not find function %s in module %s.\n", functionName, moduleName);
        }

        return pFunc;
	}
}