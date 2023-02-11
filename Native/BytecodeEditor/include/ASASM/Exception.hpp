#pragma once

#include "ABC/Label.hpp"
#include "ASASM/Multiname.hpp"

namespace ASASM
{
    struct Exception
    {
        SWFABC::Label from, to, target;
        Multiname excType;
        Multiname varName;

        auto operator<=>(const Exception&) const noexcept = default;
        bool operator==(const Exception&) const noexcept  = default;
    };
}
