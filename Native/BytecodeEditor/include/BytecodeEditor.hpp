#pragma once

#include "ASASM/ASProgram.hpp"
#include "SWF/SWFFile.hpp"
#include "utils/ANEUtils.hpp"
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

#include <windows.h>

#include <FlashRuntimeExtensions.h>

class BytecodeEditor
{
private:
    FREContext ctx;

    std::optional<SWF::SWFFile> currentSWF;

    std::jthread runningTask;

    std::variant<std::monostate, std::string, std::unordered_map<std::string, std::string>,
        std::vector<uint8_t>>
        m_taskResult;

    std::unique_ptr<ASASM::ASProgram> partialAssembly;

public:
    BytecodeEditor(FREContext ctx) noexcept : ctx(ctx) {}

    FREObject disassemble();
    FREObject disassembleAsync();

    FREObject assemble(
        std::unordered_map<std::string, std::string>&& data, bool includeDebugInstructions);
    FREObject assembleAsync(
        std::unordered_map<std::string, std::string>&& data, bool includeDebugInstructions);

    FREObject partialAssemble(
        std::unordered_map<std::string, std::string>&& data, bool includeDebugInstructions);
    FREObject partialAssembleAsync(
        std::unordered_map<std::string, std::string>&& data, bool includeDebugInstructions);
    FREObject finishAssemble();
    FREObject finishAssembleAsync();

    FREObject beginIntrospection();

    ASASM::Class* getClass(const ASASM::Multiname& className) const;
    ASASM::Script* getScript(const ASASM::Multiname& trait) const;

    void setSWF(SWF::SWFFile&& file) { currentSWF = std::move(file); }

    void cleanup()
    {
        if (runningTask.joinable())
        {
            runningTask.join();
        }
        currentSWF      = std::nullopt;
        partialAssembly = nullptr;
        m_taskResult    = std::monostate{};
    }

    FREObject taskResult();
};
