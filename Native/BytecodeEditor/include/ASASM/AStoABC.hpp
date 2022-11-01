#pragma once

#include "ABC/ABCFile.hpp"
#include "ASASM/ASTraitsVisitor.hpp"
#include "ASASM/Class.hpp"
#include "ASASM/Metadata.hpp"
#include "ASASM/Method.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Namespace.hpp"
#include "utils/ValuePool.hpp"

#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace ASASM
{
    class AStoABC : public ASTraitsVisitor
    {
    public:
        ValuePool<int64_t> ints;
        ValuePool<uint64_t> uints;
        ValuePool<double> doubles;
        ValuePool<std::string> strings;
        ValuePool<ASASM::Namespace> namespaces;
        ValuePool<std::vector<ASASM::Namespace>> namespaceSets;
        ValuePool<ASASM::Multiname> multinames;
        ValuePool<ASASM::Metadata, false> metadatas;
        ValuePool<std::shared_ptr<ASASM::Class>, false> classes;
        ValuePool<std::shared_ptr<ASASM::Method>, false> methods;

        ABC::ABCFile abc;

        void visitInt(int64_t v) { ints.add(v); }

        void visitUint(uint64_t v) { uints.add(v); }

        void visitDouble(double v) { doubles.add(v); }

        void visitString(const std::string& v) { strings.add(v); }

        void visitNamespace(const Namespace& ns)
        {
            if (namespaces.add(ns))
            {
                if (ns.kind != ABCType::Void)
                {
                    visitString(ns.name);
                }
            }
        }

        void visitNamespaceSet(const std::vector<ASASM::Namespace>& nsSet)
        {
            if (namespaceSets.add(nsSet))
            {
                for (const auto& ns : nsSet)
                {
                    visitNamespace(ns);
                }
            }
        }

        void visitMultiname(const ASASM::Multiname& multiname)
        {
            if (multinames.notAdded(multiname))
            {
                if (multiname.kind != ABCType::Void)
                {
                    switch (multiname.kind)
                    {
                        case ABCType::QName:
                        case ABCType::QNameA:
                            visitNamespace(multiname.qname().ns);
                            visitString(multiname.qname().name);
                            break;
                        case ABCType::RTQName:
                        case ABCType::RTQNameA:
                            visitString(multiname.rtqname().name);
                            break;
                        case ABCType::RTQNameL:
                        case ABCType::RTQNameLA:
                            break;
                        case ABCType::Multiname:
                        case ABCType::MultinameA:
                            visitString(multiname.multiname().name);
                            visitNamespaceSet(multiname.multiname().nsSet);
                            break;
                        case ABCType::MultinameL:
                        case ABCType::MultinameLA:
                            visitNamespaceSet(multiname.multinamel().nsSet);
                            break;
                        case ABCType::TypeName:
                            visitMultiname(multiname.Typename().name());
                            for (const auto& param : multiname.Typename().params())
                            {
                                visitMultiname(param);
                            }
                            break;
                        default:
                            throw StringException("Unknown multiname kind");
                    }
                }

                bool unrecursiveMultiname = multinames.add(multiname);
                (void)unrecursiveMultiname;
                assert(unrecursiveMultiname);
            }
        }

        void visitScript(const ASASM::Script& script)
        {
            visitTraits(script.traits);
            if (script.sinit)
            {
                visitMethod(script.sinit);
            }
        }

        void visitTrait(const ASASM::Trait& trait)
        {
            visitMultiname(trait.name);
            switch (trait.kind)
            {
                case TraitKind::Slot:
                case TraitKind::Const:
                    visitMultiname(trait.vSlot().typeName);
                    visitValue(trait.vSlot().value);
                    break;
                case TraitKind::Class:
                    if (trait.vClass().vclass)
                    {
                        visitClass(trait.vClass().vclass);
                    }
                    break;
                case TraitKind::Function:
                    if (trait.vFunction().vfunction)
                    {
                        visitMethod(trait.vFunction().vfunction);
                    }
                    break;
                case TraitKind::Method:
                case TraitKind::Getter:
                case TraitKind::Setter:
                    if (trait.vMethod().vmethod)
                    {
                        visitMethod(trait.vMethod().vmethod);
                    }
                    break;
                default:
                    throw StringException("Unknown trait kind");
            }
            for (const auto& md : trait.metadata)
            {
                visitMetadata(md);
            }
        }

        void visitMetadata(const ASASM::Metadata& metadata)
        {
            if (metadatas.add(metadata))
            {
                if (!metadata.data.empty())
                {
                    visitString(metadata.name);
                    for (const auto& md : metadata.data)
                    {
                        visitString(md.first);
                        visitString(md.second);
                    }
                }
            }
        }

        void visitValue(const ASASM::Value& value)
        {
            switch (value.vkind)
            {
                case ABCType::Integer:
                    visitInt(value.vint());
                    break;
                case ABCType::UInteger:
                    visitUint(value.vuint());
                    break;
                case ABCType::Double:
                    visitDouble(value.vdouble());
                    break;
                case ABCType::Utf8:
                    visitString(value.vstring());
                    break;
                case ABCType::Namespace:
                case ABCType::PackageNamespace:
                case ABCType::PackageInternalNs:
                case ABCType::ProtectedNamespace:
                case ABCType::ExplicitNamespace:
                case ABCType::StaticProtectedNs:
                case ABCType::PrivateNamespace:
                    visitNamespace(value.vnamespace());
                    break;
                case ABCType::True:
                case ABCType::False:
                case ABCType::Null:
                case ABCType::Undefined:
                    break;
                default:
                    throw StringException("Unknown value type");
            }
        }

        void visitClass(const std::shared_ptr<ASASM::Class>& vclass)
        {
            if (classes.add(vclass))
            {
                if (vclass)
                {
                    visitMethod(vclass->cinit);
                    visitTraits(vclass->traits);
                    visitInstance(vclass->instance);
                }
            }
        }

        void visitMethod(const std::shared_ptr<ASASM::Method>& method)
        {
            if (methods.add(method))
            {
                if (method)
                {
                    for (const auto& t : method->paramTypes)
                    {
                        visitMultiname(t);
                    }
                    visitMultiname(method->returnType);
                    visitString(method->name);
                    for (const auto& v : method->options)
                    {
                        visitValue(v);
                    }
                    for (const auto& n : method->paramNames)
                    {
                        visitString(n);
                    }

                    if (method->vbody)
                    {
                        visitMethodBody(*method->vbody);
                    }
                }
            }
        }

        void visitMethodBody(const ASASM::MethodBody& body)
        {
            if (body.method.lock() != nullptr)
            {
                for (const auto& instruction : body.instructions)
                {
                    for (size_t i = 0; i < OPCode_Info[(uint8_t)instruction.opcode].second.size();
                         i++)
                    {
                        switch (OPCode_Info[(uint8_t)instruction.opcode].second[i])
                        {
                            case OPCodeArgumentType::Unknown:
                                throw StringException(
                                    "Don't know how to visit OP_" +
                                    std::string(OPCode_Info[(uint8_t)instruction.opcode].first));

                            case OPCodeArgumentType::ByteLiteral:
                            case OPCodeArgumentType::UByteLiteral:
                            case OPCodeArgumentType::IntLiteral:
                            case OPCodeArgumentType::UIntLiteral:
                                break;

                            case OPCodeArgumentType::Int:
                                visitInt(instruction.arguments[i].intv());
                                break;
                            case OPCodeArgumentType::UInt:
                                visitUint(instruction.arguments[i].uintv());
                                break;
                            case OPCodeArgumentType::Double:
                                visitDouble(instruction.arguments[i].doublev());
                                break;
                            case OPCodeArgumentType::String:
                                visitString(instruction.arguments[i].stringv());
                                break;
                            case OPCodeArgumentType::Namespace:
                                visitNamespace(instruction.arguments[i].namespacev());
                                break;
                            case OPCodeArgumentType::Multiname:
                                visitMultiname(instruction.arguments[i].multinamev());
                                break;
                            case OPCodeArgumentType::Class:
                                visitClass(instruction.arguments[i].classv());
                                break;
                            case OPCodeArgumentType::Method:
                                visitMethod(instruction.arguments[i].methodv());
                                break;

                            case OPCodeArgumentType::JumpTarget:
                            case OPCodeArgumentType::SwitchDefaultTarget:
                            case OPCodeArgumentType::SwitchTargets:
                                break;
                            default:
                                assert(false);
                        }
                    }
                }

                for (const auto& exception : body.exceptions)
                {
                    visitMultiname(exception.excType);
                    visitMultiname(exception.varName);
                }

                visitMethod(body.method.lock());
                visitTraits(body.traits);
            }
        }

        void visitInstance(const ASASM::Instance& instance)
        {
            visitMultiname(instance.name);
            visitMultiname(instance.superName);
            visitNamespace(instance.protectedNs);
            for (const auto& intf : instance.interfaces)
            {
                visitMultiname(intf);
            }
            visitMethod(instance.iinit);
            visitTraits(instance.traits);
        }

        void run() override
        {
            for (const auto& script : as.scripts)
            {
                visitScript(script);
            }
            for (const auto& vclass : as.orphanClasses)
            {
                visitClass(vclass);
            }
            for (const auto& vmethod : as.orphanMethods)
            {
                visitMethod(vmethod);
            }
        }

        uint32_t getValueIndex(const ASASM::Value& value)
        {
            switch (value.vkind)
            {
                case ABCType::Integer:
                    return ints.get(value.vint());
                case ABCType::UInteger:
                    return uints.get(value.vuint());
                case ABCType::Double:
                    return doubles.get(value.vdouble());
                case ABCType::Utf8:
                    return strings.get(value.vstring());
                case ABCType::Namespace:
                case ABCType::PackageNamespace:
                case ABCType::PackageInternalNs:
                case ABCType::ProtectedNamespace:
                case ABCType::ExplicitNamespace:
                case ABCType::StaticProtectedNs:
                case ABCType::PrivateNamespace:
                    return namespaces.get(value.vnamespace());
                case ABCType::True:
                case ABCType::False:
                case ABCType::Null:
                case ABCType::Undefined:
                    return (uint32_t)value.vkind; // must be non-zero for True/False/Null
                default:
                    throw StringException("Unknown type");
            }
        }

        void registerClassDependencies()
        {
            std::map<ASASM::Multiname, std::shared_ptr<ASASM::Class>> classByName;

            auto classObjects = classes.getPreliminaryValues();
            for (const auto& c : classObjects)
            {
                classByName[c->instance.name] = c;
            }

            for (const auto& c : classObjects)
            {
                const auto depWork = [this, &c, &classByName](const ASASM::Multiname& dep)
                {
                    if (dep.kind != ABCType::Void)
                    {
                        for (const auto& depName : dep.toQNames())
                        {
                            if (classByName.contains(depName))
                            {
                                classes.registerDependency(c, classByName[depName]);
                            }
                        }
                    }
                };

                depWork(c->instance.superName);
                for (const auto& intf : c->instance.interfaces)
                {
                    depWork(intf);
                }
            }
        }

        void registerMultinameDependencies()
        {
            for (const auto& m : multinames.getPreliminaryValues())
            {
                if (m.kind == ABCType::TypeName)
                {
                    multinames.registerDependency(m, m.Typename().name());
                    for (const auto& t : m.Typename().params())
                    {
                        if (t.kind != ABCType::Void)
                        {
                            multinames.registerDependency(m, t);
                        }
                    }
                }
            }
        }

        explicit AStoABC(const ASProgram& as) : ASTraitsVisitor(as)
        {
            abc.minorVersion = as.minorVersion;
            abc.majorVersion = as.majorVersion;

            run();

            registerClassDependencies();
            registerMultinameDependencies();

            ints.finalize();
            uints.finalize();
            doubles.finalize();
            strings.finalize();
            namespaces.finalize();
            namespaceSets.finalize();
            multinames.finalize();
            metadatas.finalize();
            classes.finalize();
            methods.finalize();

            abc.ints    = ints.values;
            abc.uints   = uints.values;
            abc.doubles = doubles.values;
            abc.strings = strings.values;

            abc.namespaces.reserve(namespaces.values.size());
            for (size_t i = 1; i < namespaces.values.size(); i++)
            {
                abc.namespaces.emplace_back(
                    namespaces.values[i].kind, strings.get(namespaces.values[i].name));
            }

            abc.namespaceSets.reserve(namespaceSets.values.size());
            for (size_t i = 1; i < namespaceSets.values.size(); i++)
            {
                std::vector<int32_t>& nsSet = abc.namespaceSets.emplace_back(
                    std::vector<int32_t>(namespaceSets.values[i].size()));
                for (size_t j = 0; j < namespaceSets.values[i].size(); j++)
                {
                    nsSet[j] = namespaces.get(namespaceSets.values[i][j]);
                }
            }

            abc.multinames.reserve(multinames.values.size());
            for (size_t i = 1; i < multinames.values.size(); i++)
            {
                ABC::Multiname& multiname = abc.multinames.emplace_back();
                multiname.kind            = multinames.values[i].kind;
                switch (multinames.values[i].kind)
                {
                    case ABCType::QName:
                    case ABCType::QNameA:
                        multiname.qname({.ns = namespaces.get(multinames.values[i].qname().ns),
                            .name            = strings.get(multinames.values[i].qname().name)});
                        break;
                    case ABCType::RTQName:
                    case ABCType::RTQNameA:
                        multiname.rtqname(
                            {.name = strings.get(multinames.values[i].rtqname().name)});
                        break;
                    case ABCType::RTQNameL:
                    case ABCType::RTQNameLA:
                        multiname.rtqnamel({});
                        break;
                    case ABCType::Multiname:
                    case ABCType::MultinameA:
                        multiname.multiname({.name =
                                                 strings.get(multinames.values[i].multiname().name),
                            .nsSet = namespaceSets.get(multinames.values[i].multiname().nsSet)});
                        break;
                    case ABCType::MultinameL:
                    case ABCType::MultinameLA:
                        multiname.multinamel(
                            {.nsSet = namespaceSets.get(multinames.values[i].multinamel().nsSet)});
                        break;
                    case ABCType::TypeName:
                    {
                        ABC::Multiname::_Typename tn = {
                            .name   = multinames.get(multinames.values[i].Typename().name()),
                            .params = std::vector<uint32_t>(
                                multinames.values[i].Typename().params().size())};
                        for (size_t j = 0; j < multinames.values[i].Typename().params().size(); j++)
                        {
                            tn.params[j] =
                                multinames.get(multinames.values[i].Typename().params()[j]);
                        }
                        multiname.Typename(std::move(tn));
                    }
                    break;
                    default:
                        throw StringException("Unknown Multiname kind");
                }
            }

            abc.metadata.reserve(metadatas.values.size());
            for (size_t i = 0; i < metadatas.values.size(); i++)
            {
                ABC::Metadata& metadata = abc.metadata.emplace_back(
                    strings.get(metadatas.values[i].name),
                    std::vector<std::pair<uint32_t, uint32_t>>(metadatas.values[i].data.size()));
                for (size_t j = 0; j < metadatas.values[i].data.size(); j++)
                {
                    metadata.data[j] = {strings.get(metadatas.values[i].data[j].first),
                        strings.get(metadatas.values[i].data[j].second)};
                }
            }

            std::vector<ASASM::MethodBody> bodies;

            abc.methods.reserve(methods.values.size());
            for (size_t i = 0; i < methods.values.size(); i++)
            {
                ABC::MethodInfo& info = abc.methods.emplace_back();

                info.paramTypes.reserve(methods.values[i]->paramTypes.size());
                for (const auto& paramType : methods.values[i]->paramTypes)
                {
                    info.paramTypes.emplace_back(multinames.get(paramType));
                }
                info.returnType = multinames.get(methods.values[i]->returnType);
                info.name       = strings.get(methods.values[i]->name);
                info.flags      = methods.values[i]->flags;
                info.options.reserve(methods.values[i]->options.size());
                for (const auto& option : methods.values[i]->options)
                {
                    info.options.emplace_back(getValueIndex(option), option.vkind);
                }
                info.paramNames.reserve(methods.values[i]->paramNames.size());
                for (const auto& paramName : methods.values[i]->paramNames)
                {
                    info.paramNames.emplace_back(strings.get(paramName));
                }

                if (methods.values[i]->vbody)
                {
                    bodies.emplace_back(*methods.values[i]->vbody);
                }
            }

            abc.instances.reserve(classes.values.size());
            for (size_t i = 0; i < classes.values.size(); i++)
            {
                ABC::Instance& instance = abc.instances.emplace_back();

                instance.name        = multinames.get(classes.values[i]->instance.name);
                instance.superName   = multinames.get(classes.values[i]->instance.superName);
                instance.flags       = classes.values[i]->instance.flags;
                instance.protectedNs = namespaces.get(classes.values[i]->instance.protectedNs);
                instance.interfaces.reserve(classes.values[i]->instance.interfaces.size());
                for (const auto& iface : classes.values[i]->instance.interfaces)
                {
                    instance.interfaces.emplace_back(multinames.get(iface));
                }
                instance.iinit  = methods.get(classes.values[i]->instance.iinit);
                instance.traits = convertTraits(classes.values[i]->instance.traits);
            }

            abc.classes.reserve(classes.values.size());
            for (size_t i = 0; i < classes.values.size(); i++)
            {
                abc.classes.emplace_back(methods.get(classes.values[i]->cinit),
                    convertTraits(classes.values[i]->traits));
            }

            abc.scripts.reserve(as.scripts.size());
            for (size_t i = 0; i < as.scripts.size(); i++)
            {
                abc.scripts.emplace_back(
                    methods.get(as.scripts[i].sinit), convertTraits(as.scripts[i].traits));
            }

            abc.bodies.reserve(bodies.size());
            for (size_t i = 0; i < bodies.size(); i++)
            {
                ABC::MethodBody& body = abc.bodies.emplace_back();
                body.method           = methods.get(bodies[i].method.lock());
                body.maxStack         = bodies[i].maxStack;
                body.localCount       = bodies[i].localCount;
                body.initScopeDepth   = bodies[i].initScopeDepth;
                body.maxScopeDepth    = bodies[i].maxScopeDepth;
                body.instructions.reserve(bodies[i].instructions.size());
                for (size_t j = 0; j < bodies[i].instructions.size(); j++)
                {
                    body.instructions.emplace_back(convertInstruction(bodies[i].instructions[j]));
                }
                body.exceptions.reserve(bodies[i].exceptions.size());
                for (const auto& exception : bodies[i].exceptions)
                {
                    body.exceptions.emplace_back(exception.from, exception.to, exception.target,
                        multinames.get(exception.excType), multinames.get(exception.varName));
                }
                body.traits = convertTraits(bodies[i].traits);
            }
        }

        std::vector<ABC::TraitsInfo> convertTraits(const std::vector<ASASM::Trait>& traits)
        {
            std::vector<ABC::TraitsInfo> ret;
            ret.reserve(traits.size());

            for (const auto& oTrait : traits)
            {
                ABC::TraitsInfo& nTrait = ret.emplace_back();
                nTrait.name             = multinames.get(oTrait.name);
                nTrait.kind(oTrait.kind);
                nTrait.attr(oTrait.attributes);
                switch (oTrait.kind)
                {
                    case TraitKind::Slot:
                    case TraitKind::Const:
                        nTrait.Slot = {.slotId = oTrait.vSlot().slotId,
                            .typeName          = multinames.get(oTrait.vSlot().typeName),
                            .vindex            = getValueIndex(oTrait.vSlot().value),
                            .vkind             = oTrait.vSlot().value.vkind};
                        break;
                    case TraitKind::Class:
                        nTrait.Class = {.slotId = oTrait.vClass().slotId,
                            .classi             = classes.get(oTrait.vClass().vclass)};
                        break;
                    case TraitKind::Function:
                        nTrait.Function = {.slotId = oTrait.vFunction().slotId,
                            .functioni             = methods.get(oTrait.vFunction().vfunction)};
                        break;
                    case TraitKind::Method:
                    case TraitKind::Getter:
                    case TraitKind::Setter:
                        nTrait.Method = {.dispId = oTrait.vMethod().dispId,
                            .method              = methods.get(oTrait.vMethod().vmethod)};
                        break;
                    default:
                        throw StringException("Unknown trait kind");
                }
                nTrait.metadata.reserve(oTrait.metadata.size());
                for (const auto& md : oTrait.metadata)
                {
                    nTrait.metadata.emplace_back(metadatas.get(md));
                }
            }
            return ret;
        }

        ABC::Instruction convertInstruction(const ASASM::Instruction& instruction)
        {
            ABC::Instruction ret;
            ret.opcode = instruction.opcode;
            ret.arguments.reserve(instruction.arguments.size());

            for (size_t i = 0; i < OPCode_Info[(uint8_t)instruction.opcode].second.size(); i++)
            {
                ABC::Instruction::Argument& newArg = ret.arguments.emplace_back();
                switch (OPCode_Info[(uint8_t)instruction.opcode].second[i])
                {
                    case OPCodeArgumentType::Unknown:
                    default:
                        throw StringException(
                            "Don't know how to convert OP_" +
                            std::string(OPCode_Info[(uint8_t)instruction.opcode].first));

                    case OPCodeArgumentType::ByteLiteral:
                        newArg.bytev(instruction.arguments[i].bytev());
                        break;
                    case OPCodeArgumentType::UByteLiteral:
                        newArg.ubytev(instruction.arguments[i].ubytev());
                        break;
                    case OPCodeArgumentType::IntLiteral:
                        newArg.intv(instruction.arguments[i].intv());
                        break;
                    case OPCodeArgumentType::UIntLiteral:
                        newArg.uintv(instruction.arguments[i].uintv());
                        break;

                    case OPCodeArgumentType::Int:
                        newArg.index(ints.get(instruction.arguments[i].intv()));
                        break;
                    case OPCodeArgumentType::UInt:
                        newArg.index(uints.get(instruction.arguments[i].uintv()));
                        break;
                    case OPCodeArgumentType::Double:
                        newArg.index(doubles.get(instruction.arguments[i].doublev()));
                        break;
                    case OPCodeArgumentType::String:
                        newArg.index(strings.get(instruction.arguments[i].stringv()));
                        break;
                    case OPCodeArgumentType::Namespace:
                        newArg.index(namespaces.get(instruction.arguments[i].namespacev()));
                        break;
                    case OPCodeArgumentType::Multiname:
                        newArg.index(multinames.get(instruction.arguments[i].multinamev()));
                        break;
                    case OPCodeArgumentType::Class:
                        if (instruction.arguments[i].classv() == nullptr)
                        {
                            newArg.index(abc.classes.size());
                        }
                        else
                        {
                            newArg.index(classes.get(instruction.arguments[i].classv()));
                        }
                        break;
                    case OPCodeArgumentType::Method:
                        if (instruction.arguments[i].methodv() == nullptr)
                        {
                            newArg.index(abc.methods.size());
                        }
                        else
                        {
                            newArg.index(methods.get(instruction.arguments[i].methodv()));
                        }
                        break;

                    case OPCodeArgumentType::JumpTarget:
                    case OPCodeArgumentType::SwitchDefaultTarget:
                        newArg.jumpTarget(instruction.arguments[i].jumpTarget());
                        break;

                    case OPCodeArgumentType::SwitchTargets:
                        newArg.switchTargets(
                            std::vector<ABC::Label>(instruction.arguments[i].switchTargets()));
                        break;
                }
            }

            return ret;
        }
    };
}
