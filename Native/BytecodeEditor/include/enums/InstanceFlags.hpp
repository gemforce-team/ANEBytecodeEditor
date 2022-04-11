#pragma once

#include "utils/BidirectionalMap.hpp"
#include <stdint.h>

enum class InstanceFlags : uint8_t
{
    Sealed = 0x01, // The class is sealed: properties can not be dynamically added to instances of
                   // the class.
    Final       = 0x02, // The class is final: it cannot be a base class for any other class.
    Interface   = 0x04, // The class is an interface.
    ProtectedNs = 0x08, // The class uses its protected namespace and the protectedNs field is
                        // present in the interface_info structure.
};

constexpr inline BidirectionalMap InstanceFlagMap = MakeCStringMap(std::equal_to<>(), "SEALED",
    InstanceFlags::Sealed, "FINAL", InstanceFlags::Final, "INTERFACE", InstanceFlags::Interface,
    "PROTECTEDNS", InstanceFlags::ProtectedNs);
