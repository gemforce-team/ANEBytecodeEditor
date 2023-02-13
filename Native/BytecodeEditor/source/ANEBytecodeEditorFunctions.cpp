#include "ANEFunctions.hpp"

#define GET_EDITOR() BytecodeEditor& editor = *static_cast<ANEFunctionContext*>(funcData)->editor

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
        editor.setSWF(SWF::SWFFile(std::move(data)));
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

    editor.cleanup();

    return nullptr;
}

FREObject GetClass(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(1);

    GET_EDITOR();

    ASASM::Multiname name = editor.ConvertMultiname(argv[0]);

    std::shared_ptr<ASASM::Class> clazz = editor.getClass(name);

    if (clazz == nullptr)
    {
        FAIL("Class not found");
    }

    nextObjectContext = ANEFunctionContext{
        editor.shared_from_this(), nullptr, ANEFunctionContext::ObjectData{clazz}};

    FREObject ret;
    DO_OR_FAIL("Could not build com.cff.anebe.ir.ASClass",
        ANENewObject("com.cff.anebe.ir.ASClass", 0, nullptr, &ret, nullptr));

    return ret;
}

FREObject GetScript(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(1);

    GET_EDITOR();

    ASASM::Multiname name = editor.ConvertMultiname(argv[0]);

    std::shared_ptr<ASASM::Script> script = editor.getScript(name);

    if (script == nullptr)
    {
        FAIL("Script not found");
    }

    nextObjectContext = ANEFunctionContext{
        editor.shared_from_this(), nullptr, ANEFunctionContext::ObjectData{script}};

    FREObject ret;
    DO_OR_FAIL("Could not build com.cff.anebe.ir.ASScript",
        ANENewObject("com.cff.anebe.ir.ASScript", 0, nullptr, &ret, nullptr));

    return ret;
}

FREObject CreateScript(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(1);

    GET_EDITOR();

    std::shared_ptr<ASASM::Script> script;

    try
    {
        script = editor.createScript(editor.ConvertMethod(argv[0]));
    }
    catch (FREObject o)
    {
        return o;
    }
    catch (std::exception& e)
    {
        FAIL(e.what());
    }

    nextObjectContext = ANEFunctionContext{
        editor.shared_from_this(), nullptr, ANEFunctionContext::ObjectData{script}};

    FREObject ret;
    DO_OR_FAIL("Could not build com.cff.anebe.ir.ASScript",
        ANENewObject("com.cff.anebe.ir.ASScript", 0, nullptr, &ret, nullptr));

    return ret;
}

FREObject GetROClass(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(1);

    GET_EDITOR();

    ASASM::Multiname name = editor.ConvertMultiname(argv[0]);

    std::shared_ptr<ASASM::Class> clazz = editor.getClass(name);

    if (clazz == nullptr)
    {
        FAIL("Class not found");
    }

    nextObjectContext = ANEFunctionContext{
        editor.shared_from_this(), nullptr, ANEFunctionContext::ObjectData{clazz}};

    FREObject ret;
    DO_OR_FAIL("Could not build com.cff.anebe.ir.ASClass",
        ANENewObject("com.cff.anebe.ir.ASReadOnlyClass", 0, nullptr, &ret, nullptr));

    return ret;
}

FREObject GetROScript(FREContext, void* funcData, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(1);

    GET_EDITOR();

    ASASM::Multiname name = editor.ConvertMultiname(argv[0]);

    std::shared_ptr<ASASM::Script> script = editor.getScript(name);

    if (script == nullptr)
    {
        FAIL("Script not found");
    }

    nextObjectContext = ANEFunctionContext{
        editor.shared_from_this(), nullptr, ANEFunctionContext::ObjectData{script}};

    FREObject ret;
    DO_OR_FAIL("Could not build com.cff.anebe.ir.ASScript",
        ANENewObject("com.cff.anebe.ir.ASReadOnlyScript", 0, nullptr, &ret, nullptr));

    return ret;
}
