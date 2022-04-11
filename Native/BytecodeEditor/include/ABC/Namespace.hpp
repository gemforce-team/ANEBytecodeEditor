#pragma once

#include "enums/ABCType.hpp"
#include <stdint.h>

namespace ABC
{
    struct Namespace
    {
        ABCType kind;
        uint32_t name = 0;
    };
}
