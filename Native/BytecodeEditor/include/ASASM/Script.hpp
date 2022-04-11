#pragma once

#include "ASASM/Method.hpp"
#include "ASASM/Trait.hpp"

#include <memory>
#include <vector>

namespace ASASM
{
    struct Script
    {
        std::shared_ptr<Method> sinit;
        std::vector<Trait> traits;

        bool operator==(const Script& other) const = default;
        bool operator<(const Script& other) const
        {
            if (sinit < other.sinit)
            {
                return true;
            }
            if (sinit == other.sinit)
            {
                return traits < traits;
            }
            return false;
        }
    };
}
