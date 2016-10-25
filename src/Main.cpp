#include <StdInc.h>
#include <AyriaExtension.h>

#include <BaseHooking.h>

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onInitializationStart()
{
	FuncTrace("Initializing extension.\n");

	Hooking::InitializeHooking();
	InitFunctionBase::RunAll();

	InitStartEvent.Invoke();
}

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onInitializationComplete()
{
	FuncTrace("Initialized extensions.\n");

	InitCompleteEvent.Invoke();
}

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onExtensionUnloading()
{
	FuncTrace("Unloading extension.\n");

	UnloadEvent.Invoke();
}

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onMessage(uint32_t messageId, ...)
{
	va_list ap;
	va_start(ap, messageId);

	MessageEvent.Invoke(messageId, [=] ()
	{
		va_list retval;
		va_copy(retval, ap);

		return retval;
	});

	va_end(ap);
}

ayria::Event<> InitStartEvent;
ayria::Event<> InitCompleteEvent;
ayria::Event<> UnloadEvent;
ayria::Event<uint32_t, std::function<va_list()>> MessageEvent;