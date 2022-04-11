#pragma once

#include "ASASM/Namespace.hpp"
#include "enums/ABCType.hpp"
#include "utils/StringException.hpp"

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
            bool operator==(const _QName& other) const = default;
            bool operator<(const _QName& other) const
            {
                if (ns < other.ns)
                {
                    return true;
                }
                if (ns == other.ns)
                {
                    return name < other.name;
                }
                return false;
            }
        };
        struct _RTQName
        {
            std::string name;
            bool operator==(const _RTQName& other) const = default;
            bool operator<(const _RTQName& other) const { return name < other.name; }
        };
        struct _RTQNameL
        {
            bool operator==(const _RTQNameL&) const { return true; }
            bool operator<(const _RTQNameL&) const { return false; }
        };
        struct _Multiname
        {
            std::string name;
            std::vector<Namespace> nsSet;
            bool operator==(const _Multiname& other) const = default;
            bool operator<(const _Multiname& other) const
            {
                if (name < other.name)
                {
                    return true;
                }
                if (name == other.name)
                {
                    return nsSet < other.nsSet;
                }
                return false;
            }
        };
        struct _MultinameL
        {
            std::vector<Namespace> nsSet;
            bool operator==(const _MultinameL& other) const = default;
            bool operator<(const _MultinameL& other) const { return nsSet < other.nsSet; }
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

            // std::weak_ordering operator<=>(const _Typename& other) const
            // {
            //     if (auto cmp = (name() <=> other.name()); cmp != 0)
            //     {
            //         return cmp;
            //     }
            //     return params() <=> other.params();
            // }

            bool operator==(const _Typename& other) const
            {
                return name() == other.name() && params() == other.params();
            }
            bool operator<(const _Typename& other) const
            {
                if (name() < other.name())
                {
                    return true;
                }
                if (name() == other.name())
                {
                    return params() < other.params();
                }
                return false;
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
        void multiname(const _Multiname& v) { data = v; }
        [[nodiscard]] const _MultinameL& multinamel() const { return std::get<_MultinameL>(data); }
        [[nodiscard]] _MultinameL& multinamel() { return std::get<_MultinameL>(data); }
        void multinamel(const _MultinameL& v) { data = v; }
        [[nodiscard]] const _Typename& Typename() const { return std::get<_Typename>(data); }
        [[nodiscard]] _Typename& Typename() { return std::get<_Typename>(data); }
        void Typename(const _Typename& v) { data = v; }

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

        bool operator==(const Multiname& m) const { return data == m.data; }
        bool operator<(const Multiname& m) const { return data < m.data; }

    private:
        std::variant<std::monostate, _QName, _RTQName, _RTQNameL, _Multiname, _MultinameL,
            _Typename>
            data;
    };
}
