#include <StdInc.h>
#include <internal/MinHookHookBackend.h>

#include <MinHook.h>

struct MinHookInitialize
{
	MinHookInitialize()
	{
		MH_Initialize();
	}

	~MinHookInitialize()
	{
		MH_Uninitialize();
	}
};

static MinHookInitialize mhInit;

namespace Hooking::Internal
{
    bool MinHookHookBackend::InstallTrampolineHook(void* hookAddress, void* replacementFunction, void** originalFunction)
    {
		MH_STATUS status = MH_CreateHook(hookAddress, replacementFunction, originalFunction);

		if (status != MH_OK)
		{
			FuncTrace("failed to create hook for %p - status %x (%s)\n", hookAddress, status, MH_StatusToString(status));
			return false;
		}

        status = MH_EnableHook(hookAddress);

        if (status != MH_OK)
        {
            FuncTrace("failed to enable hook for %p - status %x (%s)\n", hookAddress, status, MH_StatusToString(status));
            return false;
        }

        return true;
	}
}