#include "ANEFunctions.hpp"
#include <numeric>
#include <ranges>

#define GET_EDITOR() BytecodeEditor& editor = *static_cast<ANEFunctionContext*>(funcData)->editor

FREObject InsertABCToSWF(FREContext, void*, uint32_t argc, FREObject argv[])
{
    CHECK_ARGC(2);

    FREObject swf = CHECK_OBJECT<FRE_TYPE_BYTEARRAY>(argv[0]);
    FREObject abc = CHECK_OBJECT<FRE_TYPE_BYTEARRAY>(argv[1]);

    try
    {
        uint32_t swfLength = uint32_t(CHECK_OBJECT<FRE_TYPE_NUMBER>(GetMember(swf, "length")));
        uint32_t abcLength = uint32_t(CHECK_OBJECT<FRE_TYPE_NUMBER>(GetMember(abc, "length")));

        static constexpr auto getTagHeaderLength = [](const SWF::SWFFile::Tag& tag)
        { return (tag.forceLongLength || tag.length >= 0x3F) ? 6 : 2; };
        static constexpr auto getTagOrigStart = [](const SWF::SWFFile::Tag& tag)
        { return tag.data - getTagHeaderLength(tag); };
        static constexpr auto getTagTotalSize = [](const SWF::SWFFile::Tag& tag)
        { return tag.length + getTagHeaderLength(tag); };

        FREByteArray swfData;
        DO_OR_FAIL("Failed to acquire SWF bytearray", FREAcquireByteArray(swf, &swfData));
        auto tags = SWF::SWFFile::getTagsFrom({swfData.bytes, swfData.length});

        DO_OR_FAIL("Failed to release SWF bytearray", FREReleaseByteArray(swf));

        const uint32_t finalSize = std::accumulate(tags.begin(), tags.end(),
            uint32_t(
                std::distance((const uint8_t*)swfData.bytes, getTagOrigStart(tags[0])) + abcLength),
            [](uint32_t a, const SWF::SWFFile::Tag& b)
            {
                if (b.type != TagType::DoABC2)
                {
                    return a + getTagTotalSize(b);
                }
                else
                {
                    return a;
                }
            });

        FREObject lengthObj;
        DO_OR_FAIL("Failed to create length object", FRENewObjectFromUint32(finalSize, &lengthObj));

        DO_OR_FAIL(
            "Failed to expand bytearray", ANESetObjectProperty(swf, "length", lengthObj, nullptr));

        DO_OR_FAIL("Failed to acquire SWF bytearray", FREAcquireByteArray(swf, &swfData));
        tags = SWF::SWFFile::getTagsFrom({swfData.bytes, swfLength});

        FREByteArray abcData;
        DO_OR_FAIL("Failed to acquire ABC bytearray", FREAcquireByteArray(abc, &abcData));

        size_t currentTag = 0;
        while (currentTag < tags.size() && tags[currentTag].type != TagType::DoABC2 &&
               tags[currentTag].type != TagType::End)
        {
            currentTag++;
        }

        if (currentTag == tags.size())
        {
            DO_OR_FAIL("Failed to release swf bytearray", FREReleaseByteArray(swf));
            DO_OR_FAIL("Failed to release abc bytearray", FREReleaseByteArray(abc));
            FAIL("No ABC or end tag in this SWF. Is it really an SWF?");
        }

        if (tags[currentTag].type == TagType::End)
        {
            std::copy(abcData.bytes, abcData.bytes + abcData.length,
                swfData.bytes + std::distance((const uint8_t*)swfData.bytes,
                                    getTagOrigStart(tags[currentTag])));

            swfData.bytes[finalSize - 2] = 0;
            swfData.bytes[finalSize - 1] = 0;
        }
        else if (currentTag < tags.size() - 1)
        {
            std::vector<ptrdiff_t> localOffsets(tags.size() - currentTag - 1);
            {
                ptrdiff_t globalOffset = std::distance((const uint8_t*)swfData.bytes,
                                             getTagOrigStart(tags[currentTag])) +
                                         abcLength;

                // Get the offsets for all the tags we need to copy
                for (size_t i = currentTag + 1; i < tags.size(); i++)
                {
                    localOffsets[i - currentTag - 1] =
                        globalOffset -
                        std::distance((const uint8_t*)swfData.bytes, getTagOrigStart(tags[i]));
                    if (tags[i].type != TagType::DoABC2)
                    {
                        globalOffset += getTagTotalSize(tags[i]);
                    }
                }
            }

            // Then copy them, forwards if they go backwards and backwards if they go forwards
            // - This ensures that you don't copy copied data
            for (size_t i = 0; i < localOffsets.size(); i++)
            {
                const auto& tag = tags[currentTag + 1 + i];
                if (localOffsets[i] < 0)
                {
                    if (tag.type != TagType::DoABC2)
                    {
                        tag.writeTo(
                            swfData.bytes +
                            std::distance((const uint8_t*)swfData.bytes, getTagOrigStart(tag)) +
                            localOffsets[i]);
                    }
                }
                else
                {
                    break;
                }
            }
            for (size_t i = localOffsets.size(); i > 0; i--)
            {
                const auto& tag = tags[currentTag + 1 + i - 1];
                if (localOffsets[i - 1] > 0)
                {
                    if (tag.type != TagType::DoABC2)
                    {
                        tag.writeToBackwards(
                            swfData.bytes +
                            std::distance((const uint8_t*)swfData.bytes, getTagOrigStart(tag)) +
                            localOffsets[i - 1]);
                    }
                }
                else
                {
                    break;
                }
            }

            // And add the actual new ABC data in
            std::copy(abcData.bytes, abcData.bytes + abcLength,
                swfData.bytes + std::distance((const uint8_t*)swfData.bytes,
                                    getTagOrigStart(tags[currentTag])));
        }
        else
        {
            DO_OR_FAIL("Failed to release swf bytearray", FREReleaseByteArray(swf));
            DO_OR_FAIL("Failed to release abc bytearray", FREReleaseByteArray(abc));
            FAIL("End tag is missing");
        }

        DO_OR_FAIL("Failed to release swf bytearray", FREReleaseByteArray(swf));
        DO_OR_FAIL("Failed to release abc bytearray", FREReleaseByteArray(abc));
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
