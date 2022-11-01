#pragma once

#include "ABC/Label.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Namespace.hpp"
#include "enums/OPCode.hpp"

#include <memory>
#include <stdint.h>
#include <string>
#include <variant>

namespace ASASM
{
    struct Class;
    struct Method;

    struct Instruction
    {
        OPCode opcode = OPCode::OP_raw;

        struct Argument
        {
        private:
            std::variant<std::monostate, int8_t, uint8_t, int64_t, uint64_t, double, std::string,
                Namespace, Multiname, std::shared_ptr<Class>, std::shared_ptr<Method>, ABC::Label,
                std::vector<ABC::Label>>
                data;

        public:
            [[nodiscard]] int8_t& bytev() { return std::get<int8_t>(data); }

            [[nodiscard]] const int8_t& bytev() const { return std::get<int8_t>(data); }

            void bytev(int8_t bytev) { data = bytev; }

            [[nodiscard]] uint8_t& ubytev() { return std::get<uint8_t>(data); }

            [[nodiscard]] const uint8_t& ubytev() const { return std::get<uint8_t>(data); }

            void ubytev(uint8_t ubytev) { data = ubytev; }

            [[nodiscard]] int64_t& intv() { return std::get<int64_t>(data); }

            [[nodiscard]] const int64_t& intv() const { return std::get<int64_t>(data); }

            void intv(int64_t intv) { data = intv; }

            [[nodiscard]] uint64_t& uintv() { return std::get<uint64_t>(data); }

            [[nodiscard]] const uint64_t& uintv() const { return std::get<uint64_t>(data); }

            void uintv(uint64_t uintv) { data = uintv; }

            [[nodiscard]] double& doublev() { return std::get<double>(data); }

            [[nodiscard]] const double& doublev() const { return std::get<double>(data); }

            void doublev(const double& v) { data = v; }

            [[nodiscard]] std::string& stringv() { return std::get<std::string>(data); }

            [[nodiscard]] const std::string& stringv() const { return std::get<std::string>(data); }

            void stringv(const std::string& v) { data = v; }

            [[nodiscard]] Namespace& namespacev() { return std::get<Namespace>(data); }

            [[nodiscard]] const Namespace& namespacev() const { return std::get<Namespace>(data); }

            void namespacev(const Namespace& v) { data = v; }

            [[nodiscard]] Multiname& multinamev() { return std::get<Multiname>(data); }

            [[nodiscard]] const Multiname& multinamev() const { return std::get<Multiname>(data); }

            void multinamev(const Multiname& v) { data = v; }

            [[nodiscard]] std::shared_ptr<Class>& classv()
            {
                return std::get<std::shared_ptr<Class>>(data);
            }

            [[nodiscard]] const std::shared_ptr<Class>& classv() const
            {
                return std::get<std::shared_ptr<Class>>(data);
            }

            void classv(std::shared_ptr<Class> v) { data = v; }

            [[nodiscard]] std::shared_ptr<Method>& methodv()
            {
                return std::get<std::shared_ptr<Method>>(data);
            }

            [[nodiscard]] const std::shared_ptr<Method>& methodv() const
            {
                return std::get<std::shared_ptr<Method>>(data);
            }

            void methodv(std::shared_ptr<Method> v) { data = v; }

            [[nodiscard]] ABC::Label& jumpTarget() { return std::get<ABC::Label>(data); }

            [[nodiscard]] const ABC::Label& jumpTarget() const
            {
                return std::get<ABC::Label>(data);
            }

            void jumpTarget(const ABC::Label& v) { data = v; }

            [[nodiscard]] std::vector<ABC::Label>& switchTargets()
            {
                return std::get<std::vector<ABC::Label>>(data);
            }

            [[nodiscard]] const std::vector<ABC::Label>& switchTargets() const
            {
                return std::get<std::vector<ABC::Label>>(data);
            }

            void switchTargets(const std::vector<ABC::Label>& v) { data = v; }

            auto operator<=>(const Argument& other) const noexcept
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
                        return std::get<3>(data) <=> std::get<3>(other.data);
                    case 4:
                        return std::get<4>(data) <=> std::get<4>(other.data);
                    case 5:
                        return std::bit_cast<uint64_t>(std::get<5>(data)) <=>
                               std::bit_cast<uint64_t>(std::get<5>(other.data));
                    case 6:
                        return std::get<6>(data) <=> std::get<6>(other.data);
                    case 7:
                        return std::get<7>(data) <=> std::get<7>(other.data);
                    case 8:
                        return std::get<8>(data) <=> std::get<8>(other.data);
                    case 9:
                        return std::get<9>(data) <=> std::get<9>(other.data);
                    case 10:
                        return std::get<10>(data) <=> std::get<10>(other.data);
                    case 11:
                        return std::get<11>(data) <=> std::get<11>(other.data);
                    case 12:
                        return std::get<12>(data) <=> std::get<12>(other.data);
                }
            }

            bool operator==(const Argument&) const noexcept = default;
        };

        std::vector<Argument> arguments;

        auto operator<=>(const Instruction&) const noexcept = default;
        bool operator==(const Instruction&) const noexcept  = default;
    };
}
