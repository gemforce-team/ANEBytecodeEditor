#pragma once

#include "utils/BidirectionalMap.hpp"
#include <stdint.h>

enum class MethodFlags : uint8_t
{
    NEED_ARGUMENTS = 0x01,  // Suggests to the run-time that an "arguments" object (as specified by
                            // the ActionScript 3.0 Language Reference) be created. Must not be used
                            // together with NEED_REST. See Chapter 3.
    NEED_ACTIVATION = 0x02, // Must be set if this method uses the newactivation opcode.
    NEED_REST = 0x04,    // This flag creates an ActionScript 3.0 rest arguments array. Must not be
                         // used with NEED_ARGUMENTS. See Chapter 3.
    HAS_OPTIONAL = 0x08, // Must be set if this method has optional parameters and the options field
                         // is present in this method_info structure.
    SET_DXNS = 0x40,     // Must be set if this method uses the dxns or dxnslate opcodes.
    HAS_PARAM_NAMES =
        0x80 // Must be set when the param_names field is present in this method_info structure.
};

constexpr inline BidirectionalMap MethodFlagMap = MakeCStringMap(std::equal_to<>(),
    "NEED_ARGUMENTS", MethodFlags::NEED_ARGUMENTS, "NEED_ACTIVATION", MethodFlags::NEED_ACTIVATION,
    "NEED_REST", MethodFlags::NEED_REST, "HAS_OPTIONAL", MethodFlags::HAS_OPTIONAL, "SET_DXNS",
    MethodFlags::SET_DXNS, "HAS_PARAM_NAMES", MethodFlags::HAS_PARAM_NAMES);
