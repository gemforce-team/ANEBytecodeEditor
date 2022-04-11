#pragma once

#include <stdint.h>
#include <vector>

namespace ABC
{
    struct Metadata
    {
        uint32_t name = 0;
        std::vector<std::pair<uint32_t, uint32_t>> data;
    };
}
