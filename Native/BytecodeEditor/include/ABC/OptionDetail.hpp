#pragma once

#include "enums/ABCType.hpp"
#include <stdint.h>

namespace ABC
{
    struct OptionDetail
    {
        uint32_t value = 0;
        ABCType kind;
    };
}
