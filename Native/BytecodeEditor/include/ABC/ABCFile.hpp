#pragma once

#include "ABC/Class.hpp"
#include "ABC/Instance.hpp"
#include "ABC/Metadata.hpp"
#include "ABC/MethodBody.hpp"
#include "ABC/MethodInfo.hpp"
#include "ABC/Multiname.hpp"
#include "ABC/Namespace.hpp"
#include "ABC/Script.hpp"
#include <algorithm>
#include <limits>
#include <stdint.h>
#include <string>

namespace SWFABC
{
    struct ABCFile
    {
        uint16_t minorVersion, majorVersion;
        std::vector<int64_t> ints;
        std::vector<uint64_t> uints;
        std::vector<double> doubles;
        std::vector<std::string> strings;
        std::vector<Namespace> namespaces;
        std::vector<std::vector<int32_t>> namespaceSets;
        std::vector<Multiname> multinames;

        std::vector<MethodInfo> methods;
        std::vector<Metadata> metadata;
        std::vector<Instance> instances;
        std::vector<Class> classes;
        std::vector<Script> scripts;
        std::vector<MethodBody> bodies;

        static constexpr int64_t NULL_INT   = INT64_MAX;
        static constexpr uint64_t NULL_UINT = UINT64_MAX;
        static constexpr double NULL_DOUBLE = std::numeric_limits<double>::quiet_NaN();

        static constexpr uint64_t MAX_UINT = (((uint64_t)1) << 36) - 1;
        static constexpr int64_t MAX_INT   = MAX_UINT / 2;
        static constexpr int64_t MIN_INT   = -MAX_INT - 1;

        ABCFile()
        {
            majorVersion = 46;
            minorVersion = 16;

            ints    = {NULL_INT};
            uints   = {NULL_UINT};
            doubles = {NULL_DOUBLE};

            strings       = {""};
            namespaces    = {{}};
            namespaceSets = {{}};
            multinames    = {{}};
        }

        void merge(const ABCFile& other)
        {
            const uint32_t intOffset = ints.size() - 1;
            if (other.ints.size() > 1)
            {
                ints.reserve(ints.size() + other.ints.size() - 1);
                ints.insert(ints.end(), other.ints.begin() + 1, other.ints.end());
            }
            const uint32_t uintOffset = uints.size() - 1;
            if (other.uints.size() > 1)
            {
                uints.reserve(uints.size() + other.uints.size() - 1);
                uints.insert(uints.end(), other.uints.begin() + 1, other.uints.end());
            }
            const uint32_t doubleOffset = doubles.size() - 1;
            if (other.doubles.size() > 1)
            {
                doubles.reserve(doubles.size() + other.doubles.size() - 1);
                doubles.insert(doubles.end(), other.doubles.begin() + 1, other.doubles.end());
            }
            const uint32_t stringOffset = strings.size() - 1;
            if (other.strings.size() > 1)
            {
                strings.reserve(strings.size() + other.strings.size() - 1);
                strings.insert(strings.end(), other.strings.begin() + 1, other.strings.end());
            }
            // size_t namespaceOffset = namespaces.size() - 1; // Namespaces are handled differently
            const uint32_t namespaceSetOffset = namespaceSets.size() - 1;
            const uint32_t multinameOffset    = multinames.size() - 1;

            const uint32_t methodOffset   = methods.size();
            const uint32_t metadataOffset = metadata.size();
            const uint32_t instanceOffset = instances.size();
            const uint32_t classOffset    = classes.size();
            const uint32_t scriptOffset   = scripts.size();
            const uint32_t bodyOffset     = bodies.size();

            namespaces.reserve(namespaces.size() + other.namespaces.size() - 1);
            namespaceSets.reserve(namespaceSets.size() + other.namespaceSets.size() - 1);
            multinames.reserve(multinames.size() + other.multinames.size() - 1);

            // Simple copies; we don't care about duplication

            const auto fixInt    = [intOffset](uint32_t v) { return v == 0 ? 0 : v + intOffset; };
            const auto fixUint   = [uintOffset](uint32_t v) { return v == 0 ? 0 : v + uintOffset; };
            const auto fixDouble = [doubleOffset](uint32_t v)
            { return v == 0 ? 0 : v + doubleOffset; };
            const auto fixString = [stringOffset](uint32_t v)
            { return v == 0 ? 0 : v + stringOffset; };
            const auto fixNamespaceSet = [namespaceSetOffset](uint32_t v)
            { return v == 0 ? 0 : v + namespaceSetOffset; };
            const auto fixMultiname = [multinameOffset](uint32_t v)
            { return v == 0 ? 0 : v + multinameOffset; };

            const auto fixMethod   = [methodOffset](uint32_t v) { return v + methodOffset; };
            const auto fixMetadata = [metadataOffset](uint32_t v) { return v + metadataOffset; };
            const auto fixClass    = [classOffset](uint32_t v) { return v + classOffset; };

            // Most complex: namespaces
            // We need to ensure that copies in the other ABC aren't read as distinct from this
            // one's
            std::vector<uint32_t> newNamespaceIndices(other.namespaces.size());
            newNamespaceIndices[0] = 0;
            for (size_t i = 1; i < other.namespaces.size(); i++)
            {
                const auto& ns = other.namespaces[i];
                if (auto found = std::find_if(namespaces.begin(), namespaces.end(),
                        [this, &ns, &other](const Namespace& check) {
                            return check.kind == ns.kind &&
                                   strings[check.name] == other.strings[ns.name];
                        });
                    found != namespaces.end())
                {
                    newNamespaceIndices[i] = std::distance(namespaces.begin(), found);
                }
                else
                {
                    newNamespaceIndices[i] = namespaces.size();
                    namespaces.emplace_back(ns.kind, fixString(ns.name));
                }
            }

            const auto fixNamespace = [&newNamespaceIndices](uint32_t v)
            { return newNamespaceIndices[v]; };

            const auto transformInPlace = [](auto& range, const auto& mod)
            { return std::transform(range.begin(), range.end(), range.begin(), mod); };

            // From now on it's fairly simple, we just need to make sure to edit all the indices to
            // use the new offsets
            for (size_t i = 1; i < other.namespaceSets.size(); i++)
            {
                transformInPlace(namespaceSets.emplace_back(other.namespaceSets[i]), fixNamespace);
            }

            for (size_t i = 1; i < other.multinames.size(); i++)
            {
                auto& mname = multinames.emplace_back(other.multinames[i]);
                switch (mname.kind)
                {
                    case ABCType::QName:
                    case ABCType::QNameA:
                        mname.qname().name = fixString(mname.qname().name);
                        mname.qname().ns   = fixNamespace(mname.qname().ns);
                        break;
                    case ABCType::RTQName:
                    case ABCType::RTQNameA:
                        mname.rtqname().name = fixString(mname.rtqname().name);
                        break;
                    case ABCType::RTQNameL:
                    case ABCType::RTQNameLA:
                        break;
                    case ABCType::Multiname:
                    case ABCType::MultinameA:
                        mname.multiname().name  = fixString(mname.multiname().name);
                        mname.multiname().nsSet = fixNamespaceSet(mname.multiname().nsSet);
                        break;
                    case ABCType::MultinameL:
                    case ABCType::MultinameLA:
                        mname.multinamel().nsSet = fixNamespaceSet(mname.multinamel().nsSet);
                        break;
                    case ABCType::TypeName:
                        mname.Typename().name = fixMultiname(mname.Typename().name);
                        transformInPlace(mname.Typename().params, fixMultiname);
                        break;
                }
            }

            // Now we get into the funky stuff:
            const auto fixValue = [&](ABCType kind, uint32_t val)
            {
                switch (kind)
                {
                    case ABCType::Integer:
                        return fixInt(val);
                    case ABCType::UInteger:
                        return fixUint(val);
                    case ABCType::Double:
                        return fixDouble(val);
                    case ABCType::Utf8:
                        return fixString(val);
                    case ABCType::Namespace:
                    case ABCType::PackageNamespace:
                    case ABCType::PackageInternalNs:
                    case ABCType::ProtectedNamespace:
                    case ABCType::ExplicitNamespace:
                    case ABCType::StaticProtectedNs:
                    case ABCType::PrivateNamespace:
                        return fixNamespace(val);
                    default:
                        return 0u;
                }
            };
            const auto transformMethod = [&](MethodInfo mi)
            {
                transformInPlace(mi.paramTypes, fixMultiname);
                mi.returnType = fixMultiname(mi.returnType);
                mi.name       = fixString(mi.name);
                transformInPlace(mi.options,
                    [&](const OptionDetail& od) {
                        return OptionDetail{fixValue(od.kind, od.value), od.kind};
                    });
                transformInPlace(mi.paramNames, fixString);

                return mi;
            };
            const auto transformMetadata = [&](Metadata m)
            {
                m.name = fixString(m.name);
                transformInPlace(m.data, [&](const std::pair<uint32_t, uint32_t>& p)
                    { return std::make_pair(fixString(p.first), fixString(p.second)); });
                return m;
            };
            const auto transformTrait = [&](TraitsInfo t)
            {
                t.name = fixMultiname(t.name);

                switch (t.kind())
                {
                    case TraitKind::Slot:
                    case TraitKind::Const:
                        t.Slot.typeName = fixMultiname(t.Slot.typeName);
                        t.Slot.vindex   = fixValue(t.Slot.vkind, t.Slot.vindex);
                        break;
                    case TraitKind::Class:
                        t.Class.classi = fixClass(t.Class.classi);
                        break;
                    case TraitKind::Function:
                        t.Function.functioni = fixMethod(t.Function.functioni);
                        break;
                    case TraitKind::Getter:
                    case TraitKind::Setter:
                    case TraitKind::Method:
                        t.Method.method = fixMethod(t.Method.method);
                        break;
                }

                transformInPlace(t.metadata, fixMetadata);

                return t;
            };
            const auto transformInstance = [&](Instance i)
            {
                i.name        = fixMultiname(i.name);
                i.superName   = fixMultiname(i.superName);
                i.protectedNs = fixNamespace(i.protectedNs);
                transformInPlace(i.interfaces, fixMultiname);
                i.iinit = fixMethod(i.iinit);
                transformInPlace(i.traits, transformTrait);

                return i;
            };
            const auto transformClass = [&](Class c)
            {
                c.cinit = fixMethod(c.cinit);
                transformInPlace(c.traits, transformTrait);

                return c;
            };
            const auto transformScript = [&](Script s)
            {
                s.sinit = fixMethod(s.sinit);
                transformInPlace(s.traits, transformTrait);

                return s;
            };
            const auto transformInstruction = [&](Instruction instr)
            {
                for (size_t i = 0; i < instr.arguments.size(); i++)
                {
                    auto& arg = instr.arguments[i];
                    switch (OPCode_Info[(uint8_t)instr.opcode].second[i])
                    {
                        case OPCodeArgumentType::Int:
                            arg.index() = fixInt(arg.index());
                            break;
                        case OPCodeArgumentType::UInt:
                            arg.index() = fixUint(arg.index());
                            break;
                        case OPCodeArgumentType::Double:
                            arg.index() = fixDouble(arg.index());
                            break;
                        case OPCodeArgumentType::String:
                            arg.index() = fixString(arg.index());
                            break;
                        case OPCodeArgumentType::Namespace:
                            arg.index() = fixNamespace(arg.index());
                            break;
                        case OPCodeArgumentType::Multiname:
                            arg.index() = fixMultiname(arg.index());
                            break;
                        case OPCodeArgumentType::Class:
                            arg.index() = fixClass(arg.index());
                            break;
                        case OPCodeArgumentType::Method:
                            arg.index() = fixMethod(arg.index());
                            break;
                        default:
                            break;
                    }
                }
                return instr;
            };
            const auto transformException = [&](ExceptionInfo e)
            {
                e.excType = fixMultiname(e.excType);
                e.varName = fixMultiname(e.varName);

                return e;
            };
            const auto transformBody = [&](MethodBody m)
            {
                m.method = fixMethod(m.method);
                transformInPlace(m.instructions, transformInstruction);
                transformInPlace(m.exceptions, transformException);
                transformInPlace(m.traits, transformTrait);

                return m;
            };

            methods.resize(methods.size() + other.methods.size());
            std::transform(other.methods.begin(), other.methods.end(),
                methods.begin() + methodOffset, transformMethod);

            metadata.resize(metadata.size() + other.metadata.size());
            std::transform(other.metadata.begin(), other.metadata.end(),
                metadata.begin() + metadataOffset, transformMetadata);

            instances.resize(instances.size() + other.instances.size());
            std::transform(other.instances.begin(), other.instances.end(),
                instances.begin() + instanceOffset, transformInstance);

            classes.resize(classes.size() + other.classes.size());
            std::transform(other.classes.begin(), other.classes.end(),
                classes.begin() + classOffset, transformClass);

            scripts.resize(scripts.size() + other.scripts.size());
            std::transform(other.scripts.begin(), other.scripts.end(),
                scripts.begin() + scriptOffset, transformScript);

            bodies.resize(bodies.size() + other.bodies.size());
            std::transform(other.bodies.begin(), other.bodies.end(), bodies.begin() + bodyOffset,
                transformBody);
        }
    };
}
