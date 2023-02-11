#pragma once

#include "enums/ABCType.hpp"
#include <stdint.h>

namespace SWFABC
{
    struct OptionDetail
    {
        uint32_t value = 0;
        ABCType kind;
    };
}
