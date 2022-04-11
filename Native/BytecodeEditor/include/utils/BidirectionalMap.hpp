#pragma once

#include <array>
#include <functional>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

// Note: both Key and Value must have a default constructor
// Note: KeyComparator and ValueComparator must act similarly to std::equal_to
template <typename K, typename V, size_t Size, typename KC = std::equal_to<>,
    typename VC = std::equal_to<>>
class BidirectionalMap
{
    static_assert(Size > 0);

private:
    template <size_t ArraySize>
    constexpr void populate(const std::pair<K, V>* data)
    {
        static_assert(ArraySize <= Size && ArraySize > 0);

        for (size_t i = 0; i < ArraySize; i++)
            this->entries[i] = data[i];

        if constexpr (ArraySize < Size)
        {
            for (size_t i = ArraySize; i < Size; i++)
                this->entries[i] = {Key(), Value()};
        }
    }

public:
    using Key             = std::remove_cvref_t<K>;
    using Value           = std::remove_cvref_t<V>;
    using KeyComparator   = std::remove_cvref_t<KC>;
    using ValueComparator = std::remove_cvref_t<VC>;

    using Entry = std::pair<Key, Value>;

    BidirectionalMap() = delete;

    template <size_t ArraySize>
    constexpr BidirectionalMap(
        const std::pair<K, V> (&inEntries)[ArraySize], KC kc = KC{}, VC vc = VC{})
        : entries(), populated(ArraySize), kc(kc), vc(vc)
    {
        populate<ArraySize>(inEntries);
    }

    template <size_t ArraySize>
    constexpr BidirectionalMap(
        const std::array<std::pair<K, V>, ArraySize>(&inEntries), KC kc = KC{}, VC vc = VC{})
        : entries(), populated(ArraySize), kc(kc), vc(vc)
    {
        populate<ArraySize>(inEntries.data());
    }

    constexpr BidirectionalMap(const std::pair<K, V> (&inEntries)[Size], KC kc = KC{}, VC vc = VC{})
        : entries(), populated(Size), kc(kc), vc(vc)
    {
        populate<Size>(inEntries);
    }

    constexpr BidirectionalMap(
        const std::array<std::pair<K, V>, Size>(&inEntries), KC kc = KC{}, VC vc = VC{})
        : entries(), populated(Size), kc(kc), vc(vc)
    {
        populate<Size>(inEntries.data());
    }

    constexpr std::optional<Value> Find(const Key& search) const
    {
        for (size_t i = 0; i < this->populated; ++i)
        {
            if (kc(this->entries[i].first, search))
            {
                return this->entries[i].second;
            }
        }

        return std::nullopt;
    }

    constexpr std::optional<Key> ReverseFind(const Value& search) const
    {
        for (size_t i = 0; i < this->populated; ++i)
        {
            if (vc(this->entries[i].second, search))
            {
                return this->entries[i].first;
            }
        }

        return std::nullopt;
    }

    constexpr std::pair<const Entry*, size_t> GetEntries() const
    {
        return {entries.data(), this->populated};
    }

private:
    std::array<Entry, Size> entries;
    const size_t populated;
    [[no_unique_address]] KeyComparator kc;
    [[no_unique_address]] ValueComparator vc;
};

template <typename A, typename B, typename... Args>
struct CheckArgs
{
private:
    using CheckArgs2 = CheckArgs<Args...>;

    static constexpr bool KeyConvertible =
        std::is_convertible_v<std::remove_cvref_t<A>,
            std::remove_cvref_t<typename CheckArgs2::AType>> ||
        std::is_convertible_v<std::remove_cvref_t<typename CheckArgs2::AType>,
            std::remove_cvref_t<A>>;
    static constexpr bool ValueConvertible =
        std::is_convertible_v<std::remove_cvref_t<B>,
            std::remove_cvref_t<typename CheckArgs2::BType>> ||
        std::is_convertible_v<std::remove_cvref_t<typename CheckArgs2::BType>,
            std::remove_cvref_t<B>>;

public:
    static constexpr bool value = KeyConvertible && ValueConvertible && CheckArgs2::value;

    using AType = std::conditional_t<sizeof...(Args) == 0, A,
        std::conditional_t<std::is_convertible_v<std::remove_cvref_t<typename CheckArgs2::AType>,
                               std::remove_cvref_t<A>>,
            std::remove_cvref_t<A>, std::remove_cvref_t<typename CheckArgs2::AType>>>;
    using BType = std::conditional_t<sizeof...(Args) == 0, B,
        std::conditional_t<std::is_convertible_v<std::remove_cvref_t<typename CheckArgs2::BType>,
                               std::remove_cvref_t<B>>,
            std::remove_cvref_t<B>, std::remove_cvref_t<typename CheckArgs2::BType>>>;
};

template <typename A, typename B>
struct CheckArgs<A, B>
{
    static constexpr bool value = true;
    using AType                 = std::remove_cvref_t<A>;
    using BType                 = std::remove_cvref_t<B>;
};

template <typename KeyComparator, typename ValueComparator, typename... Args>
requires(sizeof...(Args) % 2 == 0) && (sizeof...(Args) > 0) &&
    (CheckArgs<Args...>::value) constexpr auto MakeMapWithComparators(
        KeyComparator kc, ValueComparator vc, Args... args)
{
    using check = CheckArgs<Args...>;

    auto setArgs = []<typename K, typename V, size_t Size>(
                       std::array<std::pair<K, V>, Size>& addTo, Args... args)
    {
        auto setArgsRef = []<typename A, typename B, typename... InnerArgs>(
            auto& me, std::array<std::pair<K, V>, Size>& addTo, A key, B val, InnerArgs... args)
        {
            size_t index        = Size - (sizeof...(InnerArgs) + 2) / 2;
            addTo[index].first  = key;
            addTo[index].second = val;

            if constexpr (sizeof...(InnerArgs) != 0)
            {
                me(me, addTo, std::forward<InnerArgs>(args)...);
            }
        };
        setArgsRef(setArgsRef, addTo, args...);
    };

    std::array<std::pair<typename check::AType, typename check::BType>, sizeof...(Args) / 2>
        entries{};

    setArgs(entries, std::forward<Args>(args)...);

    return BidirectionalMap<typename check::AType, typename check::BType, sizeof...(Args) / 2,
        KeyComparator, ValueComparator>{entries, kc, vc};
}

// Works for all types with operator == defined
template <typename... Args>
requires(sizeof...(Args) % 2 == 0) && (sizeof...(Args) > 0) &&
    (CheckArgs<Args...>::value) constexpr auto MakeMap(Args... args)
{
    return MakeMapWithComparators(std::equal_to<>(), std::equal_to<>(), args...);
}

struct cstringcomp
{
    constexpr bool operator()(const char* a, const char* b) const
    {
        size_t idx = 0;
        while (a[idx] != '\0' && b[idx] != '\0')
        {
            if (a[idx] != b[idx])
            {
                return false;
            }
            idx++;
        }

        if ((a[idx] == '\0') != (b[idx] == '\0'))
        {
            return false;
        }

        return true;
    }

    constexpr bool operator()(const char* a, std::string_view b) const { return a == b; }

    constexpr bool operator()(std::string_view b, const char* a) const { return a == b; }
};

template <typename ValueComparator = std::equal_to<>, typename... Args>
requires(sizeof...(Args) % 2 == 0) && (sizeof...(Args) > 0) && (CheckArgs<Args...>::value) &&
    (std::is_same_v<typename CheckArgs<Args...>::AType, const char*>)constexpr auto MakeCStringMap(
        ValueComparator vc = ValueComparator(), Args... args)
{
    return MakeMapWithComparators(cstringcomp(), vc, args...);
}
