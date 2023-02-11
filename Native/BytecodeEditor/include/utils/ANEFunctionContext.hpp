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
        std::variant<ASASM::Class*, ASASM::Script*> object;
        std::pair<ASASM::ASProgram, RefBuilder>* program;
    };

    std::optional<ObjectData> objectData;
};
