#pragma once

#include "utils/BidirectionalMap.hpp"
#include <stdint.h>

enum class TraitKind : uint8_t
{
    Slot     = 0,
    Method   = 1,
    Getter   = 2,
    Setter   = 3,
    Class    = 4,
    Function = 5,
    Const    = 6
};

inline constexpr BidirectionalMap TraitKindMap = {"slot", TraitKind::Slot, "method",
    TraitKind::Method, "getter", TraitKind::Getter, "setter", TraitKind::Setter, "class",
    TraitKind::Class, "function", TraitKind::Function, "const", TraitKind::Const};
