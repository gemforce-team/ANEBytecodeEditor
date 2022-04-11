#pragma once

#include "ABC/OptionDetail.hpp"
#include <stdint.h>
#include <vector>

namespace ABC
{
    struct MethodInfo
    {
        std::vector<uint32_t> paramTypes;
        uint32_t returnType = 0;
        uint32_t name       = 0;
        std::vector<OptionDetail> options;
        std::vector<uint32_t> paramNames;
        uint8_t flags = 0;
    };
}
