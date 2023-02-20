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

#if defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L
#define ANEBYTECODEEDITOR_HAS_STACKTRACE
#endif

#define DO_OR_FAIL_NULL(x)                                                                         \
    if (x != FRE_OK)                                                                               \
    FAIL_RETURN(nullptr)

#ifdef ANEBYTECODEEDITOR_HAS_STACKTRACE
#include <stacktrace>
#define FAIL_RESULT(message, result)                                                               \
    do                                                                                             \
    {                                                                                              \
        std::source_location fail_result_current = std::source_location::current();                \
        std::stacktrace fail_result_stack_trace  = std::stacktrace::current(1);                    \
        FAIL_RETURN(FREStringForError(std::to_string(int32_t(result)) + ": " + message +           \
                                      "\nAt: " + fail_result_current.file_name() + "(" +           \
                                      std::to_string(fail_result_current.line()) + ") " +          \
                                      fail_result_current.function_name() + "Stack trace: \n" +    \
                                      std::to_string(fail_result_stack_trace)));                   \
    }                                                                                              \
    while (false)

#define FAIL(message)                                                                              \
    do                                                                                             \
    {                                                                                              \
        std::stacktrace fail_result_stack_trace = std::stacktrace::current(1);                     \
        FAIL_RETURN(FREStringForError(                                                             \
            message + ("\nStack trace: " + std::to_string(fail_result_stack_trace))));             \
    }                                                                                              \
    while (false)
#else
#define FAIL_RESULT(message, result)                                                               \
    do                                                                                             \
    {                                                                                              \
        std::source_location fail_result_current = std::source_location::current();                \
        FAIL_RETURN(FREStringForError(std::to_string(int32_t(result)) + ": " + message +           \
                                      "\nAt: " + fail_result_current.file_name() + "(" +           \
                                      std::to_string(fail_result_current.line()) + ") " +          \
                                      fail_result_current.function_name()));                       \
    }                                                                                              \
    while (false)

#define FAIL(message)                                                                              \
    do                                                                                             \
    {                                                                                              \
        std::source_location fail_result_current = std::source_location::current();                \
        FAIL_RETURN(FREStringForError(                                                             \
            message + ("\nAt: " + (fail_result_current.file_name() +                               \
                                      ("(" + std::to_string(fail_result_current.line()) + ") ") +  \
                                      fail_result_current.function_name()))));                     \
    }                                                                                              \
    while (false)
#endif

#define DO_OR_FAIL(message, x)                                                                     \
    do                                                                                             \
    {                                                                                              \
        if (FREResult do_or_fail_res = x; do_or_fail_res != FRE_OK)                                \
        {                                                                                          \
            FAIL_RESULT(message, do_or_fail_res);                                                  \
        }                                                                                          \
    }                                                                                              \
    while (false)

#define DO_OR_FAIL_EXCEPTION(message, exceptionVar, x)                                             \
    do                                                                                             \
    {                                                                                              \
        FREResult do_or_fail_exception_res = x;                                                    \
        if (do_or_fail_exception_res == FRE_ACTIONSCRIPT_ERROR)                                    \
        {                                                                                          \
            FREObject do_or_fail_exception_message    = FREStringForError(message);                \
            FREObject do_or_fail_exception_ret_args[] = {                                          \
                do_or_fail_exception_message, exceptionVar};                                       \
            FREObject do_or_fail_exception_ret;                                                    \
            DO_OR_FAIL("Could not build nested error",                                             \
                FRENewObject((const uint8_t*)"com.cff.anebe.NestedError", 2,                       \
                    do_or_fail_exception_ret_args, &do_or_fail_exception_ret, nullptr));           \
            FAIL_RETURN(do_or_fail_exception_ret);                                                 \
        }                                                                                          \
        else if (do_or_fail_exception_res != FRE_OK)                                               \
        {                                                                                          \
            FAIL_RESULT(message, do_or_fail_exception_res);                                        \
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

#ifdef ANEBYTECODEEDITOR_HAS_STACKTRACE
inline FREObject GetMember(FREObject o, std::string_view member)
{
    FREObject ret;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not get object property " + std::string(member), exception,
        FREGetObjectProperty(o, (const uint8_t*)member.data(), &ret, &exception));
    return ret;
}

template <FREObjectType Type, typename NumberType = double>
    requires std::same_as<NumberType, uint32_t> || std::same_as<NumberType, int32_t> ||
             std::same_as<NumberType, uint8_t> || std::same_as<NumberType, int8_t> ||
             std::same_as<NumberType, double>
inline auto CheckMember(FREObject o, std::string_view member)
{
    return CHECK_OBJECT<Type, NumberType>(GetMember(o, member), std::string(member));
}
#else
inline FREObject GetMember(FREObject o, std::string_view member,
    std::source_location loc = std::source_location::current())
{
    FREObject ret;
    FREObject exception;
    DO_OR_FAIL_EXCEPTION("Could not get object property " + std::string(member) + " during " +
                             loc.function_name() + " at " + std::to_string(loc.line()),
        exception, FREGetObjectProperty(o, (const uint8_t*)member.data(), &ret, &exception));
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
#endif

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
    FREObject o, const char* m, uint32_t c, FREObject v[], FREObject* r, FREObject* e)
{
    return FRECallObjectMethod(o, (const uint8_t*)m, c, v, r, e);
}

#undef FAIL_RETURN

#define FAIL_RETURN(x) return x
