#pragma once

#include "ASASM/MethodBody.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Value.hpp"

#include <optional>
#include <stdint.h>
#include <string>
#include <vector>

namespace ASASM
{
    struct Method
    {
        std::vector<Multiname> paramTypes;
        Multiname returnType;
        std::string name;
        uint8_t flags = 0;
        std::vector<Value> options;
        std::vector<std::string> paramNames;

        uint32_t id = 0;

        std::optional<MethodBody> vbody;

        std::string toString() const { return name; }

        bool operator==(const Method& other) const = default;
        bool operator<(const Method& other) const
        {
            if (paramTypes < other.paramTypes)
            {
                return true;
            }
            if (paramTypes == other.paramTypes)
            {
                if (returnType < other.returnType)
                {
                    return true;
                }
                if (returnType == other.returnType)
                {
                    if (name < other.name)
                    {
                        return true;
                    }
                    if (name == other.name)
                    {
                        if (flags < other.flags)
                        {
                            return true;
                        }
                        if (flags == other.flags)
                        {
                            if (options < other.options)
                            {
                                return true;
                            }
                            if (options == other.options)
                            {
                                if (paramNames < other.paramNames)
                                {
                                    return true;
                                }
                                if (paramNames == other.paramNames)
                                {
                                    if (id < other.id)
                                    {
                                        return true;
                                    }
                                    if (id == other.id)
                                    {
                                        return vbody < other.vbody;
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
