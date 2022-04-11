#pragma once

#include "enums/ABCType.hpp"

#include <stdint.h>
#include <string>

namespace ASASM
{
    struct Namespace
    {
        ABCType kind = ABCType::Void;
        std::string name;

        int id = 0;

        bool operator==(const Namespace& other) const = default;
        bool operator<(const Namespace& other) const
        {
            if (kind < other.kind)
            {
                return true;
            }
            if (kind == other.kind)
            {
                if (name < other.name)
                {
                    return true;
                }
                if (name == other.name)
                {
                    return id < other.id;
                }
            }
            return false;
        }
    };
}
