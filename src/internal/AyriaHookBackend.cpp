#include <StdInc.h>
#include <BaseHooking.h>
#include <internal/HookingBackend.h>
#include <internal/MinHookHookBackend.h>
#include <internal/Win32HookBackend.h>

namespace Hooking
{
    namespace Internal
    {
        class AyriaHookBackend : public IHookBackend, public MinHookHookBackend, public Win32HookBackend
        {
            virtual const char* GetName() override
            {
                return "AyriaHookBackend";
            }
        };
    }

    static Internal::AyriaHookBackend g_hookBackend;

    Internal::IHookBackend* GetDefaultBackend()
    {
        return &g_hookBackend;
    }
}