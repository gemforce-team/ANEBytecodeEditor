#pragma once

#include "ABC/TraitsInfo.hpp"
#include <stdint.h>
#include <vector>

namespace ABC
{
    struct Script
    {
        uint32_t sinit = 0;
        std::vector<TraitsInfo> traits;
    };
}
