#include <StdInc.h>
#include <AyriaExtension.h>

#include <BaseHooking.h>

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onInitializationStart()
{
	Trace("Initializing extension! Woo-hoo!\n");

	Hooking::InitializeHooking();
}

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onInitializationComplete()
{
	Trace("Initialized extension! Yippee!\n");
}

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onExtensionUnloading()
{
	Trace("Unloading extension. Bye!\n");
}

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onMessage(uint32_t messageId, ...)
{

}