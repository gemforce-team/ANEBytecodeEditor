#pragma once

#include "enums/ABCType.hpp"
#include <stdint.h>
#include <variant>
#include <vector>

namespace ABC
{
    struct Multiname
    {
        struct _QName
        {
            uint32_t ns, name;
        };
        struct _RTQName
        {
            uint32_t name;
        };
        struct _RTQNameL
        {
        };
        struct _Multiname
        {
            uint32_t name, nsSet;
        };
        struct _MultinameL
        {
            uint32_t nsSet;
        };
        struct _Typename
        {
            uint32_t name;
            std::vector<uint32_t> params;
        };

        ABCType kind = ABCType::Void;

        [[nodiscard]] const _QName& qname() const { return std::get<_QName>(data); }
        void qname(const _QName& v) { data = v; }
        [[nodiscard]] const _RTQName& rtqname() const { return std::get<_RTQName>(data); }
        void rtqname(const _RTQName& v) { data = v; }
        [[nodiscard]] const _RTQNameL& rtqnamel() const { return std::get<_RTQNameL>(data); }
        void rtqnamel(const _RTQNameL& v) { data = v; }
        [[nodiscard]] const _Multiname& multiname() const { return std::get<_Multiname>(data); }
        void multiname(const _Multiname& v) { data = v; }
        [[nodiscard]] const _MultinameL& multinamel() const { return std::get<_MultinameL>(data); }
        void multinamel(const _MultinameL& v) { data = v; }
        [[nodiscard]] const _Typename& Typename() const { return std::get<_Typename>(data); }
        void Typename(const _Typename& v) { data = v; }
        void Typename(_Typename&& v) { data = std::move(v); }

    private:
        std::variant<std::monostate, _QName, _RTQName, _RTQNameL, _Multiname, _MultinameL,
            _Typename>
            data;
    };
}
