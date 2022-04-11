#pragma once

#include "ABC/Label.hpp"
#include <string>

namespace ABC
{
    struct Error
    {
        Label loc;
        std::string message;

        std::strong_ordering operator<=>(const Error&) const = default;
    };
}
