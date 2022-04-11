#pragma once

#include "ABC/TraitsInfo.hpp"
#include <stdint.h>
#include <vector>

namespace ABC
{
    struct Instance
    {
        uint32_t name        = 0;
        uint32_t superName   = 0;
        uint32_t protectedNs = 0;
        std::vector<uint32_t> interfaces;
        uint32_t iinit = 0;
        std::vector<TraitsInfo> traits;
        uint8_t flags = 0;
    };
}
