#pragma once

#include "ABC/Error.hpp"
#include "ABC/Label.hpp"
#include "ASASM/Class.hpp"
#include "ASASM/Method.hpp"
#include "ASASM/Multiname.hpp"
#include "ASASM/Namespace.hpp"
#include "ASASM/Trait.hpp"
#include "enums/ABCType.hpp"
#include "enums/TraitAttribute.hpp"
#include <concepts>
#include <source_location>
#include <string>
#include <string_view>

#include <windows.h>

#include <FlashRuntimeExtensions.h>

#define DO_OR_FAIL_NULL(x)                                                                         \
    if (x != FRE_OK)                                                                               \
    FAIL_RETURN(nullptr)

#define FAIL_RESULT(message, result)                                                               \
    do                                                                                             \
    {                                                                                              \
        std::source_location current = std::source_location::current();                            \
        FAIL_RETURN(FREStringForError(                                                             \
            std::to_string(int32_t(result)) + ": " + message + "\nAt: " + current.file_name() +    \
            "(" + std::to_string(current.line()) + ") " + current.function_name()));               \
    }                                                                                              \
    while (false)

#define FAIL(message)                                                                              \
    do                                                                                             \
    {                                                                                              \
        FAIL_RETURN(FREStringForError(message));                                                   \
    }                                                                                              \
    while (false)

#define DO_OR_FAIL(message, x)                                                                     \
    do                                                                                             \
    {                                                                                              \
        if (FREResult res = x; res != FRE_OK)                                                      \
        {                                                                                          \
            FAIL_RESULT(message, res);                                                             \
        }                                                                                          \
    }                                                                                              \
    while (false)

#define CHECK_ARGC(num)                                                                            \
    do                                                                                             \
    {                                                                                              \
        if (argc != num)                                                                           \
        {                                                                                          \
            FAIL("argc should be " #num);                                                          \
        }                                                                                          \
    }                                                                                              \
    while (false)

#define FAIL_RETURN(x) throw x

inline FREObject FREStringForError(std::string_view v)
{
    FREObject ret;
    DO_OR_FAIL_NULL(FRENewObjectFromUTF8(v.size(), (const uint8_t*)v.data(), &ret));
    return ret;
}

inline FREObject FREString(std::string_view v)
{
    FREObject ret;
    DO_OR_FAIL(
        "Could not create string", FRENewObjectFromUTF8(v.size(), (const uint8_t*)v.data(), &ret));
    return ret;
}

template <FREObjectType Type, typename NumberType = double>
    requires std::same_as<NumberType, uint32_t> || std::same_as<NumberType, int32_t> ||
             std::same_as<NumberType, uint8_t> || std::same_as<NumberType, int8_t> ||
             std::same_as<NumberType, double>
inline auto CHECK_OBJECT(
    FREObject o, std::string name = "", std::source_location loc = std::source_location::current())
{
    if (name.empty())
    {
        name = "object";
    }
    FREObjectType type;
    DO_OR_FAIL("Failed getting object type of " + name, FREGetObjectType(o, &type));
    if (type != Type)
    {
        FAIL(name + " was not the proper type. Expected " + std::to_string(Type) + " but got " +
             std::to_string(type) + " during " + loc.function_name() + " at " +
             std::to_string(loc.line()));
    }
    if constexpr (Type == FRE_TYPE_NUMBER)
    {
        double ret;
        DO_OR_FAIL("Failed getting " + name + " as number during " + loc.function_name() + " at " +
                       std::to_string(loc.line()),
            FREGetObjectAsDouble(o, &ret));

        return NumberType(ret);
    }
    else if constexpr (Type == FRE_TYPE_BOOLEAN)
    {
        uint32_t ret;
        DO_OR_FAIL("Failed getting " + name + " as bool during " + loc.function_name() + " at " +
                       std::to_string(loc.line()),
            FREGetObjectAsBool(o, &ret));
        return bool(ret);
    }
    else if constexpr (Type == FRE_TYPE_STRING)
    {
        const uint8_t* data;
        uint32_t len;
        DO_OR_FAIL("Failed getting " + name + " as string during " + loc.function_name() + " at " +
                       std::to_string(loc.line()),
            FREGetObjectAsUTF8(o, &len, &data));

        return std::string_view((const char*)data, len);
    }
    else
    {
        return o;
    }
}

inline FREObject GetMember(FREObject o, std::string_view member,
    std::source_location loc = std::source_location::current())
{
    FREObject ret;
    DO_OR_FAIL("Could not get object property " + std::string(member) + " during " +
                   loc.function_name() + " at " + std::to_string(loc.line()),
        FREGetObjectProperty(o, (const uint8_t*)member.data(), &ret, nullptr));
    return ret;
}

template <FREObjectType Type, typename NumberType = double>
    requires std::same_as<NumberType, uint32_t> || std::same_as<NumberType, int32_t> ||
             std::same_as<NumberType, uint8_t> || std::same_as<NumberType, int8_t> ||
             std::same_as<NumberType, double>
inline auto CheckMember(FREObject o, std::string_view member,
    std::source_location loc = std::source_location::current())
{
    return CHECK_OBJECT<Type, NumberType>(GetMember(o, member), std::string(member), loc);
}

inline FREResult ANENewObject(
    const char* v, uint32_t argc, FREObject argv[], FREObject* object, FREObject* thrownException)
{
    return FRENewObject((const uint8_t*)v, argc, argv, object, thrownException);
}

inline FREResult ANESetObjectProperty(FREObject o, const char* m, FREObject v, FREObject* e)
{
    return FRESetObjectProperty(o, (const uint8_t*)m, v, e);
}

inline FREResult ANECallObjectMethod(
    FREObject o, const char* m, int32_t c, FREObject v[], FREObject* r, FREObject* e)
{
    return FRECallObjectMethod(o, (const uint8_t*)m, c, v, r, e);
}

#undef FAIL_RETURN

#define FAIL_RETURN(x) return x
