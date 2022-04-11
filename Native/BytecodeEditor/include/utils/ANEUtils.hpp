#pragma once

#include <FlashRuntimeExtensions.h>

#include <string>
#include <string_view>

#define FAIL_RETURN(x) return x

#define DO_OR_FAIL_NULL(x)                                                                         \
    if (x != FRE_OK)                                                                               \
    FAIL_RETURN(nullptr)

#define FAIL_RESULT(message, result)                                                               \
    do                                                                                             \
    {                                                                                              \
        FREObject strThrow;                                                                        \
        DO_OR_FAIL_NULL(FREString(std::to_string(int32_t(result)) + ": " + message, &strThrow));   \
        FAIL_RETURN(strThrow);                                                                     \
    } while (false)

#define FAIL(message)                                                                              \
    do                                                                                             \
    {                                                                                              \
        FREObject strThrow;                                                                        \
        DO_OR_FAIL_NULL(FREString(message, &strThrow));                                            \
        FAIL_RETURN(strThrow);                                                                     \
    } while (false)

#define DO_OR_FAIL(message, x)                                                                     \
    do                                                                                             \
    {                                                                                              \
        if (FREResult res = x; res != FRE_OK)                                                      \
        {                                                                                          \
            FAIL_RESULT(message, res);                                                             \
        }                                                                                          \
    } while (false)

inline FREResult FREString(std::string_view v, FREObject* res) noexcept
{
    return FRENewObjectFromUTF8(v.size(), (const uint8_t*)v.data(), res);
}
