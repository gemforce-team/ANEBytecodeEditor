#pragma once

#include "ASASM/Namespace.hpp"
#include "enums/ABCType.hpp"
#include "utils/StringException.hpp"

#include <algorithm>
#include <memory>
#include <stdint.h>
#include <variant>
#include <vector>

namespace ASASM
{
    struct Multiname
    {
        struct _QName
        {
            Namespace ns;
            std::string name;

            auto operator<=>(const _QName&) const noexcept = default;
            bool operator==(const _QName&) const noexcept  = default;
        };

        struct _RTQName
        {
            std::string name;
            auto operator<=>(const _RTQName&) const noexcept = default;
            bool operator==(const _RTQName&) const noexcept  = default;
        };

        struct _RTQNameL
        {
            auto operator<=>(const _RTQNameL&) const noexcept = default;
            bool operator==(const _RTQNameL&) const noexcept  = default;
        };

        struct _Multiname
        {
            std::string name;
            std::vector<Namespace> nsSet;
            auto operator<=>(const _Multiname&) const noexcept = default;
            bool operator==(const _Multiname&) const noexcept  = default;
        };

        struct _MultinameL
        {
            std::vector<Namespace> nsSet;
            auto operator<=>(const _MultinameL&) const noexcept = default;
            bool operator==(const _MultinameL&) const noexcept  = default;
        };

        struct _Typename
        {
        private:
            std::unique_ptr<Multiname> _name;
            std::unique_ptr<std::vector<Multiname>> _params;

        public:
            _Typename()
                : _name(std::make_unique<Multiname>()),
                  _params(std::make_unique<std::vector<Multiname>>())
            {
            }

            explicit _Typename(const Multiname& n)
                : _name(std::make_unique<Multiname>(n)),
                  _params(std::make_unique<std::vector<Multiname>>())
            {
            }

            _Typename(const Multiname& n, const std::vector<Multiname>& params)
                : _name(std::make_unique<Multiname>(n)),
                  _params(std::make_unique<std::vector<Multiname>>(params))
            {
            }

            _Typename(const _Typename& other) : _Typename(*other._name, *other._params) {}

            _Typename(_Typename&& other) noexcept
                : _name(std::move(other._name)), _params(std::move(other._params))
            {
            }

            _Typename& operator=(const _Typename& other)
            {
                *_name   = *other._name;
                *_params = *other._params;
                return *this;
            }

            _Typename& operator=(_Typename&& other) noexcept
            {
                _name   = std::move(other._name);
                _params = std::move(other._params);
                return *this;
            }

            ~_Typename() = default;

            const Multiname& name() const { return *_name; }

            Multiname& name() { return *_name; }

            const std::vector<Multiname>& params() const { return *_params; }

            std::vector<Multiname>& params() { return *_params; }

            std::strong_ordering operator<=>(const _Typename& other) const noexcept
            {
                if (auto cmp = (name() <=> other.name()); cmp != 0)
                {
                    return cmp;
                }
                return params() <=> other.params();
            }

            bool operator==(const _Typename& other) const noexcept
            {
                return name() == other.name() && params() == other.params();
            }
        };

        ABCType kind = ABCType::Void;

        [[nodiscard]] const _QName& qname() const { return std::get<_QName>(data); }

        [[nodiscard]] _QName& qname() { return std::get<_QName>(data); }

        void qname(const _QName& v) { data = v; }

        [[nodiscard]] const _RTQName& rtqname() const { return std::get<_RTQName>(data); }

        [[nodiscard]] _RTQName& rtqname() { return std::get<_RTQName>(data); }

        void rtqname(const _RTQName& v) { data = v; }

        [[nodiscard]] const _RTQNameL& rtqnamel() const { return std::get<_RTQNameL>(data); }

        [[nodiscard]] _RTQNameL& rtqnamel() { return std::get<_RTQNameL>(data); }

        void rtqnamel(const _RTQNameL& v) { data = v; }

        [[nodiscard]] const _Multiname& multiname() const { return std::get<_Multiname>(data); }

        [[nodiscard]] _Multiname& multiname() { return std::get<_Multiname>(data); }

        void multiname(const _Multiname& v)
        {
            data = v;
            std::sort(
                std::get<_Multiname>(data).nsSet.begin(), std::get<_Multiname>(data).nsSet.end());
        }

        [[nodiscard]] const _MultinameL& multinamel() const { return std::get<_MultinameL>(data); }

        [[nodiscard]] _MultinameL& multinamel() { return std::get<_MultinameL>(data); }

        void multinamel(const _MultinameL& v)
        {
            data = v;
            std::sort(
                std::get<_MultinameL>(data).nsSet.begin(), std::get<_MultinameL>(data).nsSet.end());
        }

        [[nodiscard]] const _Typename& Typename() const { return std::get<_Typename>(data); }

        [[nodiscard]] _Typename& Typename() { return std::get<_Typename>(data); }

        void Typename(const _Typename& v)
        {
            data = v;
            std::sort(std::get<_Typename>(data).params().begin(),
                std::get<_Typename>(data).params().end());
        }

        std::vector<Multiname> toQNames() const
        {
            switch (kind)
            {
                case ABCType::QName:
                case ABCType::QNameA:
                    return {*this};
                case ABCType::Multiname:
                case ABCType::MultinameA:
                {
                    std::vector<Multiname> ret;
                    for (const auto& ns : multiname().nsSet)
                    {
                        Multiname add;
                        add.kind = kind == ABCType::Multiname ? ABCType::QName : ABCType::QNameA;
                        add.qname({ns, multiname().name});
                        ret.emplace_back(std::move(add));
                    }
                    return ret;
                }
                default:
                    throw StringException("Can't expand Multiname of this kind");
            }
        }

        std::strong_ordering operator<=>(const Multiname& m) const noexcept = default;

        bool operator==(const Multiname&) const noexcept = default;

    private:
        std::variant<std::monostate, _QName, _RTQName, _RTQNameL, _Multiname, _MultinameL,
            _Typename>
            data;
    };
}
