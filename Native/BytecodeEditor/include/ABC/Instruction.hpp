#pragma once

#include "ABC/Label.hpp"
#include "enums/OPCode.hpp"
#include <stdint.h>
#include <variant>
#include <vector>

namespace SWFABC
{
    struct Instruction
    {
        OPCode opcode = OPCode::OP_raw;

        struct Argument
        {
        private:
            std::variant<std::monostate, int8_t, uint8_t, int64_t, uint64_t, uint32_t, Label,
                std::vector<Label>>
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

            [[nodiscard]] uint32_t& index() { return std::get<uint32_t>(data); }

            [[nodiscard]] const uint32_t& index() const { return std::get<uint32_t>(data); }

            void index(uint32_t index) { data = index; }

            [[nodiscard]] Label& jumpTarget() { return std::get<Label>(data); }

            [[nodiscard]] const Label& jumpTarget() const { return std::get<Label>(data); }

            void jumpTarget(Label jumpTarget) { data = jumpTarget; }

            [[nodiscard]] std::vector<Label>& switchTargets()
            {
                return std::get<std::vector<Label>>(data);
            }

            [[nodiscard]] const std::vector<Label>& switchTargets() const
            {
                return std::get<std::vector<Label>>(data);
            }

            void switchTargets(std::vector<Label>&& switchTargets)
            {
                data = std::move(switchTargets);
            }
        };

        std::vector<Argument> arguments;
    };
}
