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
            bool operator==(const _Slot& other) const = default;
            bool operator<(const _Slot& other) const
            {
                if (slotId < other.slotId)
                {
                    return true;
                }
                if (slotId == other.slotId)
                {
                    if (typeName < other.typeName)
                    {
                        return true;
                    }
                    if (typeName == other.typeName)
                    {
                        return value < other.value;
                    }
                }
                return false;
            }
        };
        struct _Class
        {
            uint32_t slotId;
            std::shared_ptr<Class> vclass;
            bool operator==(const _Class& other) const = default;
            bool operator<(const _Class& other) const
            {
                if (slotId < other.slotId)
                {
                    return true;
                }
                if (slotId == other.slotId)
                {
                    return vclass < other.vclass;
                }
                return false;
            }
        };
        struct _Function
        {
            uint32_t slotId;
            std::shared_ptr<Method> vfunction;
            bool operator==(const _Function& other) const = default;
            bool operator<(const _Function& other) const
            {
                if (slotId < other.slotId)
                {
                    return true;
                }
                if (slotId == other.slotId)
                {
                    return vfunction < other.vfunction;
                }
                return false;
            }
        };
        struct _Method
        {
            uint32_t dispId;
            std::shared_ptr<Method> vmethod;
            bool operator==(const _Method& other) const = default;
            bool operator<(const _Method& other) const
            {
                if (dispId < other.dispId)
                {
                    return true;
                }
                if (dispId == other.dispId)
                {
                    return vmethod < other.vmethod;
                }
                return false;
            }
        };

        Multiname name;
        TraitKind kind;
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

        bool operator==(const Trait& other) const = default;
        bool operator<(const Trait& other) const
        {
            if (name < other.name)
            {
                return true;
            }
            if (name == other.name)
            {
                if (kind < other.kind)
                {
                    return true;
                }
                if (kind == other.kind)
                {
                    if (attributes < other.attributes)
                    {
                        return true;
                    }
                    if (attributes == other.attributes)
                    {
                        if (metadata < other.metadata)
                        {
                            return true;
                        }
                        if (metadata == other.metadata)
                        {
                            return data < other.data;
                        }
                    }
                }
            }
            return false;
        }

    private:
        std::variant<std::monostate, _Slot, _Class, _Function, _Method> data;
    };
}
