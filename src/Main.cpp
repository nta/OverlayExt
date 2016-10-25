#include <StdInc.h>
#include <AyriaExtension.h>

#include <BaseHooking.h>

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onInitializationStart()
{
	FuncTrace("Initializing extension.\n");

	Hooking::InitializeHooking();
}

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onInitializationComplete()
{
    FuncTrace("Initialized extensions.\n");
}

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onExtensionUnloading()
{
    FuncTrace("Unloading extension.\n");
}

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onMessage(uint32_t messageId, ...)
{

}