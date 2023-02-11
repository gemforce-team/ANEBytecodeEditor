#pragma once

#include "ABC/Error.hpp"
#include "ABC/ExceptionInfo.hpp"
#include "ABC/Instruction.hpp"
#include "ABC/TraitsInfo.hpp"
#include <stdint.h>
#include <vector>

namespace SWFABC
{
    struct MethodBody
    {
        uint32_t method = 0, maxStack = 0, localCount = 0, initScopeDepth = 0, maxScopeDepth = 0;
        std::vector<Instruction> instructions;
        std::vector<ExceptionInfo> exceptions;
        std::vector<TraitsInfo> traits;
        std::vector<Error> errors;
    };
}
