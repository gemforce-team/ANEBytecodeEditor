#pragma once

#include "ASASM/Namespace.hpp"
#include "enums/ABCType.hpp"

#include <variant>

namespace ASASM
{
    struct Value
    {
        ABCType vkind = ABCType::Void;

        [[nodiscard]] int64_t& vint() { return std::get<int64_t>(data); }

        [[nodiscard]] const int64_t& vint() const { return std::get<int64_t>(data); }

        void vint(int64_t intv) { data = intv; }

        [[nodiscard]] uint64_t& vuint() { return std::get<uint64_t>(data); }

        [[nodiscard]] const uint64_t& vuint() const { return std::get<uint64_t>(data); }

        void vuint(uint64_t uintv) { data = uintv; }

        [[nodiscard]] double& vdouble() { return std::get<double>(data); }

        [[nodiscard]] const double& vdouble() const { return std::get<double>(data); }

        void vdouble(const double& v) { data = v; }

        [[nodiscard]] std::string& vstring() { return std::get<std::string>(data); }

        [[nodiscard]] const std::string& vstring() const { return std::get<std::string>(data); }

        void vstring(const std::string& v) { data = v; }

        [[nodiscard]] Namespace& vnamespace() { return std::get<Namespace>(data); }

        [[nodiscard]] const Namespace& vnamespace() const { return std::get<Namespace>(data); }

        void vnamespace(const Namespace& v) { data = v; }

        auto operator<=>(const Value& other) const noexcept
        {
            if (data.valueless_by_exception() && other.data.valueless_by_exception())
            {
                return std::strong_ordering::equal;
            }
            if (data.valueless_by_exception() && !other.data.valueless_by_exception())
            {
                return std::strong_ordering::less;
            }
            if (!data.valueless_by_exception() && other.data.valueless_by_exception())
            {
                return std::strong_ordering::greater;
            }
            if (auto cmp = data.index() <=> other.data.index(); cmp != 0)
            {
                return cmp;
            }
            switch (data.index())
            {
                case 0:
                default:
                    return std::strong_ordering::equal;
                case 1:
                    return std::get<1>(data) <=> std::get<1>(other.data);
                case 2:
                    return std::get<2>(data) <=> std::get<2>(other.data);
                case 3:
                    return std::bit_cast<uint64_t>(std::get<3>(data)) <=>
                           std::bit_cast<uint64_t>(std::get<3>(other.data));
                case 4:
                    return std::get<4>(data) <=> std::get<4>(other.data);
                case 5:
                    return std::get<5>(data) <=> std::get<5>(other.data);
            }
        }

        bool operator==(const Value&) const noexcept = default;

    private:
        std::variant<std::monostate, int64_t, uint64_t, double, std::string, Namespace> data;
    };
}
