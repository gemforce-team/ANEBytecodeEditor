#include "BytecodeEditor.hpp"
#include "ABC/ABCReader.hpp"
#include "ABC/ABCWriter.hpp"
#include "Assembler.hpp"
#include "Disassembler.hpp"
#include "SWF/SWFFile.hpp"

#define FAIL_ASYNC(x)                                                                              \
    m_taskResult = x;                                                                              \
    FREDispatchStatusEventAsync(ctx, (const uint8_t*)"Task complete", (const uint8_t*)"ERROR")

#define SUCCEED_ASYNC(x)                                                                           \
    m_taskResult = x;                                                                              \
    FREDispatchStatusEventAsync(ctx, (const uint8_t*)"Task complete", (const uint8_t*)"SUCCESS")

#define SUCCEED_ASYNC_WITHMESSAGE(message)                                                         \
    FREDispatchStatusEventAsync(ctx, (const uint8_t*)"Task complete", (const uint8_t*)(message))

FREObject BytecodeEditor::disassemble(std::span<const uint8_t> swf)
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    try
    {
        auto strings = Disassembler::Disassembler(
            ASASM::ASProgram::fromABC(SWF::SWFFile::extractABCFrom(swf).value()))
                           .disassemble();

        FREObject ret;
        DO_OR_FAIL(
            "Failed to create return object", ANENewObject("Object", 0, nullptr, &ret, nullptr));

        for (const auto& file : strings)
        {
            DO_OR_FAIL("Failed to set data",
                ANESetObjectProperty(ret, file.first.c_str(), FREString(file.second), nullptr));
        }

        return ret;
    }
    catch (std::bad_optional_access&)
    {
        FAIL("SWF does not appear to have any ABC tags");
    }
    catch (std::exception& e)
    {
        FAIL(std::string("Exception during disassembly: ") + e.what());
    }
}

FREObject BytecodeEditor::assemble(
    std::unordered_map<std::string, std::string>&& strings, bool includeDebugInstructions)
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    try
    {
        std::vector<uint8_t> data = std::move(
            SWFABC::ABCWriter(Assembler::assemble(strings, includeDebugInstructions).toABC())
                .data());

        auto tagInfo = SWF::SWFFile::buildTagHeaderForABCData(data);

        FREObject lengthObj;
        DO_OR_FAIL("Failed to create length object",
            FRENewObjectFromUint32(data.size() + tagInfo.size(), &lengthObj));

        FREObject bytearrayObj;
        DO_OR_FAIL("Failed to create returned bytearray",
            ANENewObject("flash.utils.ByteArray", 0, nullptr, &bytearrayObj, nullptr));

        DO_OR_FAIL("Failed to set returned bytearray length to required size",
            ANESetObjectProperty(bytearrayObj, "length", lengthObj, nullptr));

        FREByteArray ba;
        DO_OR_FAIL("Failed to acquire bytearray", FREAcquireByteArray(bytearrayObj, &ba));

        std::copy(tagInfo.begin(), tagInfo.end(), ba.bytes);

        std::copy(data.begin(), data.end(), ba.bytes + tagInfo.size());

        DO_OR_FAIL("Failed to release bytearray", FREReleaseByteArray(bytearrayObj));

        return bytearrayObj;
    }
    catch (const std::exception& e)
    {
        FAIL(std::string("Exception during assembly: ") + e.what());
    }
}

FREObject BytecodeEditor::disassembleAsync(std::span<const uint8_t> data)
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    try
    {
        runningTask = std::jthread(
            [this, abc = SWF::SWFFile::extractABCFrom(data)]
            {
                try
                {
                    SUCCEED_ASYNC(Disassembler::Disassembler(ASASM::ASProgram::fromABC(abc.value()))
                                      .disassemble());
                }
                catch (std::bad_optional_access&)
                {
                    FAIL_ASYNC("SWF does not appear to have any ABC tags");
                }
                catch (const std::exception& e)
                {
                    FAIL_ASYNC(std::string("Exception during disassembly: ") + e.what());
                }
            });
    }
    catch (const std::exception& e)
    {
        FAIL(std::string("Exception during disassembly: ") + e.what());
    }

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

FREObject BytecodeEditor::assembleAsync(
    std::unordered_map<std::string, std::string>&& strings, bool includeDebugInstructions)
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    runningTask = std::jthread(
        [this, strings = std::move(strings), includeDebugInstructions]
        {
            try
            {
                SUCCEED_ASYNC(std::move(SWFABC::ABCWriter(
                    Assembler::assemble(strings, includeDebugInstructions).toABC())
                                            .data()));
            }
            catch (const std::exception& e)
            {
                FAIL_ASYNC(std::string("Exception during assembly: ") + e.what());
            }
        });

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

FREObject BytecodeEditor::partialAssemble(
    std::unordered_map<std::string, std::string>&& strings, bool includeDebugInstructions)
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    try
    {
        auto assembled = Assembler::assemble(strings, includeDebugInstructions);
        RefBuilder rb(assembled);
        rb.run();
        this->partialAssembly =
            std::make_unique<PartialAssembly>(std::move(assembled), std::move(rb));
    }
    catch (const std::exception& e)
    {
        FAIL(std::string("Exception during partial assembly: ") + e.what());
    }

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

FREObject BytecodeEditor::partialAssembleAsync(
    std::unordered_map<std::string, std::string>&& strings, bool includeDebugInstructions)
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    runningTask = std::jthread(
        [this, strings = std::move(strings), includeDebugInstructions]
        {
            try
            {
                auto assembled = Assembler::assemble(strings, includeDebugInstructions);
                RefBuilder rb(assembled);
                rb.run();
                this->partialAssembly =
                    std::make_unique<PartialAssembly>(std::move(assembled), std::move(rb));

                SUCCEED_ASYNC_WITHMESSAGE("PartialSuccess");
            }
            catch (std::exception& e)
            {
                FAIL_ASYNC(std::string("Exception during partial assembly: ") + e.what());
            }
        });

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

FREObject BytecodeEditor::finishAssemble()
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    if (!partialAssembly)
    {
        FAIL("No partial assembly found");
    }

    try
    {
        std::vector<uint8_t> data =
            std::move(SWFABC::ABCWriter(partialAssembly->program.toABC()).data());
        partialAssembly = nullptr;

        auto tagInfo = SWF::SWFFile::buildTagHeaderForABCData(data);

        FREObject lengthObj;
        DO_OR_FAIL("Failed to create length object",
            FRENewObjectFromUint32(data.size() + tagInfo.size(), &lengthObj));

        FREObject bytearrayObj;
        DO_OR_FAIL("Failed to create returned bytearray",
            ANENewObject("flash.utils.ByteArray", 0, nullptr, &bytearrayObj, nullptr));

        DO_OR_FAIL("Failed to set returned bytearray length to required size",
            ANESetObjectProperty(bytearrayObj, "length", lengthObj, nullptr));

        FREByteArray ba;
        DO_OR_FAIL("Failed to acquire bytearray", FREAcquireByteArray(bytearrayObj, &ba));

        std::copy(tagInfo.begin(), tagInfo.end(), ba.bytes);

        std::copy(data.begin(), data.end(), ba.bytes + tagInfo.size());
        DO_OR_FAIL("Failed to release bytearray", FREReleaseByteArray(bytearrayObj));

        return bytearrayObj;
    }
    catch (const std::exception& e)
    {
        FAIL(std::string("Exception while finishing assembly: ") + e.what());
    }
}

FREObject BytecodeEditor::finishAssembleAsync()
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    runningTask = std::jthread(
        [this]
        {
            if (!partialAssembly)
            {
                FAIL_ASYNC("No partial assembly found");
            }
            try
            {
                std::vector<uint8_t> data =
                    std::move(SWFABC::ABCWriter(partialAssembly->program.toABC()).data());

                partialAssembly = nullptr;

                SUCCEED_ASYNC(std::move(data));
            }
            catch (std::exception& e)
            {
                FAIL_ASYNC(std::string("Exception while finishing assembly: ") + e.what());
            }
        });

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

FREObject BytecodeEditor::beginIntrospection(std::span<const uint8_t> swf)
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    try
    {
        auto assembled = ASASM::ASProgram::fromABC(SWF::SWFFile::extractABCFrom(swf).value());
        RefBuilder rb(assembled);
        rb.run();
        this->partialAssembly =
            std::make_unique<PartialAssembly>(std::move(assembled), std::move(rb));
    }
    catch (std::bad_optional_access&)
    {
        FAIL("SWF does not appear to have any ABC tags");
    }
    catch (std::exception& e)
    {
        FAIL(std::string("Exception while beginning introspection: ") + e.what());
    }

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

FREObject BytecodeEditor::listClasses() const
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }
    if (!partialAssembly)
    {
        FAIL("No partial assembly found");
    }

    FREObject ret;
    DO_OR_FAIL("Couldn't create class name vector",
        ANENewObject("Vector.<com.cff.anebe.ir.ASMultiname>", 0, nullptr, &ret, nullptr));
    DO_OR_FAIL("Couldn't set class name vector size to nominal",
        FRESetArrayLength(ret, partialAssembly->program.scripts.size()));
    size_t index = 0;
    for (const auto& script : partialAssembly->program.scripts)
    {
        for (const auto& trait : script->traits)
        {
            if (trait.kind == TraitKind::Class)
            {
                DO_OR_FAIL("Couldn't set class name vector entry",
                    FRESetArrayElementAt(ret, index++, ConvertMultiname(trait.name)));
            }
        }
    }
    DO_OR_FAIL("Couldn't shrink class name vector size to actual", FRESetArrayLength(ret, index));

    return ret;
}

FREObject BytecodeEditor::listScripts() const
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }
    if (!partialAssembly)
    {
        FAIL("No partial assembly found");
    }

    FREObject ret;
    DO_OR_FAIL("Couldn't create script name vector",
        ANENewObject("Vector.<com.cff.anebe.ir.ASMultiname>", 0, nullptr, &ret, nullptr));
    DO_OR_FAIL("Couldn't set script name vector size to nominal",
        FRESetArrayLength(ret, partialAssembly->program.scripts.size()));
    size_t index = 0;
    for (const auto& script : partialAssembly->program.scripts)
    {
        if (script->traits.size() > 0)
        {
            DO_OR_FAIL("Couldn't set script name vector entry",
                FRESetArrayElementAt(ret, index++, ConvertMultiname(script->traits[0].name)));
        }
    }
    DO_OR_FAIL("Couldn't shrink script name vector size to actual", FRESetArrayLength(ret, index));

    return ret;
}

std::shared_ptr<ASASM::Class> BytecodeEditor::getClass(const ASASM::Multiname& className) const
{
    if (!partialAssembly)
    {
        return nullptr;
    }

    for (const auto& script : partialAssembly->program.scripts)
    {
        for (const auto& trait : script->traits)
        {
            if (trait.kind == TraitKind::Class && className == trait.name)
            {
                return trait.vClass().vclass;
            }
        }
    }

    return nullptr;
}

std::shared_ptr<ASASM::Script> BytecodeEditor::getScript(const ASASM::Multiname& traitName) const
{
    if (!partialAssembly)
    {
        return nullptr;
    }

    for (const auto& script : partialAssembly->program.scripts)
    {
        for (const auto& trait : script->traits)
        {
            if (traitName == trait.name)
            {
                return script;
            }
        }
    }

    return nullptr;
}

std::shared_ptr<ASASM::Script> BytecodeEditor::createScript(
    const std::shared_ptr<ASASM::Method>& sinit)
{
    return partialAssembly->program.scripts.emplace_back(new ASASM::Script{sinit});
}

#undef FAIL_RETURN
#define FAIL_RETURN(x)                                                                             \
    do                                                                                             \
    {                                                                                              \
        m_taskResult = std::monostate{};                                                           \
        return x;                                                                                  \
    }                                                                                              \
    while (false)

FREObject BytecodeEditor::taskResult()
{
    if (runningTask.joinable())
    {
        runningTask.join();
    }
    switch (m_taskResult.index())
    {
        case 0:
            return nullptr;
        case 1:
        {
            FREObject r  = FREString(std::get<1>(m_taskResult));
            m_taskResult = std::monostate{};
            return r;
        }
        case 2:
        {
            const auto& strings = std::get<2>(m_taskResult);

            FREObject ret;
            DO_OR_FAIL("Failed to create return object",
                ANENewObject("Object", 0, nullptr, &ret, nullptr));

            for (const auto& file : strings)
            {
                DO_OR_FAIL("Failed to set data",
                    ANESetObjectProperty(ret, file.first.c_str(), FREString(file.second), nullptr));
            }

            m_taskResult = std::monostate{};
            return ret;
        }
        case 3:
        {
            const auto& data = std::get<3>(m_taskResult);

            try
            {
                auto tagInfo = SWF::SWFFile::buildTagHeaderForABCData(data);

                FREObject lengthObj;
                DO_OR_FAIL("Failed to create length object",
                    FRENewObjectFromUint32(data.size() + tagInfo.size(), &lengthObj));

                FREObject bytearrayObj;
                DO_OR_FAIL("Failed to create returned bytearray",
                    ANENewObject("flash.utils.ByteArray", 0, nullptr, &bytearrayObj, nullptr));

                DO_OR_FAIL("Failed to set returned bytearray length to required size",
                    ANESetObjectProperty(bytearrayObj, "length", lengthObj, nullptr));

                FREByteArray ba;
                DO_OR_FAIL("Failed to acquire bytearray", FREAcquireByteArray(bytearrayObj, &ba));

                std::copy(tagInfo.begin(), tagInfo.end(), ba.bytes);

                std::copy(data.begin(), data.end(), ba.bytes + tagInfo.size());

                DO_OR_FAIL("Failed to release bytearray", FREReleaseByteArray(bytearrayObj));

                m_taskResult = std::monostate{};

                return bytearrayObj;
            }
            catch (const std::exception& e)
            {
                FAIL(std::string("Exception occurred while getting assembled SWF: ") + e.what());
            }
        }
        default:
            FAIL("Invalid task result index");
    }
}
