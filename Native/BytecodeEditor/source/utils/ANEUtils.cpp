#include "utils/ANEUtils.hpp"
#include "ANEFunctions.hpp"
#include "enums/MethodFlags.hpp"

#undef FAIL_RETURN
#define FAIL_RETURN(x) throw x

std::shared_ptr<ASASM::Class> BytecodeEditor::ConvertClass(FREObject o) const
{
    FREObject dummy;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not get class's native pointer", exception,
        ANECallObjectMethod(o, "setNativePointerForConversion", 0, nullptr, &dummy, &exception));

    return std::get<std::shared_ptr<ASASM::Class>>(nextObjectContext->objectData->object);
}

FREObject BytecodeEditor::ConvertClass(const std::shared_ptr<ASASM::Class>& clazz) const
{
    nextObjectContext = ANEFunctionContext{const_cast<BytecodeEditor*>(this)->shared_from_this(),
        nullptr, ANEFunctionContext::ObjectData{clazz}};

    FREObject ret;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not build com.cff.anebe.ir.ASClass", exception,
        ANENewObject("com.cff.anebe.ir.ASClass", 0, nullptr, &ret, &exception));

    return ret;
}

ASASM::Namespace BytecodeEditor::ConvertNamespace(FREObject o) const
{
    ASASM::Namespace ret;
    if (auto found = ABCTypeMap.Find(CheckMember<FRE_TYPE_STRING>(o, "type")); found)
    {
        ret.kind = found->get();
    }

    if (ret.kind != ABCType::Namespace && ret.kind != ABCType::ExplicitNamespace &&
        ret.kind != ABCType::PackageNamespace && ret.kind != ABCType::PrivateNamespace &&
        ret.kind != ABCType::ProtectedNamespace && ret.kind != ABCType::PackageInternalNs &&
        ret.kind != ABCType::StaticProtectedNs)
    {
        FAIL("Invalid namespace type: " + std::string(CheckMember<FRE_TYPE_STRING>(o, "type")));
    }
    ret.name = std::string(CheckMember<FRE_TYPE_STRING>(o, "name"));

    FREObject disambiguatorObj = GetMember(o, "secondaryName");
    FREObjectType objType;
    DO_OR_FAIL("Could not get multiname type", FREGetObjectType(disambiguatorObj, &objType));

    auto homonyms = partialAssembly->namespaceResolver.getHomonyms(ret);

    if (objType == FRE_TYPE_NULL)
    {
        if (homonyms)
        {
            ret.id = homonyms->get().begin()->id;
        }
        return ret;
    }

    std::string_view disambiguator = CheckMember<FRE_TYPE_STRING>(o, "secondaryName");

    if (homonyms)
    {
        for (const auto& homonym : homonyms->get())
        {
            if (partialAssembly->namespaceResolver.namespaces[(uint8_t)homonym.kind].getName(
                    homonym.id) == disambiguator)
            {
                ret.id = homonym.id;
                break;
            }
        }
    }
    else
    {
        if (auto found = std::find(partialAssembly->extraNamespaceData.begin(),
                partialAssembly->extraNamespaceData.end(), disambiguator);
            found != partialAssembly->extraNamespaceData.end())
        {
            ret.id = std::distance(partialAssembly->extraNamespaceData.begin(), found) + 1;
        }
        else
        {
            partialAssembly->extraNamespaceData.emplace_back(disambiguator);
            ret.id = partialAssembly->extraNamespaceData.size();
        }
    }

    return ret;
}

FREObject BytecodeEditor::ConvertNamespace(const ASASM::Namespace& n) const
{
    FREObject name          = FREString(n.name);
    FREObject kind          = FREString(ABCTypeMap.ReverseFind(n.kind)->get());
    FREObject disambiguator = nullptr;
    if (partialAssembly->namespaceResolver.hasHomonyms(n))
    {
        disambiguator =
            FREString(partialAssembly->namespaceResolver.namespaces[(uint8_t)n.kind].getName(n.id));
    }

    FREObject args[] = {kind, name, disambiguator};

    FREObject ret;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not create com.cff.anebe.ir.ASNamespace", exception,
        ANENewObject("com.cff.anebe.ir.ASNamespace", 3, args, &ret, &exception));

    return ret;
}

ASASM::Multiname BytecodeEditor::ConvertMultiname(FREObject o) const
{
    ASASM::Multiname ret;

    FREObjectType type;
    DO_OR_FAIL("Could not get multiname type", FREGetObjectType(o, &type));

    if (o == nullptr || type == FRE_TYPE_NULL)
    {
        return ret;
    }

    if (auto found = ABCTypeMap.Find(CheckMember<FRE_TYPE_STRING>(o, "type")); found)
    {
        ret.kind = found->get();
    }
    else
    {
        FAIL("Invalid multiname type");
    }
    switch (ret.kind)
    {
        case ABCType::QName:
        case ABCType::QNameA:
            ret.qname({ConvertNamespace(CheckMember<FRE_TYPE_OBJECT>(o, "ns")),
                std::string(CheckMember<FRE_TYPE_STRING>(o, "name"))});
            break;
        case ABCType::RTQName:
        case ABCType::RTQNameA:
            ret.rtqname({std::string(CheckMember<FRE_TYPE_STRING>(o, "name"))});
            break;
        case ABCType::RTQNameL:
        case ABCType::RTQNameLA:
            ret.rtqnamel({});
            break;
        case ABCType::Multiname:
        case ABCType::MultinameA:
        {
            std::vector<ASASM::Namespace> nsEs;
            std::string name{CheckMember<FRE_TYPE_STRING>(o, "name")};
            FREObject vec = CheckMember<FRE_TYPE_VECTOR>(o, "nsSet");
            uint32_t size;
            DO_OR_FAIL("Could not get multiname vector's size", FREGetArrayLength(vec, &size));

            nsEs.reserve(size);
            for (size_t i = 0; i < size; i++)
            {
                FREObject elem;
                DO_OR_FAIL("Could not access a valid element of multiname vector",
                    FREGetArrayElementAt(vec, i, &elem));
                nsEs.emplace_back(ConvertNamespace(
                    CHECK_OBJECT<FRE_TYPE_OBJECT>(elem, "nsSet[" + std::to_string(i) + "]")));
            }

            ret.multiname({std::move(name), std::move(nsEs)});
        }
        break;
        case ABCType::MultinameL:
        case ABCType::MultinameLA:
        {
            std::vector<ASASM::Namespace> nsEs;
            FREObject vec = CheckMember<FRE_TYPE_VECTOR>(o, "nsSet");
            uint32_t size;
            DO_OR_FAIL("Could not get multiname vector's size", FREGetArrayLength(vec, &size));

            nsEs.reserve(size);
            for (size_t i = 0; i < size; i++)
            {
                FREObject elem;
                DO_OR_FAIL("Could not access a valid element of multiname vector",
                    FREGetArrayElementAt(vec, i, &elem));
                nsEs.emplace_back(ConvertNamespace(
                    CHECK_OBJECT<FRE_TYPE_OBJECT>(elem, "nsSet[" + std::to_string(i) + "]")));
            }

            ret.multinamel({std::move(nsEs)});
        }
        break;
        case ABCType::TypeName:
        {
            ASASM::Multiname tn = ConvertMultiname(CheckMember<FRE_TYPE_OBJECT>(o, "typename"));
            std::vector<ASASM::Multiname> params;
            FREObject vec = CheckMember<FRE_TYPE_VECTOR>(o, "params");
            uint32_t size;
            DO_OR_FAIL("Could not get multiname vector's size", FREGetArrayLength(vec, &size));

            params.reserve(size);
            for (size_t i = 0; i < size; i++)
            {
                FREObject elem;
                DO_OR_FAIL("Could not access a valid element of multiname vector",
                    FREGetArrayElementAt(vec, i, &elem));
                params.emplace_back(ConvertMultiname(
                    CHECK_OBJECT<FRE_TYPE_OBJECT>(elem, "params[" + std::to_string(i) + "]")));
            }

            ret.Typename({std::move(tn), std::move(params)});
        }
        break;
        // Unreachable
        default:
            FAIL("Invalid multiname kind: " + std::string(ABCTypeMap.ReverseFind(ret.kind)->get()));
    }

    return ret;
}

FREObject BytecodeEditor::ConvertMultiname(const ASASM::Multiname& m) const
{
    if (m.kind == ABCType::Void)
    {
        return nullptr;
    }

    FREObject ret;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not create com.cff.anebe.ir.ASMultiname", exception,
        ANENewObject("com.cff.anebe.ir.ASMultiname", 0, nullptr, &ret, &exception));

    DO_OR_FAIL_EXCEPTION("Could not set ASMultiname type", exception,
        ANESetObjectProperty(
            ret, "type", FREString(ABCTypeMap.ReverseFind(m.kind)->get()), &exception));

    switch (m.kind)
    {
        using enum ABCType;
        case QName:
        case QNameA:
            DO_OR_FAIL_EXCEPTION("Could not set ASMultiname name", exception,
                ANESetObjectProperty(ret, "name", FREString(m.qname().name), &exception));
            DO_OR_FAIL_EXCEPTION("Could not set ASMultiname namespace", exception,
                ANESetObjectProperty(ret, "ns", ConvertNamespace(m.qname().ns), &exception));
            break;
        case RTQName:
        case RTQNameA:
            DO_OR_FAIL_EXCEPTION("Could not set ASMultiname name", exception,
                ANESetObjectProperty(ret, "name", FREString(m.rtqname().name), &exception));
            break;
        case RTQNameL:
        case RTQNameLA:
            // No members
            break;
        case Multiname:
        case MultinameA:
        {
            DO_OR_FAIL_EXCEPTION("Could not set ASMultiname name", exception,
                ANESetObjectProperty(ret, "name", FREString(m.multiname().name), &exception));

            FREObject vec;
            DO_OR_FAIL_EXCEPTION("Could not create ASMultiname nsSet vector", exception,
                ANENewObject(
                    "Vector.<com.cff.anebe.ir.ASNamespace>", 0, nullptr, &vec, &exception));
            DO_OR_FAIL("Could not set ASMultiname nsSet vector size",
                FRESetArrayLength(vec, m.multiname().nsSet.size()));
            for (size_t i = 0; i < m.multiname().nsSet.size(); i++)
            {
                DO_OR_FAIL("Could not add ASNamespace to ASMultiname nsSet",
                    FRESetArrayElementAt(vec, i, ConvertNamespace(m.multiname().nsSet[i])));
            }
            DO_OR_FAIL_EXCEPTION("Could not set ASMultiname nsSet", exception,
                ANESetObjectProperty(ret, "nsSet", vec, &exception));
        }
        break;
        case MultinameL:
        case MultinameLA:
        {
            FREObject vec;
            DO_OR_FAIL_EXCEPTION("Could not create ASMultiname nsSet vector", exception,
                ANENewObject(
                    "Vector.<com.cff.anebe.ir.ASNamespace>", 0, nullptr, &vec, &exception));
            DO_OR_FAIL("Could not set ASMultiname nsSet vector size",
                FRESetArrayLength(vec, m.multinamel().nsSet.size()));
            for (size_t i = 0; i < m.multinamel().nsSet.size(); i++)
            {
                DO_OR_FAIL("Could not add ASNamespace to ASMultiname nsSet",
                    FRESetArrayElementAt(vec, i, ConvertNamespace(m.multinamel().nsSet[i])));
            }
            DO_OR_FAIL_EXCEPTION("Could not set ASMultiname nsSet", exception,
                ANESetObjectProperty(ret, "nsSet", vec, &exception));
        }
        break;
        case TypeName:
        {
            DO_OR_FAIL_EXCEPTION("Could not set ASMultiname typename", exception,
                ANESetObjectProperty(
                    ret, "typename", ConvertMultiname(m.Typename().name()), &exception));
            FREObject vec;
            DO_OR_FAIL_EXCEPTION("Could not create ASMultiname params vector", exception,
                ANENewObject(
                    "Vector.<com.cff.anebe.ir.ASMultiname>", 0, nullptr, &vec, &exception));
            DO_OR_FAIL("Could not set ASMultiname params vector size",
                FRESetArrayLength(vec, m.Typename().params().size()));
            for (size_t i = 0; i < m.Typename().params().size(); i++)
            {
                DO_OR_FAIL("Could not add ASNamespace to ASMultiname params",
                    FRESetArrayElementAt(vec, i, ConvertMultiname(m.Typename().params()[i])));
            }
            DO_OR_FAIL_EXCEPTION("Could not set ASMultiname params", exception,
                ANESetObjectProperty(ret, "params", vec, &exception));
        }
        break;
    }

    return ret;
}

FREObject BytecodeEditor::ConvertTrait(const ASASM::Trait& t) const
{
    FREObject kind      = FREString(TraitKindMap.ReverseFind(t.kind)->get());
    FREObject traitName = ConvertMultiname(t.name);
    FREObject attributes;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not create attributes array", exception,
        ANENewObject("Vector.<String>", 0, nullptr, &attributes, &exception));
    for (const auto& attribute : TraitAttributeMap.GetEntries())
    {
        if (t.attributes & uint8_t(attribute.second))
        {
            uint32_t len;
            DO_OR_FAIL(
                "Could not get attributes array length", FREGetArrayLength(attributes, &len));
            DO_OR_FAIL(
                "Could not set attributes array length", FRESetArrayLength(attributes, len + 1));
            DO_OR_FAIL("Could not add to attribute array",
                FRESetArrayElementAt(attributes, len, FREString(attribute.first)));
        }
    }
    FREObject metadatas;
    DO_OR_FAIL_EXCEPTION("Could not create metadatas array", exception,
        ANENewObject("Vector.<com.cff.anebe.ir.ASMetadata>", 0, nullptr, &metadatas, &exception));
    for (const auto& metadata : t.metadata)
    {
        FREObject name = FREString(metadata.name);
        FREObject metadataEntries;
        DO_OR_FAIL_EXCEPTION("Could not create metadata entry vector", exception,
            ANENewObject("Vector.<com.cff.anebe.ir.ASMetadataEntry>", 0, nullptr, &metadataEntries,
                &exception));
        DO_OR_FAIL("Could not set metadata entry vector size",
            FRESetArrayLength(metadataEntries, metadata.data.size()));
        for (size_t i = 0; i < metadata.data.size(); i++)
        {
            FREObject data;
            DO_OR_FAIL_EXCEPTION("Could not create metadata data object", exception,
                ANENewObject("com.cff.anebe.ir.ASMetadataEntry", 0, nullptr, &data, &exception));
            DO_OR_FAIL_EXCEPTION("Could not set metadata entry", exception,
                ANESetObjectProperty(data, "key", FREString(metadata.data[i].first), &exception));
            DO_OR_FAIL_EXCEPTION("Could not set metadata entry", exception,
                ANESetObjectProperty(
                    data, "value", FREString(metadata.data[i].second), &exception));
            DO_OR_FAIL(
                "Could not assign metadata entry", FRESetArrayElementAt(metadataEntries, i, data));
        }
        FREObject newmeta;
        FREObject args[2] = {name, metadataEntries};
        DO_OR_FAIL_EXCEPTION("Could not make metadata", exception,
            ANENewObject("com.cff.anebe.ir.ASMetadata", 2, args, &newmeta, &exception));

        uint32_t len;
        DO_OR_FAIL("Could not get metadatas array length", FREGetArrayLength(metadatas, &len));
        DO_OR_FAIL("Could not set metadatas array length", FRESetArrayLength(metadatas, len + 1));
        DO_OR_FAIL(
            "Could not add to metadata array", FRESetArrayElementAt(metadatas, len, newmeta));
    }
    FREObject slotId       = nullptr;
    FREObject type         = nullptr;
    FREObject value        = nullptr;
    FREObject funcOrMethod = nullptr;
    FREObject clazz        = nullptr;
    switch (t.kind)
    {
        case TraitKind::Class:
            DO_OR_FAIL("Could not create slot ID number",
                FRENewObjectFromUint32(t.vClass().slotId, &slotId));
            clazz = ConvertClass(t.vClass().vclass);
            break;
        case TraitKind::Method:
        case TraitKind::Getter:
        case TraitKind::Setter:
            DO_OR_FAIL("Could not create slot ID number",
                FRENewObjectFromUint32(t.vMethod().dispId, &slotId));
            funcOrMethod = ConvertMethod(*t.vMethod().vmethod);
            break;
        case TraitKind::Function:
            DO_OR_FAIL("Could not create slot ID number",
                FRENewObjectFromUint32(t.vFunction().slotId, &slotId));
            funcOrMethod = ConvertMethod(*t.vFunction().vfunction);
            break;
        case TraitKind::Slot:
        case TraitKind::Const:
            DO_OR_FAIL("Could not create slot ID number",
                FRENewObjectFromUint32(t.vSlot().slotId, &slotId));
            type  = ConvertMultiname(t.vSlot().typeName);
            value = ConvertValue(t.vSlot().value);
            break;
    }
    FREObject ret;
    FREObject args[] = {
        kind, attributes, slotId, traitName, type, metadatas, value, funcOrMethod, clazz};
    DO_OR_FAIL_EXCEPTION("Could not create com.cff.anebe.ir.ASTrait", exception,
        ANENewObject("com.cff.anebe.ir.ASTrait", 9, args, &ret, &exception));

    return ret;
}

ASASM::Trait BytecodeEditor::ConvertTrait(FREObject o) const
{
    TraitKind kind;
    if (auto found = TraitKindMap.Find(CheckMember<FRE_TYPE_STRING>(o, "kind")); found)
    {
        kind = found->get();
    }
    else
    {
        FAIL("Invalid trait kind");
    }

    FREObject attributesVec = CheckMember<FRE_TYPE_VECTOR>(o, "attributes");
    uint32_t vecSize;
    DO_OR_FAIL("Could not read attribute vector size", FREGetArrayLength(attributesVec, &vecSize));

    uint8_t attributes = 0;
    for (uint32_t i = 0; i < vecSize; i++)
    {
        FREObject currentAttr;
        DO_OR_FAIL("Could not read attribute vector element",
            FREGetArrayElementAt(attributesVec, i, &currentAttr));
        attributes |= uint8_t(TraitAttributeMap
                                  .Find(CHECK_OBJECT<FRE_TYPE_STRING>(
                                      currentAttr, "attributes[" + std::to_string(i) + "]"))
                                  ->get());
    }

    ASASM::Trait ret;
    ret.kind       = kind;
    ret.attributes = attributes;
    ret.name       = ConvertMultiname(CheckMember<FRE_TYPE_OBJECT>(o, "name"));

    FREObject metadatas = CheckMember<FRE_TYPE_VECTOR>(o, "metadata");
    DO_OR_FAIL("Could not read metadata vector size", FREGetArrayLength(metadatas, &vecSize));
    ret.metadata.reserve(vecSize);

    for (uint32_t i = 0; i < vecSize; i++)
    {
        FREObject currentMetadata;
        DO_OR_FAIL("Could not get current metadata element",
            FREGetArrayElementAt(metadatas, i, &currentMetadata));
        ASASM::Metadata md;
        md.name = CheckMember<FRE_TYPE_STRING>(currentMetadata, "name");

        FREObject currentMetadataVec = CheckMember<FRE_TYPE_VECTOR>(currentMetadata, "data");

        uint32_t metadataSize;
        DO_OR_FAIL(
            "Could not get current size", FREGetArrayLength(currentMetadataVec, &metadataSize));
        for (uint32_t j = 0; j < metadataSize; j++)
        {
            FREObject currentData;
            DO_OR_FAIL("Could not get current metadata element subvalue",
                FREGetArrayElementAt(currentMetadataVec, j, &currentData));
            md.data.emplace_back(std::string(CheckMember<FRE_TYPE_STRING>(currentData, "key")),
                std::string(CheckMember<FRE_TYPE_STRING>(currentData, "value")));
        }

        ret.metadata.emplace_back(std::move(md));
    }

    switch (kind)
    {
        case TraitKind::Slot:
        case TraitKind::Const:
        {
            ret.vSlot({CheckMember<FRE_TYPE_NUMBER, uint32_t>(o, "slotId"),
                ConvertMultiname(CheckMember<FRE_TYPE_OBJECT>(o, "typename")),
                ConvertValue(GetMember(o, "value"))});
        }
        break;
        case TraitKind::Method:
        case TraitKind::Getter:
        case TraitKind::Setter:
            ret.vMethod({CheckMember<FRE_TYPE_NUMBER, uint32_t>(o, "slotId"),
                ConvertMethod(CheckMember<FRE_TYPE_OBJECT>(o, "funcOrMethod"))});
            break;
        case TraitKind::Function:
            ret.vFunction({CheckMember<FRE_TYPE_NUMBER, uint32_t>(o, "slotId"),
                ConvertMethod(CheckMember<FRE_TYPE_OBJECT>(o, "funcOrMethod"))});
            break;
        case TraitKind::Class:
            ret.vClass({CheckMember<FRE_TYPE_NUMBER, uint32_t>(o, "slotId"),
                ConvertClass(CheckMember<FRE_TYPE_OBJECT>(o, "clazz"))});
            break;
    }

    return ret;
}

FREObject BytecodeEditor::ConvertMethod(const ASASM::Method& m) const
{
    FREObject paramTypes;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not create method paramTypes array", exception,
        ANENewObject("Vector.<com.cff.anebe.ir.ASMultiname>", 0, nullptr, &paramTypes, &exception));
    DO_OR_FAIL("Could not set paramTypes size", FRESetArrayLength(paramTypes, m.paramTypes.size()));
    for (size_t i = 0; i < m.paramTypes.size(); i++)
    {
        DO_OR_FAIL("Could not set paramTypes value",
            FRESetArrayElementAt(paramTypes, i, ConvertMultiname(m.paramTypes[i])));
    }

    FREObject returnType = ConvertMultiname(m.returnType);
    FREObject name       = FREString(m.name);
    FREObject flags;
    DO_OR_FAIL_EXCEPTION("Could not create method flags array", exception,
        ANENewObject("Vector.<String>", 0, nullptr, &flags, &exception));
    for (const auto& flag : MethodFlagMap.GetEntries())
    {
        if (m.flags & uint8_t(flag.second))
        {
            uint32_t len;
            DO_OR_FAIL("Could not get attributes array length", FREGetArrayLength(flags, &len));
            DO_OR_FAIL("Could not set attributes array length", FRESetArrayLength(flags, len + 1));
            DO_OR_FAIL("Could not add to attribute array",
                FRESetArrayElementAt(flags, len, FREString(flag.first)));
        }
    }

    FREObject options;
    DO_OR_FAIL_EXCEPTION("Could not create options array", exception,
        ANENewObject("Vector.<com.cff.anebe.ir.ASValue>", 0, nullptr, &options, &exception));
    DO_OR_FAIL("Could not set options size", FRESetArrayLength(options, m.options.size()));
    for (size_t i = 0; i < m.options.size(); i++)
    {
        FRESetArrayElementAt(options, i, ConvertValue(m.options[i]));
    }

    FREObject paramNames;
    DO_OR_FAIL_EXCEPTION("Could not create paramNames array", exception,
        ANENewObject("Vector.<String>", 0, nullptr, &paramNames, &exception));
    DO_OR_FAIL("Could not set paramNames size", FRESetArrayLength(paramNames, m.paramNames.size()));
    for (size_t i = 0; i < m.paramNames.size(); i++)
    {
        DO_OR_FAIL("Could not create paramName value",
            FRESetArrayElementAt(paramNames, i, FREString(m.paramNames[i])));
    }

    FREObject body = m.vbody ? ConvertMethodBody(*m.vbody) : nullptr;

    FREObject args[] = {paramTypes, returnType, name, flags, options, paramNames, body};

    FREObject ret;
    DO_OR_FAIL_EXCEPTION("Could not create com.cff.anebe.ir.ASMethod", exception,
        ANENewObject("com.cff.anebe.ir.ASMethod", 7, args, &ret, &exception));
    return ret;
}

std::shared_ptr<ASASM::Method> BytecodeEditor::ConvertMethod(FREObject o) const
{
    std::shared_ptr<ASASM::Method> ret = std::make_shared<ASASM::Method>();

    FREObject array = CheckMember<FRE_TYPE_VECTOR>(o, "paramTypes");
    uint32_t arrLen;
    DO_OR_FAIL("Could not get param type size", FREGetArrayLength(array, &arrLen));
    ret->paramTypes.reserve(arrLen);
    for (size_t i = 0; i < arrLen; i++)
    {
        FREObject ptype;
        DO_OR_FAIL("Could not get param type entry", FREGetArrayElementAt(array, i, &ptype));
        ret->paramTypes.emplace_back(std::move(ConvertMultiname(
            CHECK_OBJECT<FRE_TYPE_OBJECT>(ptype, "paramTypes[" + std::to_string(i) + "]"))));
    }

    ret->returnType = ConvertMultiname(GetMember(o, "returnType"));

    ret->name = CheckMember<FRE_TYPE_STRING>(o, "name");

    array = CheckMember<FRE_TYPE_VECTOR>(o, "flags");

    DO_OR_FAIL("Could not get flag size", FREGetArrayLength(array, &arrLen));
    for (size_t i = 0; i < arrLen; i++)
    {
        FREObject flag;
        DO_OR_FAIL("Could not get flag entry", FREGetArrayElementAt(array, i, &flag));
        if (auto found = MethodFlagMap.Find(
                CHECK_OBJECT<FRE_TYPE_STRING>(flag, "flags[" + std::to_string(i) + "]"));
            found)
        {
            ret->flags |= uint8_t(found.value().get());
        }
        else
        {
            FAIL("Invalid method flag" + std::string(CHECK_OBJECT<FRE_TYPE_STRING>(flag)));
        }
    }

    array = CheckMember<FRE_TYPE_VECTOR>(o, "options");

    DO_OR_FAIL("Could not get options size", FREGetArrayLength(array, &arrLen));
    ret->options.reserve(arrLen);
    for (size_t i = 0; i < arrLen; i++)
    {
        FREObject option;
        DO_OR_FAIL("Could not get option value", FREGetArrayElementAt(array, i, &option));
        ret->options.emplace_back(ConvertValue(
            CHECK_OBJECT<FRE_TYPE_OBJECT>(option, "options[" + std::to_string(i) + "]")));
    }

    array = CheckMember<FRE_TYPE_VECTOR>(o, "paramNames");

    DO_OR_FAIL("Could not get param names size", FREGetArrayLength(array, &arrLen));
    ret->paramNames.reserve(arrLen);
    for (size_t i = 0; i < arrLen; i++)
    {
        FREObject pname;
        DO_OR_FAIL("Could not get param name value", FREGetArrayElementAt(array, i, &pname));
        ret->paramNames.emplace_back(std::string(
            CHECK_OBJECT<FRE_TYPE_STRING>(pname, "paramNames[" + std::to_string(i) + "]")));
    }

    FREObjectType type;
    DO_OR_FAIL("Could not get body type", FREGetObjectType(GetMember(o, "body"), &type));

    if (type != FRE_TYPE_NULL)
    {
        ret->vbody         = ConvertMethodBody(GetMember(o, "body"));
        ret->vbody->method = ret;
    }

    return ret;
}

FREObject BytecodeEditor::ConvertMethodBody(const ASASM::MethodBody& b) const
{
    FREObject maxStack;
    DO_OR_FAIL("Could not create body max stack", FRENewObjectFromUint32(b.maxStack, &maxStack));
    FREObject localCount;
    DO_OR_FAIL(
        "Could not create body local count", FRENewObjectFromUint32(b.localCount, &localCount));
    FREObject initScopeDepth;
    DO_OR_FAIL("Could not create body init scope depth",
        FRENewObjectFromUint32(b.initScopeDepth, &initScopeDepth));
    FREObject maxScopeDepth;
    DO_OR_FAIL("Could not create body max scope depth",
        FRENewObjectFromUint32(b.maxScopeDepth, &maxScopeDepth));

    FREObject instructions;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not create body instruction vector", exception,
        ANENewObject(
            "Vector.<com.cff.anebe.ir.ASInstruction>", 0, nullptr, &instructions, &exception));
    DO_OR_FAIL(
        "Could not set instruction size", FRESetArrayLength(instructions, b.instructions.size()));

    std::vector<FREObject> FREInstructions;
    FREInstructions.reserve(b.instructions.size());
    std::vector<std::size_t> fixups;
    for (size_t i = 0; i < b.instructions.size(); i++)
    {
        const auto [instr, requiresFixup] = ConvertInstruction(b.instructions[i]);
        FREInstructions.emplace_back(instr);
        if (requiresFixup)
        {
            fixups.emplace_back(i);
        }
        DO_OR_FAIL(
            "Could not set instruction to vector", FRESetArrayElementAt(instructions, i, instr));
    }

    for (const auto& fixup : fixups)
    {
        const auto& instr    = b.instructions[fixup];
        const auto& argTypes = OPCode_Info[uint8_t(instr.opcode)].second;

        FREObject FREInstr     = FREInstructions[fixup];
        FREObject FREInstrArgs = CheckMember<FRE_TYPE_ARRAY>(FREInstr, "args");

        for (size_t i = 0; i < argTypes.size(); i++)
        {
            switch (argTypes[i])
            {
                using enum OPCodeArgumentType;
                default:
                    break; // Do nothing for non-label arguments
                case JumpTarget:
                case SwitchDefaultTarget:
                {
                    const auto& target = instr.arguments[i].jumpTarget();
                    if (target.index >= FREInstructions.size())
                    {
                        FAIL("Target too large: " + std::to_string(target.index));
                    }
                    if (FREInstructions[target.index] == nullptr)
                    {
                        FAIL("Instruction " + std::to_string(target.index) + " was null?????");
                    }
                    DO_OR_FAIL("Could not set instruction label",
                        FRESetArrayElementAt(FREInstrArgs, i, FREInstructions[target.index]));
                }
                break;
                case SwitchTargets:
                {
                    const auto& targets = instr.arguments[i].switchTargets();
                    FREObject FRETargets;
                    DO_OR_FAIL_EXCEPTION("Couldn't create switch target array", exception,
                        ANENewObject("Vector.<com.cff.anebe.ir.ASInstruction>", 0, nullptr,
                            &FRETargets, &exception));
                    DO_OR_FAIL("Couldn't set switch target array size",
                        FRESetArrayLength(FRETargets, targets.size()));

                    for (size_t j = 0; j < targets.size(); j++)
                    {
                        DO_OR_FAIL(
                            "Could not set instruction label vector element " + std::to_string(j),
                            FRESetArrayElementAt(FRETargets, j, FREInstructions[targets[j].index]));
                    }

                    DO_OR_FAIL("Could not set instruction labels",
                        FRESetArrayElementAt(FREInstrArgs, i, FRETargets));
                }
                break;
            }
        }
    }

    FREObject exceptions;
    DO_OR_FAIL_EXCEPTION("Could not create body exception vector", exception,
        ANENewObject("Vector.<com.cff.anebe.ir.ASException>", 0, nullptr, &exceptions, &exception));
    DO_OR_FAIL("Could not set exception size", FRESetArrayLength(exceptions, b.exceptions.size()));
    for (size_t i = 0; i < b.exceptions.size(); i++)
    {
        DO_OR_FAIL("Could not set exception to vector",
            FRESetArrayElementAt(
                exceptions, i, ConvertException(b.exceptions[i], FREInstructions)));
    }

    FREObject traits;
    DO_OR_FAIL_EXCEPTION("Could not create body trait vector", exception,
        ANENewObject("Vector.<com.cff.anebe.ir.ASTrait>", 0, nullptr, &traits, &exception));
    DO_OR_FAIL("Could not set trait size", FRESetArrayLength(traits, b.traits.size()));
    for (size_t i = 0; i < b.traits.size(); i++)
    {
        DO_OR_FAIL("Could not set trait to vector",
            FRESetArrayElementAt(traits, i, ConvertTrait(b.traits[i])));
    }

    FREObject errors;
    DO_OR_FAIL_EXCEPTION("Could not create body error vector", exception,
        ANENewObject("Vector.<com.cff.anebe.ir.ASError>", 0, nullptr, &errors, &exception));
    DO_OR_FAIL("Could not set error size", FRESetArrayLength(errors, b.errors.size()));
    for (size_t i = 0; i < b.errors.size(); i++)
    {
        DO_OR_FAIL("Could not set error to vector",
            FRESetArrayElementAt(errors, i, ConvertError(b.errors[i], FREInstructions)));
    }

    FREObject args[] = {maxStack, localCount, initScopeDepth, maxScopeDepth, instructions,
        exceptions, traits, errors};

    FREObject ret;
    DO_OR_FAIL_EXCEPTION("Could not create com.cff.anebe.ir.ASMethodBody", exception,
        ANENewObject("com.cff.anebe.ir.ASMethodBody", 8, args, &ret, &exception));
    return ret;
}

ASASM::MethodBody BytecodeEditor::ConvertMethodBody(FREObject o) const
{
    ASASM::MethodBody body;
    body.maxStack       = CheckMember<FRE_TYPE_NUMBER, uint32_t>(o, "maxStack");
    body.localCount     = CheckMember<FRE_TYPE_NUMBER, uint32_t>(o, "localCount");
    body.initScopeDepth = CheckMember<FRE_TYPE_NUMBER, uint32_t>(o, "initScopeDepth");
    body.maxScopeDepth  = CheckMember<FRE_TYPE_NUMBER, uint32_t>(o, "maxScopeDepth");

    FREObject array = CheckMember<FRE_TYPE_VECTOR>(o, "instructions");
    uint32_t arrLen;
    DO_OR_FAIL("Couldn't get instruction vector size", FREGetArrayLength(array, &arrLen));
    std::vector<FREObject> FREInstructions;
    FREInstructions.reserve(arrLen);
    for (uint32_t i = 0; i < arrLen; i++)
    {
        FREObject instruction;
        DO_OR_FAIL("Couldn't get instruction vector element",
            FREGetArrayElementAt(array, i, &instruction));
        FREInstructions.emplace_back(instruction);
    }

    body.instructions.reserve(arrLen);
    for (auto instruction : FREInstructions)
    {
        body.instructions.emplace_back(ConvertInstruction(instruction, FREInstructions));
    }

    array = CheckMember<FRE_TYPE_VECTOR>(o, "exceptions");
    DO_OR_FAIL("Couldn't get exception vector size", FREGetArrayLength(array, &arrLen));
    body.exceptions.reserve(arrLen);
    for (uint32_t i = 0; i < arrLen; i++)
    {
        FREObject exception;
        DO_OR_FAIL(
            "Couldn't get exception vector element", FREGetArrayElementAt(array, i, &exception));
        body.exceptions.emplace_back(ConvertException(exception, FREInstructions));
    }

    array = CheckMember<FRE_TYPE_VECTOR>(o, "traits");
    DO_OR_FAIL("Couldn't get trait vector size", FREGetArrayLength(array, &arrLen));
    body.traits.reserve(arrLen);
    for (uint32_t i = 0; i < arrLen; i++)
    {
        FREObject trait;
        DO_OR_FAIL("Couldn't get trait vector element", FREGetArrayElementAt(array, i, &trait));
        body.traits.emplace_back(ConvertTrait(trait));
    }

    array = CheckMember<FRE_TYPE_VECTOR>(o, "errors");
    DO_OR_FAIL("Couldn't get error vector size", FREGetArrayLength(array, &arrLen));
    body.errors.reserve(arrLen);
    for (uint32_t i = 0; i < arrLen; i++)
    {
        FREObject error;
        DO_OR_FAIL("Couldn't get error vector element", FREGetArrayElementAt(array, i, &error));
        body.errors.emplace_back(ConvertError(error, FREInstructions));
    }

    return body;
}

FREObject BytecodeEditor::ConvertException(
    const ASASM::Exception& e, const std::vector<FREObject>& allInstrs) const
{
    FREObject from          = ConvertLabel(e.from, allInstrs);
    FREObject to            = ConvertLabel(e.to, allInstrs);
    FREObject target        = ConvertLabel(e.target, allInstrs);
    FREObject exceptionType = ConvertMultiname(e.excType);
    FREObject exceptionName = ConvertMultiname(e.varName);

    FREObject args[] = {from, to, target, exceptionType, exceptionName};

    FREObject ret;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not create com.cff.anebe.ir.ASException", exception,
        ANENewObject("com.cff.anebe.ir.ASException", 5, args, &ret, &exception));
    return ret;
}

ASASM::Exception BytecodeEditor::ConvertException(
    FREObject o, const std::vector<FREObject>& allInstrs) const
{
    return {ConvertLabel(CheckMember<FRE_TYPE_OBJECT>(o, "from"), allInstrs),
        ConvertLabel(CheckMember<FRE_TYPE_OBJECT>(o, "to"), allInstrs),
        ConvertLabel(CheckMember<FRE_TYPE_OBJECT>(o, "target"), allInstrs),
        ConvertMultiname(GetMember(o, "exceptionType")),
        ConvertMultiname(GetMember(o, "exceptionName"))};
}

FREObject BytecodeEditor::ConvertError(
    const SWFABC::Error& e, const std::vector<FREObject>& allInstrs) const
{
    FREObject args[] = {ConvertLabel(e.loc, allInstrs), FREString(e.message)};

    FREObject ret;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Couldn't make com.cff.anebe.ir.ASError", exception,
        ANENewObject("com.cff.anebe.ir.ASError", 2, args, &ret, &exception));
    return ret;
}

SWFABC::Error BytecodeEditor::ConvertError(
    FREObject o, const std::vector<FREObject>& allInstrs) const
{
    return {ConvertLabel(CheckMember<FRE_TYPE_OBJECT>(o, "loc"), allInstrs),
        std::string(CheckMember<FRE_TYPE_STRING>(o, "message"))};
}

FREObject BytecodeEditor::ConvertLabel(
    const SWFABC::Label& l, const std::vector<FREObject>& allInstrs) const
{
    if (l.index + l.offset >= allInstrs.size())
    {
        FAIL("Invalid label");
    }
    return allInstrs[l.index + l.offset];
}

SWFABC::Label BytecodeEditor::ConvertLabel(
    FREObject o, const std::vector<FREObject>& allInstrs) const
{
    if (auto found = std::ranges::find(allInstrs, o); found != allInstrs.end())
    {
        return SWFABC::Label{.index = (uint32_t)std::distance(allInstrs.begin(), found)};
    }

    FAIL("Could not find target instruction in full instruction list");
}

ASASM::Instruction BytecodeEditor::ConvertInstruction(
    FREObject o, const std::vector<FREObject>& allInstrs) const
{
    ASASM::Instruction ret;
    if (auto found = OPCodeMap.Find(CheckMember<FRE_TYPE_STRING>(o, "opcode")); found)
    {
        ret.opcode = found->get();
    }
    else
    {
        FAIL("Invalid OPCode for instruction: " +
             std::string(CheckMember<FRE_TYPE_STRING>(o, "opcode")));
    }

    const auto& argTypes = OPCode_Info[uint8_t(ret.opcode)].second;
    uint32_t argsSize    = 0;

    FREObject args = GetMember(o, "args");
    FREObjectType argsType;
    DO_OR_FAIL("Couldn't check instruction args type", FREGetObjectType(args, &argsType));
    if (argsType == FRE_TYPE_NULL)
    {
        if (argTypes.size() == 0)
        {
            return ret;
        }
        else
        {
            FAIL("Null args given for opcode " +
                 std::string(OPCodeMap.ReverseFind(ret.opcode)->get()));
        }
    }

    args = CHECK_OBJECT<FRE_TYPE_ARRAY>(args, "args");

    DO_OR_FAIL("Couldn't check instruction args size", FREGetArrayLength(args, &argsSize));

    if (argsSize != argTypes.size())
    {
        std::string error = "Args for instruction ";
        error             += OPCodeMap.ReverseFind(ret.opcode)->get();
        error             += " cannot be ";
        error             += std::to_string(argsSize);
        error             += ". Argument types should be ";
        for (const auto& argType : argTypes)
        {
            switch (argType)
            {
                using enum OPCodeArgumentType;
                default:
                case Unknown:
                    error += "Unknown ";
                    break;
                case ByteLiteral:
                    error += "ByteLiteral ";
                    break;
                case UByteLiteral:
                    error += "UByteLiteral ";
                    break;
                case IntLiteral:
                    error += "IntLiteral ";
                    break;
                case UIntLiteral:
                    error += "UIntLiteral ";
                    break;
                case Int:
                    error += "Int ";
                    break;
                case UInt:
                    error += "UInt ";
                    break;
                case Double:
                    error += "Double ";
                    break;
                case String:
                    error += "String ";
                    break;
                case Namespace:
                    error += "Namespace ";
                    break;
                case Multiname:
                    error += "Multiname ";
                    break;
                case Class:
                    error += "Class ";
                    break;
                case Method:
                    error += "Method ";
                    break;
                case JumpTarget:
                    error += "JumpTarget ";
                    break;
                case SwitchDefaultTarget:
                    error += "SwitchDefaultTarget ";
                    break;
                case SwitchTargets:
                    error += "SwitchTargets ";
                    break;
            }
        }
        FAIL(error);
    }

    ret.arguments.reserve(argsSize);

    for (size_t i = 0; i < argTypes.size(); i++)
    {
        FREObject argO;
        ASASM::Instruction::Argument arg;
        DO_OR_FAIL("Couldn't get instruction argument", FREGetArrayElementAt(args, i, &argO));
        switch (argTypes[i])
        {
            using enum OPCodeArgumentType;
            default:
            case Unknown:
                FAIL(std::string("Couldn't handle OPCode ") +
                     OPCodeMap.ReverseFind(ret.opcode)->get());
                break;
            case ByteLiteral:
                arg.bytev(
                    CHECK_OBJECT<FRE_TYPE_NUMBER, int8_t>(argO, "args[" + std::to_string(i) + "]"));
                break;
            case IntLiteral:
            case Int:
                arg.intv(CHECK_OBJECT<FRE_TYPE_NUMBER, int32_t>(
                    argO, "args[" + std::to_string(i) + "]"));
                break;
            case UByteLiteral:
                arg.ubytev(CHECK_OBJECT<FRE_TYPE_NUMBER, uint8_t>(
                    argO, "args[" + std::to_string(i) + "]"));
                break;
            case UIntLiteral:
            case UInt:
                arg.uintv(CHECK_OBJECT<FRE_TYPE_NUMBER, uint32_t>(
                    argO, "args[" + std::to_string(i) + "]"));
                break;

            case Double:
                arg.doublev(
                    CHECK_OBJECT<FRE_TYPE_NUMBER, double>(argO, "args[" + std::to_string(i) + "]"));
                break;

            case String:
                arg.stringv(std::string(
                    CHECK_OBJECT<FRE_TYPE_STRING>(argO, "args[" + std::to_string(i) + "]")));
                break;

            case Namespace:
                arg.namespacev(ConvertNamespace(
                    CHECK_OBJECT<FRE_TYPE_OBJECT>(argO, "args[" + std::to_string(i) + "]")));
                break;

            case Multiname:
                arg.multinamev(ConvertMultiname(
                    CHECK_OBJECT<FRE_TYPE_OBJECT>(argO, "args[" + std::to_string(i) + "]")));
                break;

            case Class:
                arg.classv(ConvertClass(
                    CHECK_OBJECT<FRE_TYPE_OBJECT>(argO, "args[" + std::to_string(i) + "]")));
                break;

            case Method:
                arg.methodv(ConvertMethod(
                    CHECK_OBJECT<FRE_TYPE_OBJECT>(argO, "args[" + std::to_string(i) + "]")));
                break;

            case JumpTarget:
            case SwitchDefaultTarget:
                arg.jumpTarget(ConvertLabel(
                    CHECK_OBJECT<FRE_TYPE_OBJECT>(argO, "args[" + std::to_string(i) + "]"),
                    allInstrs));
                break;

            case SwitchTargets:
            {
                uint32_t targetLen;
                DO_OR_FAIL("Couldn't get switch targets length",
                    FREGetArrayLength(
                        CHECK_OBJECT<FRE_TYPE_VECTOR>(argO, "args[" + std::to_string(i) + "]"),
                        &targetLen));

                arg.switchTargets(std::vector<SWFABC::Label>(targetLen));
                for (size_t j = 0; j < targetLen; j++)
                {
                    FREObject target;
                    DO_OR_FAIL("Couldn't get switch targets entry",
                        FREGetArrayElementAt(argO, j, &target));
                    arg.switchTargets()[j] = ConvertLabel(target, allInstrs);
                }
            }
            break;
        }

        ret.arguments.emplace_back(std::move(arg));
    }

    return ret;
}

std::pair<FREObject, bool> BytecodeEditor::ConvertInstruction(const ASASM::Instruction& instr) const
{
    bool requiresFixup = false;

    FREObject opcode = FREString(OPCodeMap.ReverseFind(instr.opcode)->get());
    FREObject arguments;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not create instruction arguments array", exception,
        ANENewObject("Array", 0, nullptr, &arguments, &exception));

    const auto& argTypes = OPCode_Info[uint8_t(instr.opcode)].second;

    for (size_t i = 0; i < argTypes.size(); i++)
    {
        FREObject arg;
        switch (argTypes[i])
        {
            using enum OPCodeArgumentType;
            case Unknown:
            default:
                FAIL("Could not encode unknown format OPCode " +
                     std::string(OPCodeMap.ReverseFind(instr.opcode)->get()));
                break;
            case ByteLiteral:
            {
                DO_OR_FAIL("Could not create byte literal",
                    FRENewObjectFromInt32(instr.arguments[i].bytev(), &arg));
            }
            break;
            case UByteLiteral:
            {
                DO_OR_FAIL("Could not create ubyte literal",
                    FRENewObjectFromUint32(instr.arguments[i].ubytev(), &arg));
            }
            break;

            case IntLiteral:
            case Int:
            {
                DO_OR_FAIL("Could not create int argument",
                    FRENewObjectFromInt32((int32_t)instr.arguments[i].intv(), &arg));
            }
            break;
            case UIntLiteral:
            case UInt:
            {
                DO_OR_FAIL("Could not create uint argument",
                    FRENewObjectFromUint32((uint32_t)instr.arguments[i].uintv(), &arg));
            }
            break;

            case Double:
            {
                DO_OR_FAIL("Could not create number argument",
                    FRENewObjectFromDouble(instr.arguments[i].doublev(), &arg));
            }
            break;

            case String:
            {
                arg = FREString(instr.arguments[i].stringv());
            }
            break;

            case Namespace:
            {
                arg = ConvertNamespace(instr.arguments[i].namespacev());
            }
            break;

            case Multiname:
            {
                arg = ConvertMultiname(instr.arguments[i].multinamev());
            }
            break;

            case Class:
            {
                arg = ConvertClass(instr.arguments[i].classv());
            }
            break;

            case Method:
            {
                arg = ConvertMethod(*instr.arguments[i].methodv());
            }
            break;

            case JumpTarget:
            case SwitchDefaultTarget:
            {
                requiresFixup = true;
                arg           = nullptr; // = ConvertLabel(instr.arguments[i].jumpTarget());
            }
            break;

            case SwitchTargets:
            {
                requiresFixup = true;
                arg           = nullptr;

                // DO_OR_FAIL("Couldn't create switch target array",
                //     ANENewObject("Vector.<com.cff.anebe.ir.ASLabel>", 0, nullptr, &arg,
                //     nullptr));
                // DO_OR_FAIL("Couldn't set switch target array size",
                //     FRESetArrayLength(arg, instr.arguments[i].switchTargets().size()));

                // for (size_t j = 0; j < instr.arguments[i].switchTargets().size(); j++)
                // {
                //     DO_OR_FAIL("Couldn't set switch target array element",
                //         FRESetArrayElementAt(
                //             arg, i, ConvertLabel(instr.arguments[i].switchTargets()[j])));
                // }
            }
            break;
        }

        DO_OR_FAIL("Couldn't set argument array element", FRESetArrayElementAt(arguments, i, arg));
    }

    FREObject args[] = {opcode, arguments};

    FREObject ret;
    DO_OR_FAIL_EXCEPTION("Couldn't make com.cff.anebe.ir.ASInstruction", exception,
        ANENewObject("com.cff.anebe.ir.ASInstruction", 2, args, &ret, &exception));
    return {ret, requiresFixup};
}

FREObject BytecodeEditor::ConvertValue(const ASASM::Value& v) const
{
    FREObject value;
    switch (v.vkind)
    {
        using enum ABCType;
        case Void:
        default:
            return nullptr;
        case Null:
            value = nullptr;
            break;
        case Integer:
            DO_OR_FAIL(
                "Couldn't convert integer value", FRENewObjectFromInt32((int32_t)v.vint(), &value));
            break;
        case UInteger:
            DO_OR_FAIL("Couldn't convert uinteger value",
                FRENewObjectFromUint32((uint32_t)v.vuint(), &value));
            break;
        case Double:
            DO_OR_FAIL(
                "Couldn't convert double value", FRENewObjectFromDouble(v.vdouble(), &value));
            break;
        case Utf8:
            value = FREString(v.vstring());
            break;
        case True:
            DO_OR_FAIL("Couldn't convert true value", FRENewObjectFromBool(true, &value));
            break;
        case False:
            DO_OR_FAIL("Couldn't convert false value", FRENewObjectFromBool(false, &value));
            break;
        case Namespace:
        case PackageNamespace:
        case PackageInternalNs:
        case ProtectedNamespace:
        case ExplicitNamespace:
        case StaticProtectedNs:
        case PrivateNamespace:
            value = ConvertNamespace(v.vnamespace());
            break;
    }

    FREObject ret;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Couldn't create com.cff.anebe.ir.ASValue", exception,
        ANENewObject("com.cff.anebe.ir.ASValue", 1, &value, &ret, &exception));
    return ret;
}

ASASM::Value BytecodeEditor::ConvertValue(FREObject o) const
{
    ASASM::Value ret;

    FREObjectType type;
    DO_OR_FAIL("Could not get value type", FREGetObjectType(o, &type));

    if (o == nullptr || type == FRE_TYPE_NULL)
    {
        return ret;
    }

    std::string_view valType = CheckMember<FRE_TYPE_STRING>(o, "type");

    if (valType == "int")
    {
        ret.vkind = ABCType::Integer;
        ret.vint(CheckMember<FRE_TYPE_NUMBER, int32_t>(o, "value"));
    }
    else if (valType == "uint")
    {
        ret.vkind = ABCType::UInteger;
        ret.vuint(CheckMember<FRE_TYPE_NUMBER, uint32_t>(o, "value"));
    }
    else if (valType == "Number")
    {
        ret.vkind = ABCType::Double;
        ret.vdouble(CheckMember<FRE_TYPE_NUMBER, double>(o, "value"));
    }
    else if (valType == "String")
    {
        ret.vkind = ABCType::Utf8;
        ret.vstring(std::string(CheckMember<FRE_TYPE_STRING>(o, "value")));
    }
    else if (valType == "Namespace")
    {
        ret.vnamespace(ConvertNamespace(CheckMember<FRE_TYPE_OBJECT>(o, "value")));
        ret.vkind = ret.vnamespace().kind;
    }
    else if (valType == "Null")
    {
        ret.vkind = ABCType::Null;
    }
    else if (valType == "True")
    {
        ret.vkind = ABCType::True;
    }
    else if (valType == "False")
    {
        ret.vkind = ABCType::False;
    }

    return ret;
}
