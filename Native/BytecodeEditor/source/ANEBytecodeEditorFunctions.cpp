#include "ANEFunctions.hpp"

#define GET_EDITOR() BytecodeEditor* editor = reinterpret_cast<BytecodeEditor*>(funcData)

FREObject SetCurrentSWF(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(1);

    GET_EDITOR();

    CHECK_OBJECT<FRE_TYPE_BYTEARRAY>(argv[0]);

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

FREObject Cleanup(FREContext, void* funcData, uint32_t argc, FREObject[])
{
    CHECK_ARGC(0);

    GET_EDITOR();

    editor->cleanup();

    return nullptr;
}

FREObject GetClass(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(1);

    GET_EDITOR();

    ASASM::Multiname name = ConvertMultiname(argv[0]);

    classPointerHelper = editor->getClass(name);

    if (classPointerHelper == nullptr)
    {
        FAIL("Class not found");
    }

    FREObject ret;
    DO_OR_FAIL("Could not build com.cff.anebe.ir.ASClass",
        ANENewObject("com.cff.anebe.ir.ASClass", 0, nullptr, &ret, nullptr));

    return ret;
}
