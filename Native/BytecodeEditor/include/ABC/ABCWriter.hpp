// TODO: Test
#pragma once

#include "ABC/ABCFile.hpp"
#include "ABC/Class.hpp"
#include "ABC/Error.hpp"
#include "ABC/ExceptionInfo.hpp"
#include "ABC/Instance.hpp"
#include "ABC/Instruction.hpp"
#include "ABC/Label.hpp"
#include "ABC/Metadata.hpp"
#include "ABC/MethodBody.hpp"
#include "ABC/MethodInfo.hpp"
#include "ABC/Multiname.hpp"
#include "ABC/Namespace.hpp"
#include "ABC/OptionDetail.hpp"
#include "ABC/Script.hpp"
#include "ABC/TraitsInfo.hpp"
#include "enums/ABCType.hpp"
#include "enums/InstanceFlags.hpp"
#include "enums/MethodFlags.hpp"
#include "enums/OPCode.hpp"
#include "enums/OPCodeArgumentType.hpp"
#include "enums/TraitAttribute.hpp"
#include "enums/TraitKind.hpp"
#include "utils/StringException.hpp"

#include <cassert>
#include <limits>
#include <stdint.h>
#include <string>

namespace ABC
{
    class ABCWriter
    {
    private:
        std::vector<uint8_t> buf;
        size_t pos;
        const ABCFile& _abc;

        static constexpr auto writeTable = [](const auto& table, auto writeFunc, size_t start = 0)
        {
            for (size_t i = start; i < table.size(); i++)
            {
                writeFunc(table[i]);
            }
        };

    public:
        [[nodiscard]] const ABCFile& abc() const { return _abc; }

        [[nodiscard]] std::vector<uint8_t>& data() { return buf; }

        [[nodiscard]] const std::vector<uint8_t>& data() const { return buf; }

        ABCWriter(const ABCFile& _abc) : _abc(_abc), pos(0), buf(std::vector<uint8_t>(1024))
        {
            writeU16(_abc.minorVersion);
            writeU16(_abc.majorVersion);

            auto oneToZero = [](size_t n) { return n <= 1 ? 0 : n; };

            writeU30(oneToZero(_abc.ints.size()));
            writeTable(
                _abc.ints, [this](auto v) { writeS32(v); }, 1);
            writeU30(oneToZero(_abc.uints.size()));
            writeTable(
                _abc.uints, [this](auto v) { writeU32(v); }, 1);
            writeU30(oneToZero(_abc.doubles.size()));
            writeTable(
                _abc.doubles, [this](auto v) { writeD64(v); }, 1);
            writeU30(oneToZero(_abc.strings.size()));
            writeTable(
                _abc.strings, [this](auto v) { writeString(v); }, 1);
            writeU30(oneToZero(_abc.namespaces.size()));
            writeTable(
                _abc.namespaces, [this](auto v) { writeNamespace(v); }, 1);
            writeU30(oneToZero(_abc.namespaceSets.size()));
            writeTable(
                _abc.namespaceSets, [this](auto v) { writeNamespaceSet(v); }, 1);
            writeU30(oneToZero(_abc.multinames.size()));
            writeTable(
                _abc.multinames, [this](auto v) { writeMultiname(v); }, 1);

            writeU30(_abc.methods.size());
            writeTable(_abc.methods, [this](auto v) { writeMethodInfo(v); });
            writeU30(_abc.metadata.size());
            writeTable(_abc.metadata, [this](auto v) { writeMetadata(v); });
            writeU30(_abc.instances.size());
            writeTable(_abc.instances, [this](auto v) { writeInstance(v); });
            writeTable(_abc.classes, [this](auto v) { writeClass(v); });
            writeU30(_abc.scripts.size());
            writeTable(_abc.scripts, [this](auto v) { writeScript(v); });
            writeU30(_abc.bodies.size());
            writeTable(_abc.bodies, [this](auto v) { writeMethodBody(v); });

            assert(_abc.classes.size() == _abc.instances.size());

            buf.resize(pos);
        }

        void writeU8(uint8_t v)
        {
            if (pos == buf.size())
            {
                buf.resize(buf.size() * 2);
            }
            buf[pos++] = v;
        }

        void writeU16(uint16_t v)
        {
            writeU8(v & 0xFF);
            writeU8((uint8_t)(v >> 8));
        }

        void writeS24(int32_t v)
        {
            writeU8(v & 0xFF);
            writeU8((uint8_t)(v >> 8));
            writeU8((uint8_t)(v >> 16));
        }

        void writeU32(uint64_t v)
        {
            if (v < 128)
            {
                writeU8((uint8_t)(v));
            }
            else if (v < 16384)
            {
                writeU8((uint8_t)((v & 0x7F) | 0x80));
                writeU8((uint8_t)((v >> 7) & 0x7F));
            }
            else if (v < 2097152)
            {
                writeU8((uint8_t)((v & 0x7F) | 0x80));
                writeU8((uint8_t)((v >> 7) | 0x80));
                writeU8((uint8_t)((v >> 14) & 0x7F));
            }
            else if (v < 268435456)
            {
                writeU8((uint8_t)((v & 0x7F) | 0x80));
                writeU8((uint8_t)(v >> 7 | 0x80));
                writeU8((uint8_t)(v >> 14 | 0x80));
                writeU8((uint8_t)((v >> 21) & 0x7F));
            }
            else
            {
                writeU8((uint8_t)((v & 0x7F) | 0x80));
                writeU8((uint8_t)(v >> 7 | 0x80));
                writeU8((uint8_t)(v >> 14 | 0x80));
                writeU8((uint8_t)(v >> 21 | 0x80));
                writeU8((uint8_t)((v >> 28) & 0x0F));
            }
        }

        void writeS32(int64_t v) { writeU32(v); }

        void writeU30(uint64_t v)
        {
            if (v >= 0x40'00'00'00)
            {
                throw StringException("U30 out of range");
            }
            writeU32(v);
        }

        void writeExact(const void* data, size_t len)
        {
            while (pos + len > buf.size())
            {
                buf.resize(buf.size() * 2);
            }
            memcpy(buf.data() + pos, data, len);
            pos += len;
        }

        void writeD64(double v)
        {
            static_assert(std::numeric_limits<double>::is_iec559);
            static_assert(std::endian::native == std::endian::little);
            writeExact(&v, 8);
        }

        void writeString(std::string_view v)
        {
            writeU30(v.size());
            writeExact(v.data(), v.size());
        }

        void writeBytes(const std::vector<uint8_t>& v)
        {
            writeU30(v.size());
            writeExact(v.data(), v.size());
        }

        void writeNamespace(const Namespace& v)
        {
            writeU8((uint8_t)v.kind);
            writeU30(v.name);
        }

        void writeNamespaceSet(const std::vector<int32_t> v)
        {
            writeU30(v.size());
            writeTable(v, [this](auto v) { writeU30(v); });
        }

        void writeMultiname(const Multiname& v)
        {
            writeU8((uint8_t)v.kind);
            switch (v.kind)
            {
                case ABCType::QName:
                case ABCType::QNameA:
                    writeU30(v.qname().ns);
                    writeU30(v.qname().name);
                    break;
                case ABCType::RTQName:
                case ABCType::RTQNameA:
                    writeU30(v.rtqname().name);
                    break;
                case ABCType::RTQNameL:
                case ABCType::RTQNameLA:
                    break;
                case ABCType::Multiname:
                case ABCType::MultinameA:
                    writeU30(v.multiname().name);
                    writeU30(v.multiname().nsSet);
                    break;
                case ABCType::MultinameL:
                case ABCType::MultinameLA:
                    writeU30(v.multinamel().nsSet);
                    break;
                case ABCType::TypeName:
                    writeU30(v.Typename().name);
                    writeU30(v.Typename().params.size());
                    writeTable(v.Typename().params, [this](auto v) { writeU30(v); });
                    break;
                default:
                    throw StringException("Unknown multiname kind");
            }
        }

        void writeMethodInfo(const MethodInfo& v)
        {
            writeU30(v.paramTypes.size());
            writeU30(v.returnType);
            for (const auto& value : v.paramTypes)
            {
                writeU30(value);
            }
            writeU30(v.name);
            writeU8(v.flags);
            if (v.flags & (uint8_t)MethodFlags::HAS_OPTIONAL)
            {
                writeU30(v.options.size());
                writeTable(v.options, [this](auto v) { writeOptionDetail(v); });
            }
            if (v.flags & (uint8_t)MethodFlags::HAS_PARAM_NAMES)
            {
                assert(v.paramNames.size() == v.paramTypes.size());
                writeTable(v.paramNames, [this](auto v) { writeU30(v); });
            }
        }

        void writeOptionDetail(const OptionDetail& v)
        {
            writeU30(v.value);
            writeU8((uint8_t)v.kind);
        }

        void writeMetadata(const Metadata& v)
        {
            writeU30(v.name);
            writeU30(v.data.size());
            for (const auto& val : v.data)
            {
                writeU30(val.first);
                writeU30(val.second);
            }
        }

        void writeInstance(const Instance& v)
        {
            writeU30(v.name);
            writeU30(v.superName);
            writeU8(v.flags);
            if (v.flags & (uint8_t)InstanceFlags::ProtectedNs)
            {
                writeU30(v.protectedNs);
            }
            writeU30(v.interfaces.size());
            writeTable(v.interfaces, [this](auto v) { writeU30(v); });
            writeU30(v.iinit);
            writeU30(v.traits.size());
            writeTable(v.traits, [this](auto v) { writeTrait(v); });
        }

        void writeTrait(const TraitsInfo& v)
        {
            writeU30(v.name);
            writeU8(v.kindAttr);
            switch (v.kind())
            {
                case TraitKind::Slot:
                case TraitKind::Const:
                    writeU30(v.Slot.slotId);
                    writeU30(v.Slot.typeName);
                    writeU30(v.Slot.vindex);
                    if (v.Slot.vindex != 0)
                    {
                        writeU8((uint8_t)v.Slot.vkind);
                    }
                    break;
                case TraitKind::Class:
                    writeU30(v.Class.slotId);
                    writeU30(v.Class.classi);
                    break;
                case TraitKind::Function:
                    writeU30(v.Function.slotId);
                    writeU30(v.Function.functioni);
                    break;
                case TraitKind::Method:
                case TraitKind::Getter:
                case TraitKind::Setter:
                    writeU30(v.Method.dispId);
                    writeU30(v.Method.method);
                    break;
                default:
                    throw StringException("Unknown trait type");
            }

            if (v.attr() & (uint8_t)TraitAttribute::Metadata)
            {
                writeU30(v.metadata.size());
                writeTable(v.metadata, [this](auto v) { writeU30(v); });
            }
        }

        void writeClass(const Class& v)
        {
            writeU30(v.cinit);
            writeU30(v.traits.size());
            writeTable(v.traits, [this](auto v) { writeTrait(v); });
        }

        void writeScript(const Script& v)
        {
            writeU30(v.sinit);
            writeU30(v.traits.size());
            writeTable(v.traits, [this](auto v) { writeTrait(v); });
        }

        void writeMethodBody(const MethodBody& v)
        {
            writeU30(v.method);
            writeU30(v.maxStack);
            writeU30(v.localCount);
            writeU30(v.initScopeDepth);
            writeU30(v.maxScopeDepth);

            std::vector<size_t> instructionOffsets(v.instructions.size() + 1);

            auto resolveLabel = [&](const Label& label) -> ptrdiff_t
            { return instructionOffsets[label.index] + label.offset; };

            {
                // Temporarily store global buffer
                std::vector<uint8_t> globalBuf = std::move(buf);
                size_t globalPos               = pos;
                buf                            = {};
                buf.resize(1024 * 16);
                pos = 0;

                struct Fixup
                {
                    Label target;
                    size_t pos, base;
                };

                std::vector<Fixup> fixups;

                for (size_t ii = 0; ii < v.instructions.size(); ii++)
                {
                    const auto& instruction = v.instructions[ii];

                    size_t instructionOffset = pos;
                    instructionOffsets[ii]   = instructionOffset;

                    writeU8((uint8_t)instruction.opcode);

                    if (instruction.arguments.size() !=
                        OPCode_Info[(uint8_t)instruction.opcode].second.size())
                    {
                        throw StringException("Mismatching number of arguments");
                    }

                    for (size_t i = 0; i < instruction.arguments.size(); i++)
                    {
                        switch (OPCode_Info[(uint8_t)instruction.opcode].second[i])
                        {
                            case OPCodeArgumentType::Unknown:
                                throw StringException(
                                    std::string("Don't know how to encode OP_") +
                                    OPCode_Info[(uint8_t)instruction.opcode].first);

                            case OPCodeArgumentType::ByteLiteral:
                                writeU8(instruction.arguments[i].bytev());
                                break;
                            case OPCodeArgumentType::UByteLiteral:
                                writeU8(instruction.arguments[i].ubytev());
                                break;
                            case OPCodeArgumentType::IntLiteral:
                                writeS32(instruction.arguments[i].intv());
                                break;
                            case OPCodeArgumentType::UIntLiteral:
                                writeU32(instruction.arguments[i].uintv());
                                break;

                            case OPCodeArgumentType::Int:
                            case OPCodeArgumentType::UInt:
                            case OPCodeArgumentType::Double:
                            case OPCodeArgumentType::String:
                            case OPCodeArgumentType::Namespace:
                            case OPCodeArgumentType::Multiname:
                            case OPCodeArgumentType::Class:
                            case OPCodeArgumentType::Method:
                                writeU30(instruction.arguments[i].index());
                                break;

                            case OPCodeArgumentType::JumpTarget:
                                fixups.emplace_back(
                                    instruction.arguments[i].jumpTarget(), pos, pos + 3);
                                writeS24(0);
                                break;

                            case OPCodeArgumentType::SwitchDefaultTarget:
                                fixups.emplace_back(
                                    instruction.arguments[i].jumpTarget(), pos, instructionOffset);
                                writeS24(0);
                                break;

                            case OPCodeArgumentType::SwitchTargets:
                                if (instruction.arguments[i].switchTargets().size() < 1)
                                {
                                    throw StringException("Too few switch cases");
                                }
                                writeU30(instruction.arguments[i].switchTargets().size() - 1);
                                for (const auto& target : instruction.arguments[i].switchTargets())
                                {
                                    fixups.emplace_back(target, pos, instructionOffset);
                                    writeS24(0);
                                }
                                break;
                        }
                    }
                }

                buf.resize(pos);
                instructionOffsets[v.instructions.size()] = pos;

                for (const auto& fixup : fixups)
                {
                    pos = fixup.pos;
                    writeS24((int32_t)((ptrdiff_t)(resolveLabel(fixup.target) - fixup.base)));
                }

                std::vector<uint8_t> code = std::move(buf);
                // Restore global buffer
                buf = std::move(globalBuf);
                pos = globalPos;

                writeBytes(code);
            }

            writeU30(v.exceptions.size());
            for (const auto& exception : v.exceptions)
            {
                ExceptionInfo write         = exception;
                write.from.absoluteOffset   = resolveLabel(exception.from);
                write.to.absoluteOffset     = resolveLabel(exception.to);
                write.target.absoluteOffset = resolveLabel(exception.target);
                writeExceptionInfo(write);
            }
            writeU30(v.traits.size());
            writeTable(v.traits, [this](auto v) { writeTrait(v); });
        }

        void writeExceptionInfo(const ExceptionInfo& v)
        {
            writeU30(v.from.absoluteOffset);
            writeU30(v.to.absoluteOffset);
            writeU30(v.target.absoluteOffset);
            writeU30(v.excType);
            writeU30(v.varName);
        }
    };

}
