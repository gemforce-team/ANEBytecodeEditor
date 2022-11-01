#pragma once

#include "utils/BidirectionalMap.hpp"
#include <stdint.h>

enum class TraitAttribute : uint8_t
{
    Final    = 1,
    Override = 2,
    Metadata = 4
};

constexpr inline BidirectionalMap TraitAttributeMap = {"FINAL", TraitAttribute::Final, "OVERRIDE",
    TraitAttribute::Override, "METADATA", TraitAttribute::Metadata};
