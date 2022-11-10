#pragma once
#include "BytecodeEditor.hpp"
#include "utils/ANEUtils.hpp"
#include <FlashRuntimeExtensions.h>
#include <stdint.h>
#include <unordered_map>

#define GET_EDITOR() BytecodeEditor* editor = reinterpret_cast<BytecodeEditor*>(funcData)

template <FREObject (BytecodeEditor::*Assembler)(
    std::unordered_map<std::string, std::string>&&, bool)>
FREObject Assemble(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(3);

    GET_EDITOR();

    CHECK_OBJECT<FRE_TYPE_OBJECT>(argv[0]);
    CHECK_OBJECT<FRE_TYPE_VECTOR>(argv[1]);
    bool includeDebugInstructions = CHECK_OBJECT<FRE_TYPE_BOOLEAN>(argv[2]);

    FREObjectType type;
    DO_OR_FAIL("Failed to get argv[0] type", FREGetObjectType(argv[0], &type));
    if (type != FRE_TYPE_OBJECT)
    {
        FAIL("argv[0] is not an object");
    }
    DO_OR_FAIL("Failed to get argv[1] type", FREGetObjectType(argv[1], &type));
    if (type != FRE_TYPE_VECTOR)
    {
        FAIL("argv[1] is not a vector");
    }
    DO_OR_FAIL("Failed to get argv[2] type", FREGetObjectType(argv[2], &type));
    if (type != FRE_TYPE_BOOLEAN)
    {
        FAIL("argv[2] is not a boolean");
    }

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

    return (editor->*Assembler)(std::move(strings), includeDebugInstructions);
}

template <FREObject (BytecodeEditor::*Function)()>
FREObject TransparentZeroArg(FREContext, void* funcData, uint32_t argc, FREObject[])
{
    CHECK_ARGC(0);

    GET_EDITOR();

    return (editor->*Function)();
}

#undef GET_EDITOR
