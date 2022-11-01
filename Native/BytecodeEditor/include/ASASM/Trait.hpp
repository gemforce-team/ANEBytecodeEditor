#pragma once

#include "ASASM/Metadata.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Value.hpp"
#include "enums/TraitKind.hpp"

#include <memory>
#include <stdint.h>

namespace ASASM
{
    struct Class;
    struct Method;

    struct Trait
    {
        struct _Slot
        {
            uint32_t slotId;
            Multiname typeName;
            Value value;
            auto operator<=>(const _Slot&) const noexcept = default;
            bool operator==(const _Slot&) const noexcept  = default;
        };

        struct _Class
        {
            uint32_t slotId;
            std::shared_ptr<Class> vclass;
            auto operator<=>(const _Class&) const noexcept = default;
            bool operator==(const _Class&) const noexcept  = default;
        };

        struct _Function
        {
            uint32_t slotId;
            std::shared_ptr<Method> vfunction;
            auto operator<=>(const _Function&) const noexcept = default;
            bool operator==(const _Function&) const noexcept  = default;
        };

        struct _Method
        {
            uint32_t dispId;
            std::shared_ptr<Method> vmethod;
            auto operator<=>(const _Method&) const noexcept = default;
            bool operator==(const _Method&) const noexcept  = default;
        };

        Multiname name;
        TraitKind kind = TraitKind::Slot;
        uint8_t attributes = 0;

        std::vector<Metadata> metadata;

        [[nodiscard]] _Slot& vSlot() { return std::get<_Slot>(data); }

        [[nodiscard]] const _Slot& vSlot() const { return std::get<_Slot>(data); }

        void vSlot(const _Slot& v) { data = v; }

        [[nodiscard]] _Class& vClass() { return std::get<_Class>(data); }

        [[nodiscard]] const _Class& vClass() const { return std::get<_Class>(data); }

        void vClass(const _Class& v) { data = v; }

        [[nodiscard]] _Function& vFunction() { return std::get<_Function>(data); }

        [[nodiscard]] const _Function& vFunction() const { return std::get<_Function>(data); }

        void vFunction(const _Function& v) { data = v; }

        [[nodiscard]] _Method& vMethod() { return std::get<_Method>(data); }

        [[nodiscard]] const _Method& vMethod() const { return std::get<_Method>(data); }

        void vMethod(const _Method& v) { data = v; }

        auto operator<=>(const Trait&) const noexcept = default;
        bool operator==(const Trait&) const noexcept  = default;

    private:
        std::variant<std::monostate, _Slot, _Class, _Function, _Method> data;
    };
}
