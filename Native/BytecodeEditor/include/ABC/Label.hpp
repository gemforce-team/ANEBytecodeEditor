#pragma once

#include <stdint.h>

namespace ABC
{
    struct Label
    {
        uint32_t index           = 0;
        int32_t offset           = 0;
        ptrdiff_t absoluteOffset = 0;

        std::strong_ordering operator<=>(const Label&) const = default;
    };
}
