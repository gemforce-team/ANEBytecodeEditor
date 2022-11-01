#pragma once

#include <array>
#include <functional>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

namespace BidirectionalMapInternals
{
    // Used to compare C strings; operator== doesn't work properly for those
    struct cstringcomp
    {
        constexpr bool operator()(const char* a, const char* b) const
        {
            if (a == nullptr && b == nullptr)
            {
                return true;
            }
            else if (a == nullptr || b == nullptr)
            {
                return false;
            }
            return std::string_view(a) == std::string_view(b);
        }

        constexpr bool operator()(const char* a, std::string_view b) const
        {
            return a == nullptr ? b.length() == 0 : a == b;
        }

        constexpr bool operator()(std::string_view a, const char* b) const
        {
            return b == nullptr ? a.length() == 0 : a == b;
        }

        constexpr bool operator()(std::string_view a, std::string_view b) const { return a == b; }
    };

    template <typename T>
    struct DefaultComparatorForType
    {
    private:
        using testtype = std::remove_cvref_t<T>;

    public:
        using value =
            std::conditional_t<std::is_same_v<testtype, const char*>, cstringcomp, std::equal_to<>>;
    };

    template <typename T>
    using defaultcomp_v = typename DefaultComparatorForType<T>::value;

    template <typename...>
    struct FoldArgs : public std::false_type
    {
        using AType  = void;
        using BType  = void;
        using KCType = void;
        using VCType = void;
    };

    template <typename A, typename B>
    struct FoldArgs<A, B> : public std::true_type
    {
        using AType = std::remove_cvref_t<A>;
        using BType = std::remove_cvref_t<B>;

        using KCType = defaultcomp_v<AType>;
        using VCType = defaultcomp_v<BType>;
    };

    template <typename A, typename B, std::common_with<std::remove_cvref_t<A>> APrime,
        std::common_with<std::remove_cvref_t<B>> BPrime>
    struct FoldArgs<A, B, APrime, BPrime> : std::true_type
    {
        using AType = std::remove_cvref_t<std::common_type_t<A, APrime>>;
        using BType = std::remove_cvref_t<std::common_type_t<B, BPrime>>;

        using KCType = defaultcomp_v<AType>;
        using VCType = defaultcomp_v<BType>;
    };

    template <typename A, typename B, std::common_with<std::remove_cvref_t<A>> APrime,
        std::common_with<std::remove_cvref_t<B>> BPrime, typename... Args>
    struct FoldArgs<A, B, APrime, BPrime, Args...>
        : public FoldArgs<std::remove_cvref_t<std::common_type_t<A, APrime>>,
              std::remove_cvref_t<std::common_type_t<B, BPrime>>, Args...>
    {
    };

    template <typename ActualKeyType, typename ActualValueType, typename... Args>
    struct ConvertibleArgs : public std::false_type
    {
    };

    template <typename ActualKeyType, typename ActualValueType,
        std::convertible_to<ActualKeyType> ArgKey, std::convertible_to<ActualValueType> ArgVal>
    struct ConvertibleArgs<ActualKeyType, ActualValueType, ArgKey, ArgVal> : public std::true_type
    {
    };

    template <typename ActualKeyType, typename ActualValueType,
        std::convertible_to<ActualKeyType> ArgKey, std::convertible_to<ActualValueType> ArgVal,
        typename... Args>
    struct ConvertibleArgs<ActualKeyType, ActualValueType, ArgKey, ArgVal, Args...>
        : public ConvertibleArgs<ActualKeyType, ActualValueType, Args...>

    {
    };

    template <typename CheckMe, std::convertible_to<std::remove_cvref_t<CheckMe>>... Args>
    struct UnfoldedConvertible : public std::true_type
    {
        using Type = std::remove_cvref_t<CheckMe>;
        using Comp = defaultcomp_v<Type>;
    };

    template <std::size_t V1, std::size_t V2>
    concept is_less_equal = (V1 <= V2);

    template <std::size_t V1, std::size_t V2>
    concept is_less = (V1 < V2);
} // namespace BidirectionalMapInternals

template <std::movable Key, std::movable Value, std::size_t Size,
    std::equivalence_relation<Key, Key> KeyComparator       = std::equal_to<>,
    std::equivalence_relation<Value, Value> ValueComparator = std::equal_to<>>
    requires BidirectionalMapInternals::is_less_equal<0, Size> &&
             std::same_as<Key, std::remove_cvref_t<Key>> &&
             std::same_as<Value, std::remove_cvref_t<Value>> &&
             std::same_as<KeyComparator, std::remove_cvref_t<KeyComparator>> &&
             std::same_as<ValueComparator, std::remove_cvref_t<ValueComparator>>
class BidirectionalMap
{
public:
    using key_type   = Key;
    using value_type = Value;

    using Entry = std::pair<Key, Value>;

private:
    static consteval void buildVectorHelper(
        std::vector<std::pair<Key, Value>>& fillMe, auto&& a, auto&& b, auto&&... args)
    {
        if constexpr (sizeof...(args) == 0)
        {
            fillMe.emplace_back(std::forward<decltype(a)>(a), std::forward<decltype(b)>(b));
        }
        else
        {
            fillMe.emplace_back(std::forward<decltype(a)>(a), std::forward<decltype(b)>(b));
            return buildVectorHelper(fillMe, std::forward<decltype(args)>(args)...);
        }
    }

    static consteval std::vector<std::pair<Key, Value>> buildArgVector(auto&&... args)
    {
        std::vector<std::pair<Key, Value>> ret;
        buildVectorHelper(ret, std::forward<decltype(args)>(args)...);
        return ret;
    }

    static consteval auto buildArgArray(auto&&... args)
    {
        auto buildArrayHelper =
            []<std::size_t... Indices>(auto&& v, std::index_sequence<Indices...>)
                ->std::array<std::pair<Key, Value>, sizeof...(args) / 2>
        {
            return {{std::move(v[Indices])...}};
        };
        return buildArrayHelper(buildArgVector(std::forward<decltype(args)>(args)...),
            std::make_index_sequence<sizeof...(args) / 2>{});
    }

    template <typename... ArgsA, typename... ArgsB>
    static consteval std::array<std::pair<Key, Value>, sizeof...(ArgsA)> buildArgArray(
        std::pair<ArgsA, ArgsB>&&... args)
    {
        return {{std::move(args)...}};
    }

    template <std::ranges::contiguous_range PairRange, std::size_t... PairSeq>
    consteval BidirectionalMap(std::ranges::owning_view<PairRange>&& pairs, auto&& kc, auto&& vc,
        std::index_sequence<PairSeq...>)
        : entries{{std::forward<std::ranges::range_value_t<PairRange>>(
              *(pairs.begin() + PairSeq))...}},
          populated{std::min(std::ranges::size(pairs), Size)},
          kc{std::forward<decltype(kc)>(kc)},
          vc{std::forward<decltype(vc)>(vc)}
    {
    }

    template <std::ranges::contiguous_range PairRange, std::size_t RangeSize>
    consteval BidirectionalMap(std::ranges::owning_view<PairRange>&& pairs, auto&& kc, auto&& vc,
        std::integral_constant<std::size_t, RangeSize>)
        : BidirectionalMap(std::forward<decltype(pairs)>(pairs), std::forward<decltype(kc)>(kc),
              std::forward<decltype(vc)>(vc), std::make_index_sequence<RangeSize>{})
    {
    }

public:
    BidirectionalMap() = delete;

    template <std::convertible_to<Key> CurrentKey = Key,
        std::convertible_to<Value> CurrentVal = Value, std::size_t ArraySize = Size,
        std::convertible_to<KeyComparator> KC   = KeyComparator,
        std::convertible_to<ValueComparator> VC = ValueComparator>
        requires BidirectionalMapInternals::is_less_equal<ArraySize, Size>
    consteval BidirectionalMap(
        std::pair<CurrentKey, CurrentVal> (&&inEntries)[ArraySize], KC&& kc = KC{}, VC&& vc = VC{})
        : BidirectionalMap(std::ranges::owning_view(std::forward<decltype(inEntries)>(inEntries)),
              std::forward<KC>(kc), std::forward<VC>(vc),
              std::integral_constant<std::size_t, ArraySize>{})
    {
    }

    template <std::convertible_to<Key> CurrentKey = Key,
        std::convertible_to<Value> CurrentVal = Value, std::size_t ArraySize = Size,
        std::convertible_to<KeyComparator> KC   = KeyComparator,
        std::convertible_to<ValueComparator> VC = ValueComparator>
        requires BidirectionalMapInternals::is_less_equal<ArraySize, Size>
    consteval BidirectionalMap(std::array<std::pair<CurrentKey, CurrentVal>, ArraySize>&& inEntries,
        KC&& kc = KC{}, VC&& vc = VC{})
        : BidirectionalMap(std::ranges::owning_view(std::forward<decltype(inEntries)>(inEntries)),
              std::forward<KC>(kc), std::forward<VC>(vc),
              std::integral_constant<std::size_t, ArraySize>{})
    {
    }

    template <typename... Args, std::convertible_to<KeyComparator> KC,
        std::convertible_to<ValueComparator> VC>
        requires BidirectionalMapInternals::ConvertibleArgs<Key, Value, Args...>::value &&
                 BidirectionalMapInternals::is_less_equal<sizeof...(Args) / 2, Size>
    consteval BidirectionalMap(KC&& kc, VC&& vc, Args&&... args)
        : BidirectionalMap(buildArgArray(std::forward<Args>(args)...), std::forward<KC>(kc),
              std::forward<VC>(vc))
    {
    }

    template <typename... Args>
        requires BidirectionalMapInternals::ConvertibleArgs<Key, Value, Args...>::value &&
                 BidirectionalMapInternals::is_less_equal<sizeof...(Args) / 2, Size>
    consteval BidirectionalMap(Args&&... args)
        : BidirectionalMap(BidirectionalMapInternals::defaultcomp_v<Key>(),
              BidirectionalMapInternals::defaultcomp_v<Value>(), std::forward<Args>(args)...)
    {
    }

    template <std::convertible_to<KeyComparator> KC, std::convertible_to<ValueComparator> VC,
        std::convertible_to<Key>... ArgsA, std::convertible_to<Value>... ArgsB>
        requires BidirectionalMapInternals::is_less_equal<sizeof...(ArgsA), Size>
    consteval BidirectionalMap(KC&& kc, VC&& vc, std::pair<ArgsA, ArgsB>&&... args)
        : BidirectionalMap(buildArgArray(std::forward<decltype(args)>(args)...),
              std::forward<KC>(kc), std::forward<VC>(vc))
    {
    }

    template <std::convertible_to<KeyComparator> KC, std::convertible_to<ValueComparator> VC,
        std::convertible_to<Key>... ArgsA, std::convertible_to<Value>... ArgsB>
        requires BidirectionalMapInternals::is_less_equal<sizeof...(ArgsA), Size>
    consteval BidirectionalMap(std::pair<ArgsA, ArgsB>&&... args)
        : BidirectionalMap(BidirectionalMapInternals::defaultcomp_v<Key>(),
              BidirectionalMapInternals::defaultcomp_v<Value>(),
              std::forward<decltype(args)>(args)...)
    {
    }

    /*
    ** When mapped as T, V -- find value V from T
    */
    template <typename K = Key>
        requires std::equivalence_relation<KeyComparator, Key, K> ||
                 std::equivalence_relation<KeyComparator, K, Key>
    constexpr std::optional<std::reference_wrapper<const Value>> Find(const K& search) const
    {
        for (std::size_t i = 0; i < this->populated; ++i)
        {
            if (compareKey(this->entries[i].first, search))
            {
                return std::ref(this->entries[i].second);
            }
        }

        return std::nullopt;
    }

    /*
    ** When mapped as T, V -- find value T from V
    */
    template <typename V = Value>
        requires std::equivalence_relation<ValueComparator, Value, V> ||
                 std::equivalence_relation<ValueComparator, V, Value>
    constexpr std::optional<std::reference_wrapper<const Key>> ReverseFind(const V& search) const
    {
        for (std::size_t i = 0; i < this->populated; ++i)
        {
            if (compareValue(this->entries[i].second, search))
            {
                return std::ref(this->entries[i].first);
            }
        }

        return std::nullopt;
    }

    constexpr auto GetKeys() const
    {
        return std::ranges::ref_view(this->entries) | std::views::keys |
               std::views::take(this->populated);
    }

    /* Can only be used on String-mapped Keys */
    constexpr auto GetNames() const
        requires std::is_same_v<Key, const char*> || std::is_same_v<Key, char*> ||
                 std::is_same_v<Key, std::string_view>
    {
        return GetKeys();
    }

    constexpr auto GetValues() const
    {
        return std::ranges::ref_view(this->entries) | std::views::values |
               std::views::take(this->populated);
    }

    constexpr std::span<const Entry> GetEntries() const
    {
        return {entries.begin(), entries.begin() + populated};
    }

private:
    std::array<Entry, Size> entries;
    std::size_t populated;

    [[no_unique_address]] KeyComparator kc;
    [[no_unique_address]] ValueComparator vc;

    template <typename V>
        requires std::equivalence_relation<ValueComparator, Value, V>
    constexpr auto compareValue(const Value& v1, V&& v2) const
    {
        return std::invoke(vc, v1, std::forward<V>(v2));
    }

    template <typename V>
        requires std::equivalence_relation<ValueComparator, V, Value> &&
                 (!std::equivalence_relation<ValueComparator, Value, V>)
    constexpr auto compareValue(const Value& v1, V&& v2) const
    {
        return std::invoke(vc, std::forward<V>(v2), v1);
    }

    template <typename K>
        requires std::equivalence_relation<KeyComparator, Key, K>
    constexpr auto compareKey(const Key& v1, K&& v2) const
    {
        return std::invoke(kc, v1, std::forward<K>(v2));
    }

    template <typename K>
        requires std::equivalence_relation<KeyComparator, K, Key> &&
                 (!std::equivalence_relation<KeyComparator, Key, K>)
    constexpr auto compareKey(const Key& v1, K&& v2) const
    {
        return std::invoke(kc, std::forward<K>(v2), v1);
    }
};

template <typename KC, typename VC, typename... Args,
    typename Folded = BidirectionalMapInternals::FoldArgs<Args...>>
    requires Folded::value
BidirectionalMap(KC&&, VC&&, Args&&...) -> BidirectionalMap<typename Folded::AType,
    typename Folded::BType, sizeof...(Args) / 2, std::remove_cvref_t<KC>, std::remove_cvref_t<VC>>;

template <typename... Args, typename Folded = BidirectionalMapInternals::FoldArgs<Args...>>
    requires Folded::value
BidirectionalMap(Args&&...) -> BidirectionalMap<typename Folded::AType, typename Folded::BType,
    sizeof...(Args) / 2, typename Folded::KCType, typename Folded::VCType>;

template <typename KC, typename VC, typename... ArgsA, typename... ArgsB,
    typename UnfoldedA =
        BidirectionalMapInternals::UnfoldedConvertible<std::common_type_t<ArgsA...>, ArgsA...>,
    typename UnfoldedB =
        BidirectionalMapInternals::UnfoldedConvertible<std::common_type_t<ArgsB...>, ArgsB...>>
BidirectionalMap(KC&&, VC&&, std::pair<ArgsA, ArgsB>&&...)
    -> BidirectionalMap<typename UnfoldedA::Type, typename UnfoldedB::Type, sizeof...(ArgsA),
        std::remove_cvref_t<KC>, std::remove_cvref_t<VC>>;

template <typename... ArgsA, typename... ArgsB,
    typename UnfoldedA =
        BidirectionalMapInternals::UnfoldedConvertible<std::common_type_t<ArgsA...>, ArgsA...>,
    typename UnfoldedB =
        BidirectionalMapInternals::UnfoldedConvertible<std::common_type_t<ArgsB...>, ArgsB...>>
BidirectionalMap(std::pair<ArgsA, ArgsB>&&...) -> BidirectionalMap<typename UnfoldedA::Type,
    typename UnfoldedB::Type, sizeof...(ArgsA), typename UnfoldedA::Comp, typename UnfoldedB::Comp>;

template <typename K, typename V, typename KC, typename VC, std::size_t ArraySize>
BidirectionalMap(std::pair<K, V> (&&)[ArraySize], KC&&, VC&&)
    -> BidirectionalMap<std::remove_cvref_t<K>, std::remove_cvref_t<V>, ArraySize,
        std::remove_cvref_t<KC>, std::remove_cvref_t<VC>>;

template <typename K, typename V, std::size_t ArraySize>
BidirectionalMap(std::pair<K, V> (&&)[ArraySize])
    -> BidirectionalMap<std::remove_cvref_t<K>, std::remove_cvref_t<V>, ArraySize,
        BidirectionalMapInternals::defaultcomp_v<std::remove_cvref_t<K>>,
        BidirectionalMapInternals::defaultcomp_v<std::remove_cvref_t<V>>>;

template <typename K, typename V, typename KC, typename VC, std::size_t ArraySize>
BidirectionalMap(std::array<std::pair<K, V>, ArraySize>&&, KC&&, VC&&)
    -> BidirectionalMap<std::remove_cvref_t<K>, std::remove_cvref_t<V>, ArraySize,
        std::remove_cvref_t<KC>, std::remove_cvref_t<VC>>;

template <typename K, typename V, std::size_t ArraySize>
BidirectionalMap(std::array<std::pair<K, V>, ArraySize>&&)
    -> BidirectionalMap<std::remove_cvref_t<K>, std::remove_cvref_t<V>, ArraySize,
        BidirectionalMapInternals::defaultcomp_v<std::remove_cvref_t<K>>,
        BidirectionalMapInternals::defaultcomp_v<std::remove_cvref_t<V>>>;
