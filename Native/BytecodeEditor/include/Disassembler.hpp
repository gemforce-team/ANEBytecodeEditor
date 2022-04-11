#pragma once

#include "ASASM/ASProgram.hpp"
#include "ASASM/Class.hpp"
#include "ASASM/Exception.hpp"
#include "ASASM/Instance.hpp"
#include "ASASM/Instruction.hpp"
#include "ASASM/Metadata.hpp"
#include "ASASM/Method.hpp"
#include "ASASM/MethodBody.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Namespace.hpp"
#include "ASASM/Script.hpp"
#include "ASASM/Trait.hpp"
#include "ASASM/Value.hpp"
#include "enums/InstanceFlags.hpp"
#include "enums/MethodFlags.hpp"
#include "enums/TraitAttribute.hpp"
#include "utils/RefBuilder.hpp"
#include "utils/StringBuilder.hpp"

#include <climits>
#include <cmath>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

class Disassembler
{
private:
    std::unordered_map<std::string, std::string> strings;
    const ASASM::ASProgram& as;

    RefBuilder refs;
    bool dumpRaw = true;

    template <typename Callback>
    void newInclude(
        StringBuilder& mainsb, std::string_view filename, Callback callback, bool doInline = true)
    {
        if (doInline)
        {
            StringBuilder sb;
            callback(sb);
            strings.insert_or_assign(std::string(filename), sb.str());

            mainsb << "#include ";
            dumpString(mainsb, filename);
            mainsb.newLine();
        }
        else
        {
            callback(mainsb);
        }
    }

public:
    Disassembler(const ASASM::ASProgram& as) : as(as), refs(as) {}

    std::unordered_map<std::string, std::string> disassemble()
    {
        refs.run();

        StringBuilder sb;

        sb << "#version 4";
        sb.newLine();

        sb << "program";
        sb.indent++;
        sb.newLine();

        sb << "minorversion " << as.minorVersion;
        sb.newLine();
        sb << "majorversion " << as.majorVersion;
        sb.newLine();
        sb.newLine();

        for (size_t i = 0; i < as.scripts.size(); i++)
        {
            newInclude(sb, refs.scripts.getFilename(&as.scripts[i], "script"),
                [this, i](StringBuilder& sb) { dumpScript(sb, as.scripts[i], i); });
        }
        sb.newLine();

        if (!as.orphanClasses.empty())
        {
            sb << "; ============================= Orphan classes ==============================";
            sb.newLine();
            sb.newLine();

            for (size_t i = 0; i < as.orphanClasses.size(); i++)
            {
                newInclude(sb, refs.objects.getFilename(as.orphanClasses[i].get(), "class"),
                    [this, i](StringBuilder& sb) { dumpClass(sb, *as.orphanClasses[i]); });
            }
            sb.newLine();
        }

        if (!as.orphanMethods.empty())
        {
            sb << "; ============================= Orphan methods ==============================";
            sb.newLine();
            sb.newLine();

            for (size_t i = 0; i < as.orphanMethods.size(); i++)
            {
                newInclude(sb, refs.objects.getFilename(as.orphanMethods[i].get(), "method"),
                    [this, i](
                        StringBuilder& sb) { dumpMethod(sb, *as.orphanMethods[i], "method"); });
            }
            sb.newLine();
        }

        sb.indent--;
        sb << "end ; program";
        sb.newLine();

        strings.insert_or_assign(std::string("main.asasm"), sb.str());
        return std::move(strings);
    }

    void dumpInt(StringBuilder& sb, int64_t v)
    {
        if (v == ABC::ABCFile::NULL_INT)
            sb << "null";
        else
            sb.write(v);
    }

    void dumpUInt(StringBuilder& sb, uint64_t v)
    {
        if (v == ABC::ABCFile::NULL_UINT)
            sb << "null";
        else
            sb.write(v);
    }

    void dumpDouble(StringBuilder& sb, double v)
    {
        if (std::isnan(v)) // v == ABC::ABCFile::NULL_DOUBLE
        {
            sb << "null";
            return;
        }
        else if (std::isinf(v))
        {
            assert(-INFINITY < 0);
            if (v < 0)
            {
                sb << "-inf";
            }
            else
            {
                sb << "inf";
            }
        }
        else
        {
            std::array<char, 20> formatStr;
            std::array<std::array<char, 64>, 19> precisions;
            std::array<int, 19> lengths;

            // Try all precisions of printing from 0-18
            for (int i = 0; i <= 18; i++)
            {
                sprintf(formatStr.data(), "%%.%ig", i);
                sprintf(precisions[i].data(), formatStr.data(), v);

                lengths[i] = strlen(precisions[i].data());
            }

            int current      = INT_MAX;
            int currentIndex = -1;
            for (int i = 0; i <= 18; i++)
            {
                if (current > lengths[i] && std::strtod(precisions[i].data(), nullptr) == v)
                {
                    currentIndex = i;
                    current      = lengths[i];
                }
            }

            if (currentIndex != -1)
            {
                sb << precisions[currentIndex].data();
                return;
            }

            // Otherwise we try hex notation

            sprintf(precisions[0].data(), "%.13a", v);
            assert(std::strtod(precisions[0].data(), nullptr) == v);

            sb << precisions[0].data();
        }
    }

    void dumpString(StringBuilder& sb, std::string_view str, bool emptyIsNull = false)
    {
        if (emptyIsNull && (str.size() == 0 || str == ""))
        {
            sb << "null";
            return;
        }
        // if (str is null)
        //   sb ~= "null";

        static constexpr std::array<char, 16> hexDigits{
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        sb << '"';
        for (unsigned char c : str)
        {
            if (c == '\n')
            {
                sb << "\\n";
            }
            else if (c == '\r')
            {
                sb << "\\r";
            }
            else if (c == '\\')
            {
                sb << "\\\\";
            }
            else if (c == '"')
            {
                sb << "\\\"";
            }
            else if (c < 0x20)
            {
                sb << "\\x" << hexDigits[c / 0x10] << hexDigits[c % 0x10];
            }
            else
            {
                sb << c;
            }
        }
        sb << '"';
    }

    void dumpNamespace(StringBuilder& sb, const ASASM::Namespace& ns)
    {
        if (ns.kind == ABCType::Void)
        {
            sb << "null";
        }
        else
        {
            sb << *ABCTypeMap.ReverseFind(ns.kind) << '(';
            dumpString(sb, ns.name);
            if (refs.hasHomonyms(ns))
            {
                sb << ", ";
                dumpString(sb, refs.namespaces[(uint8_t)ns.kind].getName(ns.id));
            }
            sb << ')';
        }
    }

    void dumpNamespaceSet(StringBuilder& sb, const ASASM::Namespace* nsSet, size_t size)
    {
        if (size == 0)
        {
            sb << "null";
        }
        else
        {
            sb << '[';
            for (size_t i = 0; i < size; i++)
            {
                dumpNamespace(sb, nsSet[i]);
                if (i < size - 1)
                    sb << ", ";
            }
            sb << ']';
        }
    }
    void dumpNamespaceSet(StringBuilder& sb, const std::vector<ASASM::Namespace>& nsSet)
    {
        dumpNamespaceSet(sb, nsSet.data(), nsSet.size());
    }

    void dumpMultiname(StringBuilder& sb, const ASASM::Multiname& multiname)
    {
        if (multiname.kind == ABCType::Void)
        {
            sb << "null";
        }
        else
        {
            sb << *ABCTypeMap.ReverseFind(multiname.kind) << '(';
            switch (multiname.kind)
            {
                case ABCType::QName:
                case ABCType::QNameA:
                    dumpNamespace(sb, multiname.qname().ns);
                    sb << ", ";
                    dumpString(sb, multiname.qname().name);
                    break;
                case ABCType::RTQName:
                case ABCType::RTQNameA:
                    dumpString(sb, multiname.rtqname().name);
                    break;
                case ABCType::RTQNameL:
                case ABCType::RTQNameLA:
                    break;
                case ABCType::Multiname:
                case ABCType::MultinameA:
                    dumpString(sb, multiname.multiname().name);
                    sb << ", ";
                    dumpNamespaceSet(sb, multiname.multiname().nsSet);
                    break;
                case ABCType::MultinameL:
                case ABCType::MultinameLA:
                    dumpNamespaceSet(sb, multiname.multinamel().nsSet);
                    break;
                case ABCType::TypeName:
                    dumpMultiname(sb, multiname.Typename().name());
                    sb << '<';
                    for (size_t i = 0; i < multiname.Typename().params().size(); i++)
                    {
                        dumpMultiname(sb, multiname.Typename().params()[i]);
                        if (i < multiname.Typename().params().size() - 1)
                            sb << ", ";
                    }
                    sb << '>';
                    break;
                default:
                    throw StringException("Unknown multiname kind");
            }
            sb << ')';
        }
    }

    void dumpTraits(
        StringBuilder& sb, const ASASM::Trait* traits, size_t size, bool inScript = false)
    {
        for (size_t i = 0; i < size; i++)
        {
            const auto& trait = traits[i];

            sb << "trait " << *TraitKindMap.ReverseFind(trait.kind) << ' ';
            dumpMultiname(sb, trait.name);
            if (trait.attributes)
                dumpFlags(sb, trait.attributes, TraitAttributeMap, true);
            bool inLine = false;
            switch (trait.kind)
            {
                case TraitKind::Slot:
                case TraitKind::Const:
                    if (trait.vSlot().slotId)
                    {
                        sb << " slotid ";
                        dumpUInt(sb, trait.vSlot().slotId);
                    }
                    if (trait.vSlot().typeName.kind != ABCType::Void)
                    {
                        sb << " type ";
                        dumpMultiname(sb, trait.vSlot().typeName);
                    }
                    if (trait.vSlot().value.vkind != ABCType::Void)
                    {
                        sb << " value ";
                        dumpValue(sb, trait.vSlot().value);
                    }
                    inLine = true;
                    break;
                case TraitKind::Class:
                    if (trait.vClass().slotId)
                    {
                        sb << " slotid ";
                        dumpUInt(sb, trait.vClass().slotId);
                    }
                    sb.indent++;
                    sb.newLine();

                    newInclude(sb, refs.objects.getFilename(trait.vClass().vclass.get(), "class"),
                        [this, &trait](
                            StringBuilder& sb) { dumpClass(sb, *trait.vClass().vclass); });
                    break;
                case TraitKind::Function:
                    if (trait.vFunction().slotId)
                    {
                        sb << " slotid ";
                        dumpUInt(sb, trait.vFunction().slotId);
                    }
                    sb.indent++;
                    sb.newLine();
                    newInclude(
                        sb, refs.objects.getFilename(trait.vFunction().vfunction.get(), "method"),
                        [this, &trait](StringBuilder& sb) {
                            dumpMethod(sb, *trait.vFunction().vfunction, "method");
                        },
                        inScript);
                    break;
                case TraitKind::Method:
                case TraitKind::Getter:
                case TraitKind::Setter:
                    if (trait.vMethod().dispId)
                    {
                        sb << " dispid ";
                        dumpUInt(sb, trait.vMethod().dispId);
                    }
                    sb.indent++;
                    sb.newLine();
                    newInclude(
                        sb, refs.objects.getFilename(trait.vMethod().vmethod.get(), "method"),
                        [this, &trait](StringBuilder& sb) {
                            dumpMethod(sb, *trait.vMethod().vmethod, "method");
                        },
                        inScript);
                    break;
                default:
                    throw StringException("Unknown trait kind");
            }

            for (const auto& metadata : trait.metadata)
            {
                if (inLine)
                {
                    sb.indent++;
                    sb.newLine();
                    inLine = false;
                }
                dumpMetadata(sb, metadata);
            }

            if (inLine)
            {
                sb << " end";
                sb.newLine();
            }
            else
            {
                sb.indent--;
                sb << "end ; trait";
                sb.newLine();
            }
        }
    }
    void dumpTraits(
        StringBuilder& sb, const std::vector<ASASM::Trait> traits, bool inScript = false)
    {
        dumpTraits(sb, traits.data(), traits.size(), inScript);
    }

    void dumpMetadata(StringBuilder& sb, const ASASM::Metadata& metadata)
    {
        sb << "metadata ";
        dumpString(sb, metadata.name);
        sb.indent++;
        sb.newLine();
        for (const auto& md : metadata.data)
        {
            sb << "item ";
            dumpString(sb, md.first);
            sb << " ";
            dumpString(sb, md.second);
            sb.newLine();
        }
        sb.indent--;
        sb << "end ; metadata";
        sb.newLine();
    }

    template <typename FlagMap>
    void dumpFlags(StringBuilder& sb, uint8_t v, const FlagMap& map, bool oneLine = false)
    {
        const auto& entries = map.GetEntries();
        for (size_t i = 0; i < entries.second; i++)
        {
            const auto& entry = entries.first[i];
            if (v & (uint8_t)entry.second)
            {
                sb << (oneLine ? " flag " : "flag ") << entry.first;
                if (!oneLine)
                {
                    sb.newLine();
                }
            }
        }
    }

    void dumpValue(StringBuilder& sb, const ASASM::Value& value)
    {
        sb << *ABCTypeMap.ReverseFind(value.vkind) << '(';
        switch (value.vkind)
        {
            case ABCType::Integer:
                dumpInt(sb, value.vint());
                break;
            case ABCType::UInteger:
                dumpUInt(sb, value.vuint());
                break;
            case ABCType::Double:
                dumpDouble(sb, value.vdouble());
                break;
            case ABCType::Utf8:
                dumpString(sb, value.vstring());
                break;
            case ABCType::Namespace:
            case ABCType::PackageNamespace:
            case ABCType::PackageInternalNs:
            case ABCType::ProtectedNamespace:
            case ABCType::ExplicitNamespace:
            case ABCType::StaticProtectedNs:
            case ABCType::PrivateNamespace:
                dumpNamespace(sb, value.vnamespace());
                break;
            case ABCType::True:
            case ABCType::False:
            case ABCType::Null:
            case ABCType::Undefined:
                break;
            default:
                throw StringException("Unknown type");
        }
        sb << ')';
    }

    void dumpMethod(StringBuilder& sb, const ASASM::Method& method, std::string_view label)
    {
        sb << label;
        sb.indent++;
        sb.newLine();
        // if (method.name !is null)
        // {
        sb << "name ";
        dumpString(sb, method.name);
        sb.newLine();
        // }

        auto refName = refs.objects.getName(&method);
        // if (refName)
        // {
        sb << "refid ";
        dumpString(sb, refName);
        sb.newLine();
        // }

        for (const auto& type : method.paramTypes)
        {
            sb << "param ";
            dumpMultiname(sb, type);
            sb.newLine();
        }

        if (method.returnType.kind != ABCType::Void)
        {
            sb << "returns ";
            dumpMultiname(sb, method.returnType);
            sb.newLine();
        }

        dumpFlags(sb, method.flags, MethodFlagMap);

        for (const auto& v : method.options)
        {
            sb << "optional ";
            dumpValue(sb, v);
            sb.newLine();
        }

        for (const auto& s : method.paramNames)
        {
            sb << "paramname ";
            dumpString(sb, s, true);
            sb.newLine();
        }
        if (method.vbody)
        {
            dumpMethodBody(sb, *method.vbody);
        }
        sb.indent--;
        sb << "end ; method";
        sb.newLine();
    }

    void dumpClass(StringBuilder& sb, const ASASM::Class& vclass)
    {
        sb << "class";
        sb.indent++;
        sb.newLine();

        auto refName = refs.objects.getName(&vclass);
        // if (refName)
        // {
        sb << "refid ";
        dumpString(sb, refName);
        sb.newLine();
        // }

        sb << "instance ";
        dumpInstance(sb, vclass.instance);
        dumpMethod(sb, *vclass.cinit, "cinit");
        dumpTraits(sb, vclass.traits);

        sb.indent--;
        sb << "end ; class";
        sb.newLine();
    }

    void dumpInstance(StringBuilder& sb, const ASASM::Instance& instance)
    {
        dumpMultiname(sb, instance.name);
        sb.indent++;
        sb.newLine();

        if (instance.superName.kind != ABCType::Void)
        {
            sb << "extends ";
            dumpMultiname(sb, instance.superName);
            sb.newLine();
        }

        for (const auto& interface : instance.interfaces)
        {
            sb << "implements ";
            dumpMultiname(sb, interface);
            sb.newLine();
        }

        dumpFlags(sb, instance.flags, InstanceFlagMap);

        if (instance.protectedNs.kind != ABCType::Void)
        {
            sb << "protectedns ";
            dumpNamespace(sb, instance.protectedNs);
            sb.newLine();
        }

        dumpMethod(sb, *instance.iinit, "iinit");
        dumpTraits(sb, instance.traits);
        sb.indent--;
        sb << "end ; instance";
        sb.newLine();
    }

    void dumpScript(StringBuilder& sb, const ASASM::Script& script, uint32_t index)
    {
        sb << "script";
        sb.indent++;
        sb.newLine();

        dumpMethod(sb, *script.sinit, "sinit");
        dumpTraits(sb, script.traits, true);

        sb.indent--;
        sb << "end ; script";
        sb.newLine();
    }

    void dumpUIntField(StringBuilder& sb, std::string_view name, uint32_t value)
    {
        sb << name << ' ';
        dumpUInt(sb, value);
        sb.newLine();
    }

    void dumpLabel(StringBuilder& sb, const ABC::Label& label)
    {
        sb << 'L';
        sb.write(label.index);
        if (label.offset != 0)
        {
            if (label.offset > 0)
                sb << '+';
            sb.write(label.offset);
        }
    }

    void dumpMethodBody(StringBuilder& sb, const ASASM::MethodBody& body)
    {
        sb << "body";
        sb.indent++;
        sb.newLine();

        dumpUIntField(sb, "maxstack", body.maxStack);
        dumpUIntField(sb, "localcount", body.localCount);
        dumpUIntField(sb, "initscopedepth", body.initScopeDepth);
        dumpUIntField(sb, "maxscopedepth", body.maxScopeDepth);

        sb << "code";
        sb.newLine();

        std::vector<bool> labels(body.instructions.size() + 1);
        // reserve exception labels
        for (const auto& e : body.exceptions)
        {
            labels[e.from.index] = labels[e.to.index] = labels[e.target.index] = true;
        }

        sb.indent++;
        dumpInstructions(sb, body.instructions, labels, body.errors);
        sb.indent--;

        sb << "end ; code";
        sb.newLine();

        for (const auto& e : body.exceptions)
        {
            sb << "try from ";
            dumpLabel(sb, e.from);
            sb << " to ";
            dumpLabel(sb, e.to);
            sb << " target ";
            dumpLabel(sb, e.target);
            sb << " type ";
            dumpMultiname(sb, e.excType);
            sb << " name ";
            dumpMultiname(sb, e.varName);
            sb << " end";
            sb.newLine();
        }

        dumpTraits(sb, body.traits);

        sb.indent--;
        sb << "end ; body";
        sb.newLine();
        sb.linePrefix = "";
    }

    void dumpInstructions(StringBuilder& sb, const std::vector<ASASM::Instruction>& instructions,
        std::vector<bool>& labels, const std::vector<ABC::Error>& errors)
    {
        for (const auto& instruction : instructions)
        {
            for (size_t i = 0; i < OPCode_Info[(uint8_t)instruction.opcode].second.size(); i++)
            {
                switch (OPCode_Info[(uint8_t)instruction.opcode].second[i])
                {
                    case OPCodeArgumentType::JumpTarget:
                    case OPCodeArgumentType::SwitchDefaultTarget:
                        labels[instruction.arguments[i].jumpTarget().index] = true;
                        break;
                    case OPCodeArgumentType::SwitchTargets:
                        for (const auto& label : instruction.arguments[i].switchTargets())
                        {
                            labels[label.index] = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        auto checkLabel = [&](size_t ii) {
            if (labels[ii])
            {
                sb.noIndent();
                sb << 'L';
                sb.write(ii);
                sb << ':';
                sb.newLine();
            }
        };

        std::vector<std::string> iErrors(instructions.size() + 1);
        for (const auto& error : errors)
        {
            iErrors[error.loc.index] = error.message;
        }

        bool extraNewLine = false;
        for (size_t ii = 0; ii < instructions.size(); ii++)
        {
            const auto& instruction = instructions[ii];
            if (extraNewLine)
                sb.newLine();
            // extraNewLine = newLineAfter[instruction.opcode];
            checkLabel(ii);

            if (!iErrors[ii].empty())
            {
                sb << "; Error: " << iErrors[ii];
                sb.newLine();
            }

            if (instruction.opcode == OPCode::OP_raw)
            {
                // if (dumpRaw)
                //  sb << std::ios::hex << "; " << instruction.arguments[0].ubytev() <<
                //  std::ios::dec;
                // sb.newLine();
                continue;
            }

            sb << OPCode_Info[(uint8_t)instruction.opcode].first;

            const auto& argTypes = OPCode_Info[(uint8_t)instruction.opcode].second;
            if (argTypes.size() != 0)
            {
                sb << ' ';
                for (size_t i = 0; i < argTypes.size(); i++)
                {
                    switch (argTypes[i])
                    {
                        case OPCodeArgumentType::Unknown:
                            throw StringException(std::string("Don't know how to disassemble OP_") +
                                                  OPCode_Info[(uint8_t)instruction.opcode].first);

                        case OPCodeArgumentType::ByteLiteral:
                            sb.write((int32_t)instruction.arguments[i].bytev());
                            break;
                        case OPCodeArgumentType::UByteLiteral:
                            sb.write((uint32_t)instruction.arguments[i].ubytev());
                            break;
                        case OPCodeArgumentType::IntLiteral:
                            sb.write(instruction.arguments[i].intv());
                            break;
                        case OPCodeArgumentType::UIntLiteral:
                            sb.write(instruction.arguments[i].uintv());
                            break;

                        case OPCodeArgumentType::Int:
                            dumpInt(sb, instruction.arguments[i].intv());
                            break;
                        case OPCodeArgumentType::UInt:
                            dumpUInt(sb, instruction.arguments[i].uintv());
                            break;
                        case OPCodeArgumentType::Double:
                            dumpDouble(sb, instruction.arguments[i].doublev());
                            break;
                        case OPCodeArgumentType::String:
                            dumpString(sb, instruction.arguments[i].stringv());
                            break;
                        case OPCodeArgumentType::Namespace:
                            dumpNamespace(sb, instruction.arguments[i].namespacev());
                            break;
                        case OPCodeArgumentType::Multiname:
                            dumpMultiname(sb, instruction.arguments[i].multinamev());
                            break;
                        case OPCodeArgumentType::Class:
                            // if (instruction.arguments[i].classv is null)
                            //     sb ~ = "null";
                            // else
                            dumpString(
                                sb, refs.objects.getName(instruction.arguments[i].classv().get()));
                            break;
                        case OPCodeArgumentType::Method:
                            // if (instruction.arguments[i].methodv is null)
                            //     sb ~ = "null";
                            // else
                            dumpString(
                                sb, refs.objects.getName(instruction.arguments[i].methodv().get()));
                            break;

                        case OPCodeArgumentType::JumpTarget:
                        case OPCodeArgumentType::SwitchDefaultTarget:
                            dumpLabel(sb, instruction.arguments[i].jumpTarget());
                            break;

                        case OPCodeArgumentType::SwitchTargets:
                        {
                            sb << '[';
                            const auto& targets = instruction.arguments[i].switchTargets();
                            for (size_t i = 0; i < targets.size(); i++)
                            {
                                dumpLabel(sb, targets[i]);
                                if (i < targets.size() - 1)
                                {
                                    sb << ", ";
                                }
                            }
                            sb << ']';
                        }
                        break;
                    }
                    if (i < argTypes.size() - 1)
                    {
                        sb << ", ";
                    }
                }
            }
            sb.newLine();
        }
        checkLabel(instructions.size());
    }
};
