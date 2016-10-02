#pragma once

#ifdef _MSC_VER
#define AYRIA_DLL_EXPORT __declspec(dllexport)
#define AYRIA_DLL_IMPORT __declspec(dllimport)
#else
#define AYRIA_DLL_EXPORT
#define AYRIA_DLL_IMPORT
#endif

#define AYRIA_EXTENSION_API extern "C" AYRIA_DLL_EXPORT
#define AYRIA_EXTENSION_CALL __cdecl

// extension exports, forward declarations
AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onInitializationStart();
AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onInitializationComplete();
AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onExtensionUnloading();

AYRIA_EXTENSION_API void AYRIA_EXTENSION_CALL onMessage(uint32_t messageId, ...);