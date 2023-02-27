#pragma once

#include <string>
#include <utility>
#include <vector>

namespace ASASM
{
    struct Metadata
    {
        std::optional<std::string> name;
        std::vector<std::pair<std::optional<std::string>, std::optional<std::string>>> data;

        auto operator<=>(const Metadata&) const noexcept = default;
        bool operator==(const Metadata&) const noexcept  = default;
    };
}
