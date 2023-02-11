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

FREObject BytecodeEditor::disassemble()
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    if (!currentSWF)
    {
        FAIL("No SWF data specified to disassemble");
    }

    try
    {
        auto strings = Disassembler::Disassembler(
            ASASM::ASProgram::fromABC(SWFABC::ABCReader(currentSWF->abcData()).abc()))
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

    if (!currentSWF)
    {
        FAIL("No SWF data specified to reassemble");
    }

    try
    {
        std::vector<uint8_t> data = std::move(
            SWFABC::ABCWriter(Assembler::assemble(strings, includeDebugInstructions).toABC())
                .data());

        currentSWF->replaceABCData(data.data(), data.size());

        FREObject lengthObj;
        DO_OR_FAIL("Failed to create length object",
            FRENewObjectFromUint32(currentSWF->getFullSize(), &lengthObj));

        FREObject bytearrayObj;
        DO_OR_FAIL("Failed to create returned bytearray",
            ANENewObject("flash.utils.ByteArray", 0, nullptr, &bytearrayObj, nullptr));

        DO_OR_FAIL("Failed to set returned bytearray length to required size",
            ANESetObjectProperty(bytearrayObj, "length", lengthObj, nullptr));

        FREByteArray ba;
        DO_OR_FAIL("Failed to acquire bytearray", FREAcquireByteArray(bytearrayObj, &ba));

        currentSWF->writeTo(ba.bytes);
        DO_OR_FAIL("Failed to release bytearray", FREReleaseByteArray(bytearrayObj));

        return bytearrayObj;
    }
    catch (const std::exception& e)
    {
        FAIL(std::string("Exception during assembly: ") + e.what());
    }
}

FREObject BytecodeEditor::disassembleAsync()
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    runningTask = std::jthread(
        [this]
        {
            if (!currentSWF)
            {
                FAIL_ASYNC("No SWF data specified to disassemble");
            }

            try
            {
                SUCCEED_ASYNC(Disassembler::Disassembler(
                    ASASM::ASProgram::fromABC(SWFABC::ABCReader(currentSWF->abcData()).abc()))
                                  .disassemble());
            }
            catch (const std::exception& e)
            {
                FAIL_ASYNC(std::string("Exception during disassembly: ") + e.what());
            }
        });

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
            if (!currentSWF)
            {
                FAIL_ASYNC("No SWF data specified to reassemble");
            }
            try
            {
                std::vector<uint8_t> data = std::move(SWFABC::ABCWriter(
                    Assembler::assemble(strings, includeDebugInstructions).toABC())
                                                          .data());

                SUCCEED_ASYNC(data);
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
        this->partialAssembly = std::make_unique<std::pair<ASASM::ASProgram, RefBuilder>>(
            std::move(assembled), std::move(rb));
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
                this->partialAssembly = std::make_unique<std::pair<ASASM::ASProgram, RefBuilder>>(
                    std::move(assembled), std::move(rb));

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

    if (!currentSWF)
    {
        FAIL("No SWF data specified to reassemble");
    }
    if (!partialAssembly)
    {
        FAIL("No partial assembly found");
    }

    try
    {
        std::vector<uint8_t> data =
            std::move(SWFABC::ABCWriter(partialAssembly->first.toABC()).data());
        partialAssembly = nullptr;

        currentSWF->replaceABCData(data.data(), data.size());

        FREObject lengthObj;
        DO_OR_FAIL("Failed to create length object",
            FRENewObjectFromUint32(currentSWF->getFullSize(), &lengthObj));

        FREObject bytearrayObj;
        DO_OR_FAIL("Failed to create returned bytearray",
            ANENewObject("flash.utils.ByteArray", 0, nullptr, &bytearrayObj, nullptr));

        DO_OR_FAIL("Failed to set returned bytearray length to required size",
            ANESetObjectProperty(bytearrayObj, "length", lengthObj, nullptr));

        FREByteArray ba;
        DO_OR_FAIL("Failed to acquire bytearray", FREAcquireByteArray(bytearrayObj, &ba));

        currentSWF->writeTo(ba.bytes);
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
            if (!currentSWF)
            {
                FAIL_ASYNC("No SWF data specified to reassemble");
            }
            if (!partialAssembly)
            {
                FAIL_ASYNC("No partial assembly found");
            }
            try
            {
                std::vector<uint8_t> data =
                    std::move(SWFABC::ABCWriter(partialAssembly->first.toABC()).data());

                partialAssembly = nullptr;

                SUCCEED_ASYNC(data);
            }
            catch (std::exception& e)
            {
                FAIL_ASYNC(std::string("Exception while finishing assembly: ") + e.what());
            }
            catch (...)
            {
                FAIL_ASYNC("AAAA");
            }
        });

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

FREObject BytecodeEditor::beginIntrospection()
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    if (!currentSWF)
    {
        FAIL("No SWF data to begin introspection on");
    }

    try
    {
        auto assembled = ASASM::ASProgram::fromABC(SWFABC::ABCReader(currentSWF->abcData()).abc());
        RefBuilder rb(assembled);
        rb.run();
        this->partialAssembly = std::make_unique<std::pair<ASASM::ASProgram, RefBuilder>>(
            std::move(assembled), std::move(rb));

        currentSWF = std::nullopt;
    }
    catch (std::exception& e)
    {
        FAIL(std::string("Exception while beginning introspection: ") + e.what());
    }

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

ASASM::Class* BytecodeEditor::getClass(const ASASM::Multiname& className) const
{
    for (const auto& script : partialAssembly->first.scripts)
    {
        for (const auto& trait : script.traits)
        {
            if (trait.kind == TraitKind::Class && className == trait.name)
            {
                return trait.vClass().vclass.get();
            }
        }
    }

    return nullptr;
}

ASASM::Script* BytecodeEditor::getScript(const ASASM::Multiname& traitName) const
{
    for (auto& script : partialAssembly->first.scripts)
    {
        for (const auto& trait : script.traits)
        {
            if (traitName == trait.name)
            {
                return &script;
            }
        }
    }

    return nullptr;
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

            if (!currentSWF)
            {
                FAIL("Current SWF not set when trying to access result data. Did you accidentally "
                     "clean up first?");
            }

            try
            {
                currentSWF->replaceABCData(data.data(), data.size());

                FREObject lengthObj;
                DO_OR_FAIL("Failed to create length object",
                    FRENewObjectFromUint32(currentSWF->getFullSize(), &lengthObj));

                FREObject bytearrayObj;
                DO_OR_FAIL("Failed to create returned bytearray",
                    ANENewObject("flash.utils.ByteArray", 0, nullptr, &bytearrayObj, nullptr));

                DO_OR_FAIL("Failed to set returned bytearray length to required size",
                    ANESetObjectProperty(bytearrayObj, "length", lengthObj, nullptr));

                FREByteArray ba;
                DO_OR_FAIL("Failed to acquire bytearray", FREAcquireByteArray(bytearrayObj, &ba));

                currentSWF->writeTo(ba.bytes);

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
