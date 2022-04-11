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
        uint32_t maxStack = 0;
        uint32_t localCount = 0;
        uint32_t initScopeDepth = 0;
        uint32_t maxScopeDepth = 0;

        std::vector<Instruction> instructions;
        std::vector<Exception> exceptions;
        std::vector<Trait> traits;

        std::vector<ABC::Error> errors;

        bool operator==(const MethodBody& other) const = default;
        bool operator<(const MethodBody& other) const
        {
            if (maxStack < other.maxStack)
            {
                return true;
            }
            if (maxStack == other.maxStack)
            {
                if (localCount < other.localCount)
                {
                    return true;
                }
                if (localCount == other.localCount)
                {
                    if (initScopeDepth < other.initScopeDepth)
                    {
                        return true;
                    }
                    if (initScopeDepth == other.initScopeDepth)
                    {
                        if (maxScopeDepth < other.maxScopeDepth)
                        {
                            return true;
                        }
                        if (maxScopeDepth == other.maxScopeDepth)
                        {
                            if (instructions < other.instructions)
                            {
                                return true;
                            }
                            if (instructions == other.instructions)
                            {
                                if (exceptions < other.exceptions)
                                {
                                    return true;
                                }
                                if (exceptions == other.exceptions)
                                {
                                    if (traits < other.traits)
                                    {
                                        return true;
                                    }
                                    if (traits == other.traits)
                                    {
                                        return errors < other.errors;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return false;
        }
    };
}
