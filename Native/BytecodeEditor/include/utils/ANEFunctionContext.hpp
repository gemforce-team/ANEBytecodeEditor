#pragma once

#include "ASASM/ASProgram.hpp"
#include "ASASM/Class.hpp"
#include "ASASM/Script.hpp"
#include "BytecodeEditor.hpp"
#include <memory>
#include <optional>
#include <variant>

struct ANEFunctionContext
{
    std::shared_ptr<BytecodeEditor> editor;
    std::unique_ptr<FRENamedFunction[]> functions;

    struct ObjectData
    {
        std::variant<std::shared_ptr<ASASM::Class>, std::shared_ptr<ASASM::Script>> object;
        BytecodeEditor::PartialAssembly* program;
    };

    std::optional<ObjectData> objectData;
};
