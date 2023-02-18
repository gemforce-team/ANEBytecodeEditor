#pragma once

#include "BytecodeEditor.hpp"
#include "utils/ANEFunctionContext.hpp"
#include "utils/ANEUtils.hpp"
#include <stdint.h>
#include <unordered_map>

#include <windows.h>

#include <FlashRuntimeExtensions.h>

inline std::optional<ANEFunctionContext> nextObjectContext;

template <FREObject (BytecodeEditor::*Assembler)(
    std::unordered_map<std::string, std::string>&&, bool)>
FREObject Assemble(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
template <FREObject (BytecodeEditor::*Function)()>
FREObject TransparentZeroArg(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
template <FREObject (*Function)(FREContext, void*, uint32_t, FREObject[])>
FREObject CheckAssemblyValid(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);

#include "ANEFunctions.tcc"

FREObject SetCurrentSWF(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
FREObject Cleanup(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
FREObject GetClass(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
FREObject GetScript(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
FREObject GetROClass(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
FREObject GetROScript(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);

FREObject CreateScript(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);

FREObject InsertABCToSWF(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);

namespace ASClass
{
    // traits
    FREObject GetStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject ListStaticTraits(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject DeleteStaticTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // instance.traits
    FREObject GetInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject ListInstanceTraits(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject DeleteInstanceTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // cinit
    FREObject GetStaticConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetStaticConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // instance.iinit
    FREObject GetConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetConstructor(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // instance.interfaces
    FREObject GetInterfaces(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetInterfaces(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // instance.superName
    FREObject GetSuperclass(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetSuperclass(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // instance.name
    FREObject GetInstanceName(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetInstanceName(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // instance.flags
    FREObject GetFlags(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetFlags(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // instance.protectedNs
    FREObject GetProtectedNamespace(
        FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetProtectedNamespace(
        FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);

    // Allows converting an FREObject back to a shared_ptr
    FREObject ConvertClassHelper(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
}

namespace ASScript
{
    // sinit
    FREObject GetInitializer(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetInitializer(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // traits
    FREObject GetTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject ListTraits(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject SetTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    FREObject DeleteTrait(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
    // Adding classes
    FREObject CreateClass(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]);
}
