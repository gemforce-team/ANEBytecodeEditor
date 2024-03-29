#pragma once
#include "BytecodeEditor.hpp"
#include "utils/ANEUtils.hpp"
#include <stdint.h>
#include <unordered_map>

#include <windows.h>

#include <FlashRuntimeExtensions.h>

#define GET_EDITOR() BytecodeEditor& editor = *static_cast<ANEFunctionContext*>(funcData)->editor

template <FREObject (BytecodeEditor::*Assembler)(
    std::unordered_map<std::string, std::string>&&, bool)>
FREObject Assemble(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(3);

    GET_EDITOR();

    CHECK_OBJECT<FRE_TYPE_OBJECT>(argv[0]);
    CHECK_OBJECT<FRE_TYPE_VECTOR>(argv[1]);
    bool includeDebugInstructions = CHECK_OBJECT<FRE_TYPE_BOOLEAN>(argv[2]);

    uint32_t vecSize;
    DO_OR_FAIL("Failed to get argv[1] size", FREGetArrayLength(argv[1], &vecSize));

    std::unordered_map<std::string, std::string> strings;
    try
    {
        for (uint32_t i = 0; i < vecSize; i++)
        {
            FREObject str;
            DO_OR_FAIL("Failed to get argv[1][i]", FREGetArrayElementAt(argv[1], i, &str));
            char* key;
            uint32_t keyLen;
            DO_OR_FAIL("Failed to get argv[1][i]'s string value",
                FREGetObjectAsUTF8(str, &keyLen, (const uint8_t**)&key));
            FREObject str2;
            DO_OR_FAIL("Failed to get argv[0][argv[1][i]]",
                FREGetObjectProperty(argv[0], (const uint8_t*)key, &str2, nullptr));
            char* val;
            uint32_t valLen;
            DO_OR_FAIL("Failed to get argv[0][argv[1][i]]'s string value",
                FREGetObjectAsUTF8(str2, &valLen, (const uint8_t**)&val));
            strings.emplace(key, val);
        }
    }
    catch (const std::exception& e)
    {
        FAIL(std::string("Exception occurred while converting strings: ") + e.what());
    }

    return (editor.*Assembler)(std::move(strings), includeDebugInstructions);
}

template <FREObject (BytecodeEditor::*Disassembler)(std::span<const uint8_t>)>
FREObject Disassemble(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(1);

    GET_EDITOR();

    CHECK_OBJECT<FRE_TYPE_BYTEARRAY>(argv[0]);

    FREByteArray ba;
    DO_OR_FAIL("Could not acquire SWF byte data", FREAcquireByteArray(argv[0], &ba));

    FREObject ret = (editor.*Disassembler)({ba.bytes, ba.length});

    DO_OR_FAIL("Could not release SWF byte data", FREReleaseByteArray(argv[0]));

    return ret;
}

template <auto Function>
    requires std::same_as<decltype(Function), FREObject (BytecodeEditor::*)()> ||
             std::same_as<decltype(Function), FREObject (BytecodeEditor::*)() const>
FREObject TransparentZeroArg(FREContext, void* funcData, uint32_t argc, FREObject[])
{
    CHECK_ARGC(0);

    GET_EDITOR();

    return (editor.*Function)();
}

template <FREObject (*Function)(FREContext, void*, uint32_t, FREObject[])>
FREObject CheckAssemblyValid(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
    ANEFunctionContext& context = *static_cast<ANEFunctionContext*>(funcData);
    BytecodeEditor& editor      = *context.editor;

    if (editor.partialAssembly.get() != context.objectData->program)
    {
        FAIL("Invalid instance: underlying data has been disposed of");
    }

    return Function(ctx, funcData, argc, argv);
}

#undef GET_EDITOR
