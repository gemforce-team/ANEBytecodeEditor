#pragma once

#include <string>
#include <utility>
#include <vector>

namespace ASASM
{
    struct Metadata
    {
        std::string name;
        std::vector<std::pair<std::string, std::string>> data;

        auto operator<=>(const Metadata&) const noexcept = default;
        bool operator==(const Metadata&) const noexcept  = default;
    };
}
