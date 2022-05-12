#include "ANEBytecodeEditor.hpp"
#include "BytecodeEditor.hpp"
#include "utils/ANEUtils.hpp"

#include <string_view>
#include <vector>

static FREContext AIRContext;

#define GET_EDITOR(ctx)                                                                            \
    BytecodeEditor* editor = nullptr;                                                              \
    do                                                                                             \
    {                                                                                              \
        void* o;                                                                                   \
        DO_OR_FAIL("Failed to get reference to BytecodeEditor instance",                           \
            FREGetContextNativeData(ctx, &o));                                                     \
        editor = static_cast<BytecodeEditor*>(o);                                                  \
    } while (false)

FREObject Disassemble(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
    if (argc != 0)
    {
        FAIL("argc should be 0");
    }

    GET_EDITOR(ctx);

    return editor->disassemble();
}

FREObject Assemble(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
    if (argc != 3)
    {
        FAIL("argc should be 3");
    }

    GET_EDITOR(ctx);

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

    uint32_t includeDebugInstructions;
    DO_OR_FAIL("Failed to get argv[2]'s boolean value",
        FREGetObjectAsBool(argv[2], &includeDebugInstructions));

    return editor->assemble(std::move(strings), includeDebugInstructions != 0);
}

FREObject DisassembleAsync(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
    if (argc != 0)
    {
        FAIL("argc should be 0");
    }

    GET_EDITOR(ctx);

    return editor->disassembleAsync();
}

FREObject AssembleAsync(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
    if (argc != 3)
    {
        FAIL("argc should be 3");
    }

    GET_EDITOR(ctx);

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

    uint32_t includeDebugInstructions;
    DO_OR_FAIL("Failed to get argv[2]'s boolean value",
        FREGetObjectAsBool(argv[2], &includeDebugInstructions));

    return editor->assembleAsync(std::move(strings), includeDebugInstructions != 0);
}

FREObject AsyncTaskResult(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
    if (argc != 0)
    {
        FAIL("argc should be 0");
    }

    GET_EDITOR(ctx);

    return editor->taskResult();
}

FREObject SetCurrentSWF(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
    if (argc != 1)
    {
        FAIL("argc should be 1");
    }

    GET_EDITOR(ctx);

    FREObjectType type;
    DO_OR_FAIL("Failed to get object type", FREGetObjectType(argv[0], &type));
    if (type != FRE_TYPE_BYTEARRAY)
    {
        FAIL("argv[0] is not a bytearray");
    }

    FREByteArray ba;
    DO_OR_FAIL("Failed to acquire bytearray", FREAcquireByteArray(argv[0], &ba));

    std::vector<uint8_t> data;
    data.insert(data.begin(), ba.bytes, ba.bytes + ba.length);
    DO_OR_FAIL("Failed to release bytearray", FREReleaseByteArray(argv[0]));

    try
    {
        editor->setSWF(SWF::SWFFile(std::move(data)));
    }
    catch (std::exception& e)
    {
        FAIL(e.what());
    }

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

FREObject Cleanup(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[])
{
    if (argc != 0)
    {
        FAIL("argc should be 0");
    }

    GET_EDITOR(ctx);

    editor->cleanup();

    return nullptr;
}

static FRENamedFunction ANEBE_functions[] = {
    {(const uint8_t*)"Disassemble", nullptr, &Disassemble},
    {(const uint8_t*)"Assemble", nullptr, &Assemble},
    {(const uint8_t*)"DisassembleAsync", nullptr, &DisassembleAsync},
    {(const uint8_t*)"AssembleAsync", nullptr, &AssembleAsync},
    {(const uint8_t*)"AsyncTaskResult", nullptr, &AsyncTaskResult},
    {(const uint8_t*)"SetCurrentSWF", nullptr, &SetCurrentSWF},
    {(const uint8_t*)"Cleanup", nullptr, &Cleanup},
};

void ContextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx,
    uint32_t* numFunctions, const FRENamedFunction** functions)
{
    *functions    = ANEBE_functions;
    *numFunctions = sizeof(ANEBE_functions) / sizeof(ANEBE_functions[0]);

    FRESetContextNativeData(ctx, new BytecodeEditor(ctx));
}

void ContextFinalizer(FREContext ctx)
{
    void* d;
    FREGetContextNativeData(ctx, &d);
    delete static_cast<BytecodeEditor*>(d);
}

extern "C" EXPORT void ExtInitializerANEBytecodeEditor(void** extDataToSet,
    FREContextInitializer* ctxInitializerToSet, FREContextFinalizer* ctxFinalizerToSet)
{
    *extDataToSet        = nullptr;
    *ctxInitializerToSet = &ContextInitializer;
    *ctxFinalizerToSet   = &ContextFinalizer;
}

extern "C" EXPORT void ExtFinalizerANEBytecodeEditor(void* extDat) {}
