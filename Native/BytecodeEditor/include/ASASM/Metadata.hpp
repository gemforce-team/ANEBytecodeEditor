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

        bool operator==(const Metadata& other) const = default;
        bool operator<(const Metadata& other) const
        {
            if (name < other.name)
            {
                return true;
            }
            if (name == other.name)
            {
                return data < other.data;
            }
            return false;
        }
    };
}
