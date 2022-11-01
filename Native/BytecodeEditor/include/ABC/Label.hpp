#pragma once

#include <stdint.h>
#include <compare>

namespace ABC
{
    struct Label
    {
        uint32_t index           = 0;
        int32_t offset           = 0;
        ptrdiff_t absoluteOffset = 0;

        auto operator<=>(const Label&) const noexcept = default;
        bool operator==(const Label&) const noexcept  = default;
    };
}
