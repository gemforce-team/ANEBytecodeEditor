#pragma once

#include "ABC/Error.hpp"
#include "ASASM/Exception.hpp"
#include "ASASM/Instruction.hpp"
#include "ASASM/Trait.hpp"

#include <memory>
#include <stdint.h>
#include <vector>

namespace ASASM
{
    struct Method;

    struct MethodBody
    {
        std::weak_ptr<Method> method;
        uint32_t maxStack       = 0;
        uint32_t localCount     = 0;
        uint32_t initScopeDepth = 0;
        uint32_t maxScopeDepth  = 0;

        std::vector<Instruction> instructions;
        std::vector<Exception> exceptions;
        std::vector<Trait> traits;

        std::vector<SWFABC::Error> errors;

        auto operator<=>(const MethodBody&) const noexcept = default;
        bool operator==(const MethodBody&) const noexcept  = default;
    };
}
