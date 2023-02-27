#pragma once

#include "utils/BidirectionalMap.hpp"
#include <array>
#include <stdint.h>
#include <string_view>

enum class ABCType : uint8_t
{
    Void               = 0x00, // not actually interned
    Undefined          = Void,
    Utf8               = 0x01,
    Decimal            = 0x02,
    Integer            = 0x03,
    UInteger           = 0x04,
    PrivateNamespace   = 0x05,
    Double             = 0x06,
    QName              = 0x07, // ns::name, const ns, const name
    Namespace          = 0x08,
    Multiname          = 0x09, //[ns...]::name, const [ns...], const name
    False              = 0x0A,
    True               = 0x0B,
    Null               = 0x0C,
    QNameA             = 0x0D, // @ns::name, const ns, const name
    MultinameA         = 0x0E, // @[ns...]::name, const [ns...], const name
    RTQName            = 0x0F, // ns::name, var ns, const name
    RTQNameA           = 0x10, // @ns::name, var ns, const name
    RTQNameL           = 0x11, // ns::[name], var ns, var name
    RTQNameLA          = 0x12, // @ns::[name], var ns, var name
    Namespace_Set      = 0x15, // a set of namespaces - used by multiname
    PackageNamespace   = 0x16, // a namespace that was derived from a package
    PackageInternalNs  = 0x17, // a namespace that had no uri
    ProtectedNamespace = 0x18,
    ExplicitNamespace  = 0x19,
    StaticProtectedNs  = 0x1A,
    MultinameL         = 0x1B,
    MultinameLA        = 0x1C,
    TypeName           = 0x1D,
    Float4             = 0x1E
};

inline constexpr BidirectionalMap ABCTypeMap = {"Void", ABCType::Void, "Utf8", ABCType::Utf8,
    "Decimal", ABCType::Decimal, "Integer", ABCType::Integer, "UInteger", ABCType::UInteger,
    "PrivateNamespace", ABCType::PrivateNamespace, "Double", ABCType::Double, "QName",
    ABCType::QName, "Namespace", ABCType::Namespace, "Multiname", ABCType::Multiname, "False",
    ABCType::False, "True", ABCType::True, "Null", ABCType::Null, "QNameA", ABCType::QNameA,
    "MultinameA", ABCType::MultinameA, "RTQName", ABCType::RTQName, "RTQNameA", ABCType::RTQNameA,
    "RTQNameL", ABCType::RTQNameL, "RTQNameLA", ABCType::RTQNameLA, "Namespace_Set",
    ABCType::Namespace_Set, "PackageNamespace", ABCType::PackageNamespace, "PackageInternalNs",
    ABCType::PackageInternalNs, "ProtectedNamespace", ABCType::ProtectedNamespace,
    "ExplicitNamespace", ABCType::ExplicitNamespace, "StaticProtectedNs",
    ABCType::StaticProtectedNs, "MultinameL", ABCType::MultinameL, "MultinameLA",
    ABCType::MultinameLA, "TypeName", ABCType::TypeName};
