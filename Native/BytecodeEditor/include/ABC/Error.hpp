#pragma once

#include "ABC/Label.hpp"
#include <string>

namespace SWFABC
{
    struct Error
    {
        Label loc;
        std::string message;

        auto operator<=>(const Error&) const noexcept = default;
        bool operator==(const Error&) const noexcept  = default;
    };
}
