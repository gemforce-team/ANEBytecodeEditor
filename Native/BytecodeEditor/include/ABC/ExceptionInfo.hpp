#pragma once

#include "ABC/Label.hpp"
#include <stdint.h>

namespace ABC
{
    struct ExceptionInfo
    {
        Label from, to, target;
        uint32_t excType = 0;
        uint32_t varName = 0;
    };
}
