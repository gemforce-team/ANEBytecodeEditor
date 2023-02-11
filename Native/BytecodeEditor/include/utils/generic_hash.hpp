#pragma once

#include <functional>

template <typename T, auto MemberPointer, auto... MemberPointers>
    requires requires(T v) {
                 v.*MemberPointer;
                 std::hash<std::remove_cvref_t<decltype(v.*MemberPointer)>>{}(v.*MemberPointer);
             }
constexpr size_t generic_hash(const T& val) noexcept
{
    static constexpr auto hash_combine = [](size_t seed, size_t value)
    { return seed ^ value + 0x9e3779b9 + (seed << 6) + (seed >> 2); };

    if constexpr (sizeof...(MemberPointers) == 0)
    {
        return std::hash<std::remove_cvref_t<decltype(val.*MemberPointer)>>{}(val.*MemberPointer);
    }
    else
    {
        return hash_combine(
            std::hash<std::remove_cvref_t<decltype(val.*MemberPointer)>>{}(val.*MemberPointer),
            generic_hash<T, MemberPointers...>(val));
    }
}

#define DEFINE_HASH(Type, ...)                                                                     \
    template <>                                                                                    \
    struct std::hash<Type>                                                                         \
    {                                                                                              \
        size_t operator()(const Type& v) const noexcept                                            \
        {                                                                                          \
            return generic_hash<Type, __VA_ARGS__>(v);                                             \
        }                                                                                          \
    }
