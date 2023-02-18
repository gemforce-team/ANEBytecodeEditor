#include "ASASM/ASProgram.hpp"
#include "ASASM/AStoABC.hpp"
#include <queue>

namespace
{
    constexpr size_t calcTypenameDepth(
        const std::vector<SWFABC::Multiname> multinames, const SWFABC::Multiname& m)
    {
        if (m.kind != ABCType::TypeName)
        {
            return 0;
        }

        size_t ret = 1 + calcTypenameDepth(multinames, multinames[m.Typename().name]);

        for (size_t param : m.Typename().params)
        {
            ret = std::max(ret, 1 + calcTypenameDepth(multinames, multinames[param]));
        }

        return ret;
    }
}

ASASM::ASProgram ASASM::ASProgram::fromABC(const SWFABC::ABCFile& abc)
{
    ASProgram asp;

    std::vector<Namespace> namespaces;
    std::vector<std::vector<Namespace>> namespaceSets;
    std::vector<Multiname> multinames;
    std::vector<Metadata> metadatas;
    std::vector<Instance> instances;

    std::vector<std::shared_ptr<Class>> classes;
    std::vector<std::shared_ptr<Method>> methods;

    std::vector<bool> methodAdded;
    std::vector<bool> classAdded;

    // Whether or not the class has already been converted
    std::vector<bool> classSet;

    std::vector<uint32_t> typenameQueue;

    const auto comparator = [&abc](uint32_t a, uint32_t b)
    {
        return calcTypenameDepth(abc.multinames, abc.multinames[a]) <
               calcTypenameDepth(abc.multinames, abc.multinames[b]);
    };

    // std::priority_queue<uint32_t, std::vector<uint32_t>, decltype(comparator)> typenameQueue(
    //     comparator);

    auto getMethod = [&](uint32_t index) -> std::shared_ptr<Method>&
    {
        methodAdded[index] = true;
        return methods[index];
    };
    const auto getClass = [&](uint32_t index) -> std::shared_ptr<Class>&
    {
        classAdded[index] = true;
        return classes[index];
    };

    const auto convertValue = [&](ABCType kind, uint32_t val)
    {
        Value ret;
        ret.vkind = kind;
        switch (kind)
        {
            case ABCType::Integer:
                ret.vint(abc.ints[val]); // WARNING: discarding extra bits
                break;
            case ABCType::UInteger:
                ret.vuint(abc.uints[val]); // WARNING: discarding extra bits
                break;
            case ABCType::Double:
                ret.vdouble(abc.doubles[val]);
                break;
            case ABCType::Utf8:
                ret.vstring(abc.strings[val]);
                break;
            case ABCType::Namespace:
            case ABCType::PackageNamespace:
            case ABCType::PackageInternalNs:
            case ABCType::ProtectedNamespace:
            case ABCType::ExplicitNamespace:
            case ABCType::StaticProtectedNs:
            case ABCType::PrivateNamespace:
                ret.vnamespace(namespaces[val]);
                break;
            case ABCType::True:
            case ABCType::False:
            case ABCType::Null:
            case ABCType::Undefined:
                break;
            default:
                throw StringException("Unknown type");
        }
        return ret;
    };

    const auto convertNamespace = [&](const SWFABC::Namespace& ns, int id) {
        return Namespace{ns.kind, abc.strings[ns.name], id};
    };

    const auto convertNamespaceSet = [&](const std::vector<int32_t>& nsSet)
    {
        std::vector<Namespace> ret(nsSet.size());
        for (size_t i = 0; i < nsSet.size(); i++)
        {
            ret[i] = namespaces[nsSet[i]];
        }
        return ret;
    };

    const auto convertMultiname = [&](const SWFABC::Multiname& multiname, uint32_t index)
    {
        Multiname ret;
        ret.kind = multiname.kind;
        switch (multiname.kind)
        {
            case ABCType::QName:
            case ABCType::QNameA:
                ret.qname({namespaces[multiname.qname().ns], abc.strings[multiname.qname().name]});
                break;
            case ABCType::RTQName:
            case ABCType::RTQNameA:
                ret.rtqname({abc.strings[multiname.rtqname().name]});
                break;
            case ABCType::RTQNameL:
            case ABCType::RTQNameLA:
                ret.rtqnamel({});
                break;
            case ABCType::Multiname:
            case ABCType::MultinameA:
                ret.multiname({abc.strings[multiname.multiname().name],
                    namespaceSets[multiname.multiname().nsSet]});
                break;
            case ABCType::MultinameL:
            case ABCType::MultinameLA:
                ret.multinamel({namespaceSets[multiname.multinamel().nsSet]});
                break;
            case ABCType::TypeName:
                // handled in postConvertMultiname; needs the sub-multinames to be processed
                // first
                typenameQueue.push_back(index);
                std::inplace_merge(typenameQueue.begin(), typenameQueue.end() - 1,
                    typenameQueue.end(), comparator);
                break;
            default:
                throw StringException("Unknown Multiname kind");
        }
        return ret;
    };

    const auto postConvertMultiname = [&](const SWFABC::Multiname& multiname, Multiname& edit)
    {
        if (multiname.kind == ABCType::TypeName)
        {
            Multiname::_Typename nTypeName{multinames[multiname.Typename().name]};
            nTypeName.params().reserve(multiname.Typename().params.size());
            for (size_t i = 0; i < multiname.Typename().params.size(); i++)
            {
                nTypeName.params().emplace_back(multinames[multiname.Typename().params[i]]);
            }

            edit.Typename(nTypeName);
        }
    };

    const auto convertMethod = [&](const SWFABC::MethodInfo& method, int id)
    {
        std::shared_ptr<Method> ret = std::make_shared<Method>();
        ret->paramTypes.reserve(method.paramTypes.size());
        for (const auto& param : method.paramTypes)
        {
            ret->paramTypes.emplace_back(multinames[param]);
        }
        ret->returnType = multinames[method.returnType];
        ret->name       = abc.strings[method.name];
        ret->flags      = method.flags;
        ret->options.reserve(method.options.size());
        for (const auto& option : method.options)
        {
            ret->options.emplace_back(convertValue(option.kind, option.value));
        }
        ret->paramNames.reserve(method.paramNames.size());
        for (const auto& name : method.paramNames)
        {
            ret->paramNames.emplace_back(abc.strings[name]);
        }
        ret->id = id;
        return ret;
    };

    const auto convertMetadata = [&](const SWFABC::Metadata& metadata)
    {
        Metadata ret;
        ret.name = abc.strings[metadata.name];
        ret.data.reserve(metadata.data.size());
        for (const auto& kv : metadata.data)
        {
            ret.data.emplace_back(abc.strings[kv.first], abc.strings[kv.second]);
        }
        return ret;
    };

    const auto convertTraits = [&](const std::vector<SWFABC::TraitsInfo>& traits)
    {
        std::vector<Trait> ret;
        ret.reserve(traits.size());
        for (const auto& trait : traits)
        {
            Trait add;
            add.name       = multinames[trait.name];
            add.kind       = trait.kind();
            add.attributes = trait.attr();
            switch (trait.kind())
            {
                case TraitKind::Slot:
                case TraitKind::Const:
                    add.vSlot({trait.Slot.slotId, multinames[trait.Slot.typeName],
                        convertValue(trait.Slot.vkind, trait.Slot.vindex)});
                    break;
                case TraitKind::Class:
                    if (classes.size() == 0 || !classSet[trait.Class.classi])
                    {
                        throw StringException("Forward class reference");
                    }
                    add.vClass({trait.Class.slotId, getClass(trait.Class.classi)});
                    break;
                case TraitKind::Function:
                    add.vFunction({trait.Function.slotId, getMethod(trait.Function.functioni)});
                    break;
                case TraitKind::Method:
                case TraitKind::Getter:
                case TraitKind::Setter:
                    add.vMethod({trait.Method.dispId, getMethod(trait.Method.method)});
                    break;
                default:
                    throw StringException("Unknown trait kind");
            }
            add.metadata.reserve(trait.metadata.size());
            for (const auto& md : trait.metadata)
            {
                add.metadata.emplace_back(metadatas[md]);
            }
            ret.emplace_back(std::move(add));
        }
        return ret;
    };

    const auto convertInstance = [&](const SWFABC::Instance& instance)
    {
        Instance ret;
        ret.name        = multinames[instance.name];
        ret.superName   = multinames[instance.superName];
        ret.flags       = instance.flags;
        ret.protectedNs = namespaces[instance.protectedNs];
        ret.interfaces.reserve(instance.interfaces.size());
        for (const auto& interface : instance.interfaces)
        {
            ret.interfaces.emplace_back(multinames[interface]);
        }
        ret.iinit  = getMethod(instance.iinit);
        ret.traits = convertTraits(instance.traits);
        return ret;
    };

    const auto convertClass = [&](const SWFABC::Class& clazz, uint32_t i)
    {
        classSet[i] = true;
        return std::shared_ptr<Class>(new Class(std::move(getMethod(clazz.cinit)),
            convertTraits(clazz.traits), std::move(instances[i])));
    };

    const auto convertScript = [&](const SWFABC::Script& script)
    {
        return std::shared_ptr<Script>(
            new Script{getMethod(script.sinit), convertTraits(script.traits)});
    };

    const auto convertInstruction = [&](const SWFABC::Instruction& instruction)
    {
        Instruction ret;
        ret.opcode = instruction.opcode;
        ret.arguments.reserve(instruction.arguments.size());

        for (size_t i = 0; i < instruction.arguments.size(); i++)
        {
            Instruction::Argument arg;
            switch (OPCode_Info[(uint8_t)instruction.opcode].second[i])
            {
                case OPCodeArgumentType::Unknown:
                    throw StringException(std::string("Don't know how to convert OP_") +
                                          OPCode_Info[(uint8_t)instruction.opcode].first);

                case OPCodeArgumentType::ByteLiteral:
                    arg.bytev(instruction.arguments[i].bytev());
                    break;
                case OPCodeArgumentType::UByteLiteral:
                    arg.ubytev(instruction.arguments[i].ubytev());
                    break;
                case OPCodeArgumentType::IntLiteral:
                    arg.intv(instruction.arguments[i].intv());
                    break;
                case OPCodeArgumentType::UIntLiteral:
                    arg.uintv(instruction.arguments[i].uintv());
                    break;

                case OPCodeArgumentType::Int:
                    arg.intv(abc.ints[instruction.arguments[i].index()]);
                    break;
                case OPCodeArgumentType::UInt:
                    arg.uintv(abc.uints[instruction.arguments[i].index()]);
                    break;
                case OPCodeArgumentType::Double:
                    arg.doublev(abc.doubles[instruction.arguments[i].index()]);
                    break;
                case OPCodeArgumentType::String:
                    arg.stringv(abc.strings[instruction.arguments[i].index()]);
                    break;
                case OPCodeArgumentType::Namespace:
                    arg.namespacev(instruction.arguments[i].index() < namespaces.size()
                                       ? namespaces[instruction.arguments[i].index()]
                                       : Namespace{});
                    break;
                case OPCodeArgumentType::Multiname:
                    arg.multinamev(instruction.arguments[i].index() < multinames.size()
                                       ? multinames[instruction.arguments[i].index()]
                                       : Multiname{});
                    break;
                case OPCodeArgumentType::Class:
                    arg.classv(instruction.arguments[i].index() < classes.size()
                                   ? classes[instruction.arguments[i].index()]
                                   : nullptr);
                    break;
                case OPCodeArgumentType::Method:
                    arg.methodv(instruction.arguments[i].index() < methods.size()
                                    ? methods[instruction.arguments[i].index()]
                                    : nullptr);
                    break;

                case OPCodeArgumentType::JumpTarget:
                case OPCodeArgumentType::SwitchDefaultTarget:
                    arg.jumpTarget(instruction.arguments[i].jumpTarget());
                    break;

                case OPCodeArgumentType::SwitchTargets:
                    arg.switchTargets(instruction.arguments[i].switchTargets());
                    break;
            }
            ret.arguments.emplace_back(std::move(arg));
        }
        return ret;
    };

    const auto convertBody = [&](const SWFABC::MethodBody& body)
    {
        MethodBody ret;
        ret.method         = methods[body.method];
        ret.maxStack       = body.maxStack;
        ret.localCount     = body.localCount;
        ret.initScopeDepth = body.initScopeDepth;
        ret.maxScopeDepth  = body.maxScopeDepth;
        ret.instructions.reserve(body.instructions.size());
        for (const auto& instruction : body.instructions)
        {
            ret.instructions.emplace_back(convertInstruction(instruction));
        }
        ret.exceptions.reserve(ret.exceptions.size());
        for (const auto& exception : body.exceptions)
        {
            ret.exceptions.emplace_back(exception.from, exception.to, exception.target,
                multinames[exception.excType], multinames[exception.varName]);
        }
        ret.traits = convertTraits(body.traits);
        ret.errors = body.errors;
        return ret;
    };

    asp.minorVersion = abc.minorVersion;
    asp.majorVersion = abc.majorVersion;

    namespaces.reserve(abc.namespaces.size());
    namespaceSets.reserve(abc.namespaceSets.size());
    multinames.reserve(abc.multinames.size());
    methods.reserve(abc.methods.size());
    metadatas.reserve(abc.metadata.size());
    instances.reserve(abc.instances.size());
    classes.reserve(abc.classes.size());
    asp.scripts.reserve(abc.scripts.size());

    classSet.resize(abc.classes.size(), false);
    classAdded.resize(abc.instances.size(), false);
    methodAdded.resize(abc.methods.size(), false);

    namespaces.emplace_back();
    for (size_t i = 1; i < abc.namespaces.size(); i++)
    {
        namespaces.emplace_back(convertNamespace(abc.namespaces[i], i));
    }

    namespaceSets.emplace_back();
    for (size_t i = 1; i < abc.namespaceSets.size(); i++)
    {
        namespaceSets.emplace_back(convertNamespaceSet(abc.namespaceSets[i]));
    }

    multinames.emplace_back();
    for (size_t i = 1; i < abc.multinames.size(); i++)
    {
        multinames.emplace_back(convertMultiname(abc.multinames[i], i));
    }
    for (uint32_t idx : typenameQueue)
    {
        postConvertMultiname(abc.multinames[idx], multinames[idx]);
    }

    for (size_t i = 0; i < abc.methods.size(); i++)
    {
        methods.emplace_back(convertMethod(abc.methods[i], i));
    }

    for (size_t i = 0; i < abc.metadata.size(); i++)
    {
        metadatas.emplace_back(convertMetadata(abc.metadata[i]));
    }

    for (size_t i = 0; i < abc.instances.size(); i++)
    {
        instances.emplace_back(convertInstance(abc.instances[i]));
    }

    for (size_t i = 0; i < abc.classes.size(); i++)
    {
        classes.emplace_back(convertClass(abc.classes[i], i));
    }

    for (size_t i = 0; i < abc.scripts.size(); i++)
    {
        asp.scripts.emplace_back(convertScript(abc.scripts[i]));
    }

    for (size_t i = 0; i < abc.bodies.size(); i++)
    {
        methods[abc.bodies[i].method]->vbody = convertBody(abc.bodies[i]);
    }

    for (size_t i = 0; i < classAdded.size(); i++)
    {
        if (!classAdded[i])
        {
            asp.orphanClasses.emplace_back(classes[i]);
        }
    }

    for (size_t i = 0; i < methodAdded.size(); i++)
    {
        if (!methodAdded[i])
        {
            asp.orphanMethods.emplace_back(methods[i]);
        }
    }

    return asp;
}

SWFABC::ABCFile ASASM::ASProgram::toABC()
{
    return AStoABC(*this).abc;
}
