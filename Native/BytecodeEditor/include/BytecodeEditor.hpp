#pragma once

#include "SWF/SWFFile.hpp"
#include "utils/ANEUtils.hpp"
#include <FlashRuntimeExtensions.h>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

class BytecodeEditor
{
private:
    FREContext ctx;

    std::optional<SWF::SWFFile> currentSWF;

    std::jthread runningTask;

    std::variant<std::monostate, std::string, std::unordered_map<std::string, std::string>,
        std::vector<uint8_t>>
        m_taskResult;

public:
    BytecodeEditor(FREContext ctx) noexcept : ctx(ctx) {}

    FREObject disassemble();
    FREObject disassembleAsync();

    FREObject assemble(std::unordered_map<std::string, std::string>&& data, bool includeDebugInstructions);
    FREObject assembleAsync(std::unordered_map<std::string, std::string>&& data, bool includeDebugInstructions);

    void setSWF(SWF::SWFFile&& file) { currentSWF = std::move(file); }

    void cleanup()
    {
        if (runningTask.joinable())
        {
            runningTask.join();
        }
        currentSWF   = std::nullopt;
        m_taskResult = std::monostate{};
    }

    FREObject taskResult();
};
