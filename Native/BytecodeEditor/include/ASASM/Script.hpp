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

        auto operator<=>(const Script&) const noexcept = default;
        bool operator==(const Script&) const noexcept  = default;
    };
}
