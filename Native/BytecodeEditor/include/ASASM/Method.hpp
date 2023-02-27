#pragma once

#include "ASASM/MethodBody.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Value.hpp"

#include <optional>
#include <stdint.h>
#include <string>
#include <vector>

namespace ASASM
{
    struct Method
    {
        std::vector<Multiname> paramTypes;
        Multiname returnType;
        std::optional<std::string> name;
        uint8_t flags = 0;
        std::vector<Value> options;
        std::vector<std::optional<std::string>> paramNames;

        uint32_t id = 0;

        std::optional<MethodBody> vbody;

        std::string toString() const { return name.value_or("<unnamed_method>"); }

        auto operator<=>(const Method&) const noexcept = default;
        bool operator==(const Method&) const noexcept  = default;
    };
}
