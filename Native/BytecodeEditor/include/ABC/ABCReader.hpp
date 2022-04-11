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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>
#include <limits>
#include <stdint.h>
#include <string>

namespace ABC
{
    class ABCReader
    {
    private:
        const uint8_t* buf;
        size_t len;
        size_t pos;
        ABCFile _abc;

        static constexpr auto setTable = [](auto& table, size_t number, auto readFunc,
                                             size_t start = 0) {
            table.reserve(number + start);
            for (size_t i = start; i < number; i++)
            {
                table.emplace_back(std::invoke(readFunc));
            }
        };

    public:
        ABCFile& abc() { return _abc; }
        const ABCFile& abc() const { return _abc; }

        ABCReader(std::pair<const uint8_t*, size_t> d) : ABCReader(d.first, d.second) {}
        ABCReader(const uint8_t* data, size_t len) : buf(data), len(len), pos(0)
        {
            _abc.minorVersion = readU16();
            _abc.majorVersion = readU16();

            const auto atLeastOne = [](uint32_t n) { return n == 0 ? 1 : n; };

            try
            {
                setTable(
                    _abc.ints, atLeastOne(readU30()), [&] { return readS32(); }, 1);
                setTable(
                    _abc.uints, atLeastOne(readU30()), [&] { return readU32(); }, 1);
                setTable(
                    _abc.doubles, atLeastOne(readU30()), [&] { return readD64(); }, 1);
                setTable(
                    _abc.strings, atLeastOne(readU30()), [&] { return readString(); }, 1);
                setTable(
                    _abc.namespaces, atLeastOne(readU30()), [&] { return readNamespace(); }, 1);
                setTable(
                    _abc.namespaceSets, atLeastOne(readU30()), [&] { return readNamespaceSet(); },
                    1);
                setTable(
                    _abc.multinames, atLeastOne(readU30()), [&] { return readMultiname(); }, 1);

                setTable(_abc.methods, readU30(), [&] { return readMethodInfo(); });
                setTable(_abc.metadata, readU30(), [&] { return readMetadata(); });
                setTable(_abc.instances, readU30(), [&] { return readInstance(); });
                setTable(_abc.classes, _abc.instances.size(), [&] { return readClass(); });
                setTable(_abc.scripts, readU30(), [&] { return readScript(); });
                setTable(_abc.bodies, readU30(), [&] { return readMethodBody(); });
            }
            catch (std::exception& e)
            {
                char print[256] = {'\0'};
                sprintf(print, "Error at %d (0x%X): ", pos, pos);
                throw StringException(std::string(print) + e.what());
            }
        }

        uint8_t readU8()
        {
            if (pos >= len)
                throw StringException("End of file reached");
            return buf[pos++];
        }

        uint16_t readU16() { return readU8() | (readU8() << 8); }

        int32_t readS24()
        {
            uint32_t val = readU8() | (readU8() << 8) | (readU8() << 16);
            if (val & 0x00800000)
            {
                val |= 0xFF000000;
            }
            return val;
        }

        uint64_t readU32()
        {
            const auto next = [this]() -> uint64_t { return readU8(); };

            uint64_t result = next();
            if (!(result & 0x80))
                return result;
            result = (result & 0x7F) | (next() << 7);
            if (!(result & 0x40'00))
                return result;
            result = (result & 0x3F'FF) | (next() << 14);
            if (!(result & 0x20'00'00))
                return result;
            result = (result & 0x1F'FF'FF) | (next() << 21);
            if (!(result & 0x10'00'00'00))
                return result;
            result = (result & 0x0F'FF'FF'FF) | (next() << 28);

            assert(result <= ABCFile::MAX_UINT);
            return result;
        }

        int64_t readS32()
        {
            int64_t ret = readU32();
            if (ret & 0xFFFFFFFF'80000000)
            {
                ret |= 0xFFFFFFFF'00000000;
            }

            assert(ret >= ABCFile::MIN_INT && ret <= ABCFile::MAX_INT);
            return ret;
        }

        uint32_t readU30() { return readU32() & 0x3FFFFFFF; }

        void readExact(std::vector<uint8_t>& data, size_t size)
        {
            if (pos + size > this->len)
                throw StringException("End of file reached");
            data.clear();
            data.insert(data.begin(), buf + pos, buf + pos + size);
            pos += size;
        }

        void readExact(void* data, size_t size)
        {
            if (pos + size > this->len)
                throw StringException("End of file reached");
            memcpy(data, buf + pos, size);
            pos += size;
        }

        double readD64()
        {
            double ret;
            static_assert(std::numeric_limits<decltype(ret)>::is_iec559);
            // static_assert(std::endian::native == std::endian::little);
            readExact(&ret, sizeof(ret));
            return ret;
        }

        std::string readString()
        {
            std::string ret(readU30(), '\0');
            readExact(ret.data(), ret.size());
            return ret;
        }

        std::vector<uint8_t> readBytes()
        {
            std::vector<uint8_t> ret(readU30(), 0);
            readExact(ret.data(), ret.size());
            return ret;
        }

        Namespace readNamespace() { return {ABCType(readU8()), readU30()}; }

        std::vector<int32_t> readNamespaceSet()
        {
            std::vector<int32_t> ret(readU30(), 0);
            for (auto& v : ret)
            {
                v = readU30();
            }
            return ret;
        }

        Multiname readMultiname()
        {
            Multiname ret;
            ret.kind = ABCType(readU8());
            switch (ret.kind)
            {
                case ABCType::QName:
                case ABCType::QNameA:
                    ret.qname({readU30(), readU30()});
                    break;
                case ABCType::RTQName:
                case ABCType::RTQNameA:
                    ret.rtqname({readU30()});
                    break;
                case ABCType::RTQNameL:
                case ABCType::RTQNameLA:
                    ret.rtqnamel({});
                    break;
                case ABCType::Multiname:
                case ABCType::MultinameA:
                    ret.multiname({readU30(), readU30()});
                    break;
                case ABCType::MultinameL:
                case ABCType::MultinameLA:
                    ret.multinamel({readU30()});
                    break;
                case ABCType::TypeName:
                {
                    Multiname::_Typename tname{readU30(), std::vector<uint32_t>(readU30(), 0)};
                    for (auto& v : tname.params)
                    {
                        v = readU30();
                    }
                    ret.Typename(std::move(tname));
                }
                break;
                default:
                    throw StringException("Unknown multiname type");
            }

            return ret;
        }

        MethodInfo readMethodInfo()
        {
            MethodInfo ret;
            size_t tempSize = readU30();
            ret.paramTypes.reserve(tempSize);
            ret.returnType = readU30();
            for (size_t i = 0; i < tempSize; i++)
            {
                ret.paramTypes.emplace_back(readU30());
            }
            ret.name  = readU30();
            ret.flags = readU8();
            if (ret.flags & uint8_t(MethodFlags::HAS_OPTIONAL))
            {
                tempSize = readU30();
                ret.options.reserve(tempSize);
                for (size_t i = 0; i < tempSize; i++)
                {
                    ret.options.emplace_back(readOptionDetail());
                }
            }
            if (ret.flags & uint8_t(MethodFlags::HAS_PARAM_NAMES))
            {
                ret.paramNames.reserve(ret.paramTypes.size());
                for (size_t i = 0; i < ret.paramTypes.size(); i++)
                {
                    ret.paramNames.emplace_back(readU30());
                }
            }
            return ret;
        }

        OptionDetail readOptionDetail() { return {readU30(), ABCType(readU8())}; }

        Metadata readMetadata()
        {
            Metadata ret{readU30(), std::vector<std::pair<uint32_t, uint32_t>>(readU30())};
            for (auto& v : ret.data)
            {
                v = {readU30(), readU30()};
            }
            return ret;
        }

        Instance readInstance()
        {
            Instance ret;
            ret.name      = readU30();
            ret.superName = readU30();
            ret.flags     = readU8();
            if (ret.flags & uint8_t(InstanceFlags::ProtectedNs))
                ret.protectedNs = readU30();
            size_t tempSize = readU30();
            ret.interfaces.reserve(tempSize);
            for (size_t i = 0; i < tempSize; i++)
            {
                ret.interfaces.emplace_back(readU30());
            }
            ret.iinit = readU30();
            tempSize  = readU30();
            ret.traits.reserve(tempSize);
            for (size_t i = 0; i < tempSize; i++)
            {
                ret.traits.emplace_back(readTrait());
            }

            return ret;
        }

        TraitsInfo readTrait()
        {
            TraitsInfo ret;
            ret.name     = readU30();
            ret.kindAttr = readU8();

            switch (ret.kind())
            {
                case TraitKind::Slot:
                case TraitKind::Const:
                    ret.Slot.slotId   = readU30();
                    ret.Slot.typeName = readU30();
                    ret.Slot.vindex   = readU30();
                    if (ret.Slot.vindex != 0)
                    {
                        ret.Slot.vkind = ABCType(readU8());
                    }
                    else
                    {
                        ret.Slot.vkind = ABCType::Void;
                    }
                    break;
                case TraitKind::Class:
                    ret.Class = {readU30(), readU30()};
                    break;
                case TraitKind::Function:
                    ret.Function = {readU30(), readU30()};
                    break;
                case TraitKind::Method:
                case TraitKind::Getter:
                case TraitKind::Setter:
                    ret.Method = {readU30(), readU30()};
                    break;
                default:
                    throw StringException(
                        "Unknown trait type" + std::to_string((uint32_t)ret.kindAttr));
            }
            if (ret.attr() & uint8_t(TraitAttribute::Metadata))
            {
                size_t tempSize = readU30();
                ret.metadata.reserve(tempSize);
                for (size_t i = 0; i < tempSize; i++)
                {
                    ret.metadata.emplace_back(readU30());
                }
            }
            return ret;
        }

        Class readClass()
        {
            Class ret{readU30(), std::vector<TraitsInfo>(readU30())};
            for (auto& v : ret.traits)
            {
                v = readTrait();
            }
            return ret;
        }

        Script readScript()
        {
            Script ret{readU30(), std::vector<TraitsInfo>(readU30())};
            for (auto& v : ret.traits)
            {
                v = readTrait();
            }
            return ret;
        }

        MethodBody readMethodBody()
        {
            MethodBody ret;
            ret.method         = readU30();
            ret.maxStack       = readU30();
            ret.localCount     = readU30();
            ret.initScopeDepth = readU30();
            ret.maxScopeDepth  = readU30();

            size_t len   = readU30();
            size_t start = pos;
            size_t end   = pos + len;

            pos = end;

            setTable(ret.exceptions, readU30(), [&] { return readExceptionInfo(); });

            size_t postExceptions = pos;

            enum class TraceState : uint8_t
            {
                unexplored,
                pending,
                instruction,
                instructionBody,
                error
            };

            std::vector<TraceState> traceState(len, TraceState::unexplored);
            std::vector<Instruction> instructions(len);

            const auto offset = [&] { return pos - start; };

            bool done = false;

            const auto queue = [&](size_t traceOffset) {
                if (traceOffset < len && traceState[traceOffset] == TraceState::unexplored)
                {
                    traceState[traceOffset] = TraceState::pending;
                    done                    = false;
                }
            };

            queue(0);

            for (const auto& exception : ret.exceptions)
            {
                queue(exception.target.absoluteOffset);
            }

            while (!done)
            {
                done = true;
                pos  = start;
                while (pos < end)
                {
                    if (traceState[offset()] == TraceState::pending)
                    {
                        size_t instructionOffset;

                        try
                        {
                            while (pos < end)
                            {
                                instructionOffset = offset();
                                if (traceState[instructionOffset] == TraceState::instructionBody)
                                    throw StringException("Overlapping instruction");
                                if (traceState[instructionOffset] == TraceState::instruction)
                                    break; // already decoded

                                Instruction instruction;
                                instruction.opcode = OPCode(readU8());
                                if (instruction.opcode == OPCode::OP_raw)
                                    throw StringException("Null OPCode");

                                instruction.arguments.resize(
                                    OPCode_Info[(uint8_t)instruction.opcode].second.size());

                                for (size_t i = 0; i < instruction.arguments.size(); i++)
                                {
                                    switch (OPCode_Info[(uint8_t)instruction.opcode].second[i])
                                    {
                                        case OPCodeArgumentType::Unknown:
                                            throw StringException(
                                                "Don't know how to decode OP_" +
                                                std::string(OPCode_Info[(uint8_t)instruction.opcode]
                                                                .first));

                                        case OPCodeArgumentType::ByteLiteral:
                                            instruction.arguments[i].bytev(readU8());
                                            break;
                                        case OPCodeArgumentType::UByteLiteral:
                                            instruction.arguments[i].ubytev(readU8());
                                            break;
                                        case OPCodeArgumentType::IntLiteral:
                                            instruction.arguments[i].intv(readS32());
                                            break;
                                        case OPCodeArgumentType::UIntLiteral:
                                            instruction.arguments[i].uintv(readU32());
                                            break;

                                        case OPCodeArgumentType::Int:
                                        case OPCodeArgumentType::UInt:
                                        case OPCodeArgumentType::Double:
                                        case OPCodeArgumentType::String:
                                        case OPCodeArgumentType::Namespace:
                                        case OPCodeArgumentType::Multiname:
                                        case OPCodeArgumentType::Class:
                                        case OPCodeArgumentType::Method:
                                        {
                                            size_t index = readU30();
                                            size_t length;
                                            switch (
                                                OPCode_Info[(uint8_t)instruction.opcode].second[i])
                                            {
                                                case OPCodeArgumentType::Int:
                                                    length = _abc.ints.size();
                                                    break;
                                                case OPCodeArgumentType::UInt:
                                                    length = _abc.uints.size();
                                                    break;
                                                case OPCodeArgumentType::Double:
                                                    length = _abc.doubles.size();
                                                    break;
                                                case OPCodeArgumentType::String:
                                                    length = _abc.strings.size();
                                                    break;
                                                case OPCodeArgumentType::Namespace:
                                                    length = _abc.namespaces.size();
                                                    break;
                                                case OPCodeArgumentType::Multiname:
                                                    length = _abc.multinames.size();
                                                    break;
                                                case OPCodeArgumentType::Class:
                                                    length = _abc.classes.size();
                                                    break;
                                                case OPCodeArgumentType::Method:
                                                    length = _abc.methods.size();
                                                    break;
                                                default:
                                                    assert(false);
                                            }
                                            if (index >= length)
                                                throw StringException(
                                                    "Out of bounds constant index");
                                            instruction.arguments[i].index(index);
                                        }
                                        break;

                                        case OPCodeArgumentType::JumpTarget:
                                        {
                                            int32_t delta  = readS24();
                                            int32_t target = offset() + delta;
                                            instruction.arguments[i].jumpTarget(
                                                Label{.absoluteOffset = target});
                                            queue(target);
                                        }
                                        break;

                                        case OPCodeArgumentType::SwitchDefaultTarget:
                                        {
                                            int32_t target = instructionOffset + readS24();
                                            instruction.arguments[i].jumpTarget(
                                                Label{.absoluteOffset = target});
                                            queue(target);
                                        }
                                        break;

                                        case OPCodeArgumentType::SwitchTargets:
                                        {
                                            std::vector<Label> switchTargets(readU30() + 1);
                                            for (Label& label : switchTargets)
                                            {
                                                label.absoluteOffset =
                                                    instructionOffset + readS24();
                                                queue(label.absoluteOffset);
                                            }

                                            instruction.arguments[i].switchTargets(
                                                std::move(switchTargets));
                                        }
                                        break;
                                    }
                                }

                                if (offset() > len)
                                    throw StringException("Out-of-bounds code read error");

                                instructions[instructionOffset] = instruction;
                                traceState[instructionOffset]   = TraceState::instruction;
                                for (size_t i = instructionOffset + 1; i < offset(); i++)
                                {
                                    traceState[i] = TraceState::instructionBody;
                                }

                                // if (stopsExecution[instruction.opcode])
                                //   break;
                            }
                        }
                        catch (std::exception& e)
                        {
                            traceState[instructionOffset] = TraceState::error;
                            Label loc{.absoluteOffset = (ptrdiff_t)instructionOffset};
                            ret.errors.emplace_back(loc, e.what());

                            pos = start + instructionOffset + 1;
                        }
                    }
                    else
                    {
                        pos++;
                    }
                }
            }

            std::vector<size_t> instructionOffsets;
            std::vector<uint32_t> instructionAtOffset(len, UINT32_MAX);

            const auto addInstruction = [&](const Instruction& i, size_t offset) {
                instructionAtOffset[offset] = ret.instructions.size();
                ret.instructions.emplace_back(i);
                instructionOffsets.emplace_back(offset);
            };

            for (size_t currentOffset = 0; currentOffset < traceState.size(); currentOffset++)
            {
                assert(traceState[currentOffset] != TraceState::pending);
                if (traceState[currentOffset] == TraceState::instruction)
                {
                    addInstruction(instructions[currentOffset], currentOffset);
                }
                else if (traceState[currentOffset] == TraceState::unexplored ||
                         traceState[currentOffset] == TraceState::error)
                {
                    Instruction instruction;
                    instruction.opcode = OPCode::OP_raw;
                    instruction.arguments.resize(1);
                    instruction.arguments[0].ubytev(buf[start + currentOffset]);
                    addInstruction(instruction, currentOffset);
                }
                else
                {
                    assert(traceState[currentOffset] == TraceState::instructionBody);
                }
            }

            const auto translateLabel = [&](Label& label) {
                ptrdiff_t absoluteOffset    = label.absoluteOffset;
                ptrdiff_t instructionOffset = absoluteOffset;

                while (true)
                {
                    if (instructionOffset >= len)
                    {
                        label = {.index = (uint32_t)ret.instructions.size(), .offset = 0};
                        instructionOffset = len;
                        break;
                    }
                    if (instructionOffset <= 0)
                    {
                        label             = {.index = 0, .offset = 0};
                        instructionOffset = 0;
                        break;
                    }
                    if (instructionAtOffset[instructionOffset] != UINT32_MAX)
                    {
                        label = {.index = instructionAtOffset[instructionOffset], .offset = 0};
                        break;
                    }
                    instructionOffset--;
                }
                label.offset = (int32_t)(absoluteOffset - instructionOffset);
            };

            for (auto& instruction : ret.instructions)
            {
                for (size_t i = 0; i < OPCode_Info[(uint8_t)instruction.opcode].second.size(); i++)
                {
                    switch (OPCode_Info[(uint8_t)instruction.opcode].second[i])
                    {
                        case OPCodeArgumentType::JumpTarget:
                        case OPCodeArgumentType::SwitchDefaultTarget:
                            translateLabel(instruction.arguments[i].jumpTarget());
                            break;
                        case OPCodeArgumentType::SwitchTargets:
                        {
                            for (auto& label : instruction.arguments[i].switchTargets())
                            {
                                translateLabel(label);
                            }
                        }
                    }
                }
            }

            for (auto& error : ret.errors)
            {
                translateLabel(error.loc);
            }

            for (auto& exception : ret.exceptions)
            {
                translateLabel(exception.from);
                translateLabel(exception.to);
                translateLabel(exception.target);
            }

            pos = postExceptions;

            size_t tempSize = readU30();
            ret.traits.reserve(tempSize);
            for (size_t i = 0; i < tempSize; i++)
            {
                ret.traits.emplace_back(readTrait());
            }

            return ret;
        }

        ExceptionInfo readExceptionInfo()
        {
            return {.from = {.absoluteOffset = (ptrdiff_t)readU30()},
                .to       = {.absoluteOffset = (ptrdiff_t)readU30()},
                .target   = {.absoluteOffset = (ptrdiff_t)readU30()},
                .excType  = readU30(),
                .varName  = readU30()};
        }
    };
}
