#pragma once

#include <windows.h>

#include <FlashRuntimeExtensions.h>

#if defined(WIN32)
#define EXPORT __declspec(dllexport)
#else
// Symbols tagged with EXPORT are externally visible.
// Must use the -fvisibility=hidden gcc option.
#define EXPORT __attribute__((visibility("default")))
#endif

void ContextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx,
    uint32_t* numFunctions, const FRENamedFunction** functions);
void ContextFinalizer(FREContext ctx);

extern "C" {
EXPORT void ExtInitializerANEBytecodeEditor(void** extDataToSet,
    FREContextInitializer* ctxInitializerToSet, FREContextFinalizer* ctxFinalizerToSet);
EXPORT void ExtFinalizerANEBytecodeEditor(void* extData);
}
