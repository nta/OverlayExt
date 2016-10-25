#include <StdInc.h>
#include <BaseHooking.h>
#include <internal/HookingBackend.h>

static Hooking::Internal::IHookBackend* g_hookBackend = nullptr;

namespace Hooking
{
	bool InitializeHooking(Internal::IHookBackend* backend)
	{
		if (g_hookBackend)
		{
			FuncTrace("Tried to initialize hooking while it was already initialized.\n");
			return true;
		}

		if (!backend)
		{
			backend = GetDefaultBackend();
		}

		FuncTrace("Initialized hooking with backend %s.\n", backend->GetName());
		g_hookBackend = backend;

		return true;
	}

	//
	// Looks up an API import, loading the module if needed.
	//
	void* LookupAPIStart(const char* moduleName, const char* functionName)
	{
		auto backend = dynamic_cast<Internal::IHookAPIBackend*>(g_hookBackend);

		if (backend)
		{
			return backend->LookupAPIStart(moduleName, functionName);
		}

		return nullptr;
	}

	//
	// Installs a generic trampoline-style hook, similar to Microsoft Detours.
	//
	bool InstallTrampolineHook(void* hookAddress, void* replacementFunction, void** originalFunction)
	{
		auto backend = dynamic_cast<Internal::IHookTrampolineBackend*>(g_hookBackend);

		if (backend)
		{
			return backend->InstallTrampolineHook(hookAddress, replacementFunction, originalFunction);
		}

		return nullptr;
	}
}