#pragma once

#include "ASASM/ASProgram.hpp"
#include "SWF/SWFFile.hpp"
#include "utils/ANEUtils.hpp"
#include "utils/RefBuilder.hpp"
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

#include <windows.h>

#include <FlashRuntimeExtensions.h>

class BytecodeEditor : public std::enable_shared_from_this<BytecodeEditor>
{
private:
    FREContext ctx;

    std::jthread runningTask;

    std::variant<std::monostate, std::string, std::unordered_map<std::string, std::string>,
        std::vector<uint8_t>>
        m_taskResult;

public:
    struct PartialAssembly
    {
        ASASM::ASProgram program;
        RefBuilder namespaceResolver;
        std::vector<std::string> extraNamespaceData;
    };

    std::unique_ptr<PartialAssembly> partialAssembly;

    BytecodeEditor(FREContext ctx) noexcept : ctx(ctx) {}

    ~BytecodeEditor() noexcept
    {
        if (runningTask.joinable())
        {
            runningTask.join();
        }
    }

    FREObject disassemble(std::span<const uint8_t> swf);
    FREObject disassembleAsync(std::span<const uint8_t> swf);

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

    FREObject beginIntrospection(std::span<const uint8_t> swf);

    FREObject listScripts() const;
    FREObject listClasses() const;

    std::shared_ptr<ASASM::Class> getClass(const ASASM::Multiname& className) const;
    std::shared_ptr<ASASM::Script> getScript(const ASASM::Multiname& trait) const;

    std::shared_ptr<ASASM::Script> createScript(const std::shared_ptr<ASASM::Method>& sinit);

    void cleanup()
    {
        if (runningTask.joinable())
        {
            runningTask.join();
        }
        partialAssembly = nullptr;
        m_taskResult    = std::monostate{};
    }

    FREObject taskResult();

    std::shared_ptr<ASASM::Class> ConvertClass(FREObject o) const;
    FREObject ConvertClass(const std::shared_ptr<ASASM::Class>& c) const;
    ASASM::Namespace ConvertNamespace(FREObject o) const;
    FREObject ConvertNamespace(const ASASM::Namespace& o) const;
    ASASM::Multiname ConvertMultiname(FREObject o) const;
    FREObject ConvertMultiname(const ASASM::Multiname& o) const;
    ASASM::Trait ConvertTrait(FREObject o) const;
    FREObject ConvertTrait(const ASASM::Trait& t) const;
    std::shared_ptr<ASASM::Method> ConvertMethod(FREObject o) const;
    FREObject ConvertMethod(const ASASM::Method& m) const;
    ASASM::MethodBody ConvertMethodBody(FREObject o) const;
    FREObject ConvertMethodBody(const ASASM::MethodBody& b) const;
    ASASM::Exception ConvertException(FREObject o, const std::vector<FREObject>& allInstrs) const;
    FREObject ConvertException(
        const ASASM::Exception& e, const std::vector<FREObject>& allInstrs) const;
    SWFABC::Error ConvertError(FREObject o, const std::vector<FREObject>& allInstrs) const;
    FREObject ConvertError(const SWFABC::Error& e, const std::vector<FREObject>& allInstrs) const;
    SWFABC::Label ConvertLabel(FREObject o, const std::vector<FREObject>& allInstrs) const;
    FREObject ConvertLabel(const SWFABC::Label& l, const std::vector<FREObject>& allInstrs) const;
    ASASM::Instruction ConvertInstruction(
        FREObject o, const std::vector<FREObject>& allInstrs) const;
    std::pair<FREObject, bool> ConvertInstruction(const ASASM::Instruction& i) const;
    ASASM::Value ConvertValue(FREObject o) const;
    FREObject ConvertValue(const ASASM::Value& v) const;
};
