#pragma once

#include "enums/ABCType.hpp"
#include "enums/TraitKind.hpp"
#include <stdint.h>
#include <vector>

namespace ABC
{
    struct TraitsInfo
    {
        uint32_t name    = 0;
        uint8_t kindAttr = 0;
        union
        {
            struct
            {
                uint32_t slotId, typeName, vindex;
                ABCType vkind;
            } Slot;
            struct
            {
                uint32_t slotId, classi;
            } Class;
            struct
            {
                uint32_t slotId, functioni;
            } Function;
            struct
            {
                uint32_t dispId, method;
            } Method;
        };
        std::vector<uint32_t> metadata;

        [[nodiscard]] TraitKind kind() const { return TraitKind(kindAttr & 0xF); }
        void kind(TraitKind k) { kindAttr = (kindAttr & 0xF0) | uint8_t(k); }
        [[nodiscard]] uint8_t attr() const { return kindAttr >> 4; }
        void attr(uint8_t a) { kindAttr = (kindAttr & 0xF) | uint8_t(a << 4); }
    };
}
