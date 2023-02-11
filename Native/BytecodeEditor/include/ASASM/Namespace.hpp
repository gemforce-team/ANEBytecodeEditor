#pragma once

#include "enums/ABCType.hpp"
#include "utils/generic_hash.hpp"

#include <stdint.h>
#include <string>

namespace ASASM
{
    struct Namespace
    {
        ABCType kind = ABCType::Void;
        std::string name;

        int id = 0;

        auto operator<=>(const Namespace&) const noexcept = default;
        bool operator==(const Namespace&) const noexcept  = default;
    };
}

DEFINE_HASH(
    ASASM::Namespace, &ASASM::Namespace::kind, &ASASM::Namespace::name, &ASASM::Namespace::id);
