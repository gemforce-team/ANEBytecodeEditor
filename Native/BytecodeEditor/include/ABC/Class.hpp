#pragma once

#include "ABC/TraitsInfo.hpp"
#include <stdint.h>
#include <vector>

namespace ABC
{
    struct Class
    {
        uint32_t cinit = 0;
        std::vector<TraitsInfo> traits;
    };
}
