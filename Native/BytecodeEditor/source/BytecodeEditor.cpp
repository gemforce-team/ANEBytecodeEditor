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
            ASASM::ASProgram::fromABC(ABC::ABCReader(currentSWF->abcData()).abc()))
                           .disassemble();

        FREObject ret;
        DO_OR_FAIL("Failed to create return object",
            FRENewObject((uint8_t*)"Object", 0, nullptr, &ret, nullptr));

        for (const auto& file : strings)
        {
            FREObject str;
            DO_OR_FAIL("Failed to create data string", FREString(file.second, &str));
            DO_OR_FAIL("Failed to set data",
                FRESetObjectProperty(ret, (const uint8_t*)file.first.c_str(), str, nullptr));
        }

        return ret;
    }
    catch (std::exception& e)
    {
        FAIL(std::string("Exception during disassembly: ") + e.what());
    }
}

FREObject BytecodeEditor::assemble(std::unordered_map<std::string, std::string>&& strings)
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    if (!currentSWF)
    {
        FAIL("No SWF data specified to reassemble");
    }
    std::vector<uint8_t> data =
        std::move(ABC::ABCWriter(Assembler::assemble(strings).toABC()).data());

    currentSWF->replaceABCData(data.data(), data.size());

    FREObject lengthObj;
    DO_OR_FAIL("Failed to create length object",
        FRENewObjectFromUint32(currentSWF->getFullSize(), &lengthObj));

    FREObject bytearrayObj;
    DO_OR_FAIL("Failed to create returned bytearray",
        FRENewObject((const uint8_t*)"flash.utils.ByteArray", 0, nullptr, &bytearrayObj, nullptr));

    DO_OR_FAIL("Failed to set returned bytearray length to required size",
        FRESetObjectProperty(bytearrayObj, (const uint8_t*)"length", lengthObj, nullptr));

    FREByteArray ba;
    DO_OR_FAIL("Failed to acquire bytearray", FREAcquireByteArray(bytearrayObj, &ba));

    currentSWF->writeTo(ba.bytes);
    DO_OR_FAIL("Failed to release bytearray", FREReleaseByteArray(bytearrayObj));

    return bytearrayObj;
}

FREObject BytecodeEditor::disassembleAsync()
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    runningTask = std::jthread([this] {
        if (!currentSWF)
        {
            FAIL_ASYNC("No SWF data specified to disassemble");
        }

        try
        {
            SUCCEED_ASYNC(Disassembler::Disassembler(
                ASASM::ASProgram::fromABC(ABC::ABCReader(currentSWF->abcData()).abc()))
                              .disassemble());
        }
        catch (std::exception& e)
        {
            FAIL_ASYNC(std::string("Exception during disassembly: ") + e.what());
        }
    });

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

FREObject BytecodeEditor::assembleAsync(std::unordered_map<std::string, std::string>&& strings)
{
    if (runningTask.joinable())
    {
        FAIL("Already running a task");
    }

    runningTask = std::jthread([this, strings = std::move(strings)] {
        if (!currentSWF)
        {
            FAIL_ASYNC("No SWF data specified to reassemble");
        }
        try
        {
            std::vector<uint8_t> data =
                std::move(ABC::ABCWriter(Assembler::assemble(strings).toABC()).data());

            currentSWF->replaceABCData(data.data(), data.size());

            std::vector<uint8_t> finalData(currentSWF->getFullSize());

            currentSWF->writeTo(finalData.data());

            SUCCEED_ASYNC(std::move(finalData));
        }
        catch (std::exception& e)
        {
            FAIL_ASYNC(std::string("Exception during assembly: ") + e.what());
        }
    });

    FREObject ret;
    DO_OR_FAIL("Failed to create success boolean", FRENewObjectFromBool(1, &ret));
    return ret;
}

#undef FAIL_RETURN
#define FAIL_RETURN(x)                                                                             \
    do                                                                                             \
    {                                                                                              \
        m_taskResult = std::monostate{};                                                           \
        return x;                                                                                  \
    } while (false)

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
            FREObject r;
            FREString(std::get<1>(m_taskResult), &r);
            m_taskResult = std::monostate{};
            return r;
        }
        case 2:
        {
            const auto& strings = std::get<2>(m_taskResult);

            FREObject ret;
            DO_OR_FAIL("Failed to create return object",
                FRENewObject((uint8_t*)"Object", 0, nullptr, &ret, nullptr));

            for (const auto& file : strings)
            {
                FREObject str;
                DO_OR_FAIL("Failed to create data string", FREString(file.second, &str));
                DO_OR_FAIL("Failed to set data",
                    FRESetObjectProperty(ret, (const uint8_t*)file.first.c_str(), str, nullptr));
            }

            m_taskResult = std::monostate{};
            return ret;
        }
        case 3:
        {
            const auto& data = std::get<3>(m_taskResult);

            FREObject lengthObj;
            DO_OR_FAIL(
                "Failed to create length object", FRENewObjectFromUint32(data.size(), &lengthObj));

            FREObject bytearrayObj;
            DO_OR_FAIL("Failed to create returned bytearray",
                FRENewObject(
                    (const uint8_t*)"flash.utils.ByteArray", 0, nullptr, &bytearrayObj, nullptr));

            DO_OR_FAIL("Failed to set returned bytearray length to required size",
                FRESetObjectProperty(bytearrayObj, (const uint8_t*)"length", lengthObj, nullptr));

            FREByteArray ba;
            DO_OR_FAIL("Failed to acquire bytearray", FREAcquireByteArray(bytearrayObj, &ba));

            std::copy(data.begin(), data.end(), ba.bytes);
            DO_OR_FAIL("Failed to release bytearray", FREReleaseByteArray(bytearrayObj));

            m_taskResult = std::monostate{};
            return bytearrayObj;
        }
        default:
            FAIL("Invalid task result index");
    }
}