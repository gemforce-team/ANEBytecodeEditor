#pragma once

#include "ABC/TraitsInfo.hpp"
#include <stdint.h>
#include <vector>

namespace SWFABC
{
    struct Class
    {
        uint32_t cinit = 0;
        std::vector<TraitsInfo> traits;
    };
}
