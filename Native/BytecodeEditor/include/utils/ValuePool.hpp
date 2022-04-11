#pragma once

#include "ASASM/Multiname.hpp"
#include "ASASM/Namespace.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <map>
#include <memory>
#include <stdint.h>
#include <string>
#include <type_traits>

template <typename T, bool haveNull>
class ValuePoolBase
{
public:
    using Key = std::remove_cvref_t<T>;
    static Key toKey(T val) { return val; }
};

template <bool haveNull>
class ValuePoolBase<double, haveNull>
{
    static_assert(sizeof(double) == sizeof(uint64_t));

public:
    using Key = uint64_t;
    static Key toKey(double val) { return std::bit_cast<uint64_t>(val); }
};

template <typename T, bool haveNull>
class ValuePoolBase<std::shared_ptr<T>, haveNull>
{
public:
    using Key = T*;
    static Key toKey(const std::shared_ptr<T>& val) { return val.get(); }
};

template <typename T, bool haveNull = true>
class ValuePool : private ValuePoolBase<T, haveNull>
{
private:
    using Key = ValuePoolBase<T, haveNull>::Key;
    using ValuePoolBase<T, haveNull>::toKey;

    template <typename T>
    constexpr bool isNull(T v)
    {
        using comp = std::remove_cvref_t<decltype(v)>;
        static_assert(std::is_same_v<comp, uint64_t> || std::is_same_v<comp, int64_t> ||
                      std::is_same_v<comp, double> || std::is_same_v<comp, std::string> ||
                      std::is_same_v<comp, ASASM::Multiname> ||
                      std::is_same_v<comp, std::vector<ASASM::Namespace>> ||
                      std::is_same_v<comp, ASASM::Namespace> ||
                      std::is_same_v<comp, ASASM::Metadata> ||
                      std::is_same_v<comp, std::shared_ptr<ASASM::Class>> ||
                      std::is_same_v<comp, std::shared_ptr<ASASM::Method>>);
        if constexpr (std::is_same_v<comp, uint64_t>)
        {
            return v == ABC::ABCFile::NULL_UINT;
        }
        else if constexpr (std::is_same_v<comp, int64_t>)
        {
            return v == ABC::ABCFile::NULL_INT;
        }
        else if constexpr (std::is_same_v<comp, double>)
        {
            return std::isnan(v);
        }
        else if constexpr (std::is_same_v<comp, std::string>)
        {
            return false;
        }
        else if constexpr (std::is_same_v<comp, ASASM::Multiname>)
        {
            return v.kind == ABCType::Void;
        }
        else if constexpr (std::is_same_v<comp, std::vector<ASASM::Namespace>>)
        {
            return v.empty();
        }
        else if constexpr (std::is_same_v<comp, ASASM::Namespace>)
        {
            return v.kind == ABCType::Void;
        }
        else if constexpr (std::is_same_v<comp, ASASM::Metadata>)
        {
            return false;
        }
        else if constexpr (std::is_same_v<comp, std::shared_ptr<ASASM::Class>> ||
                           std::is_same_v<comp, std::shared_ptr<ASASM::Method>>)
        {
            return v == nullptr;
        }
    }

public:
    struct Entry
    {
        uint32_t hits;
        T value;
        size_t addIndex, index;
        std::vector<Key> parents;
    };

    std::map<Key, Entry> pool;
    std::vector<T> values;

    bool add(const T& value)
    {
        if constexpr (haveNull || std::is_pointer_v<Key>)
        {
            if (isNull(value))
            {
                return false;
            }
        }

        auto key = toKey(value);
        if (pool.contains(key))
        {
            pool[key].hits++;
            return false;
        }
        else
        {
            pool.emplace(key, Entry{1, value, pool.size(), 0, {}});
            return true;
        }
    }

    bool notAdded(const T& value)
    {
        if constexpr (haveNull || std::is_pointer_v<Key>)
        {
            if (isNull(value))
            {
                return false;
            }
        }

        auto key = toKey(value);
        if (pool.contains(key))
        {
            pool[key].hits++;
            return false;
        }
        return true;
    }

    void registerDependency(const T& _from, const T& _to)
    {
        auto from = toKey(_from);
        auto to   = toKey(_to);

        assert(pool.contains(from));
        assert(pool.contains(to));

        auto& fromV = pool[from];

        assert(std::none_of(
            fromV.parents.begin(), fromV.parents.end(), [to](const Key& k) { return k == to; }));

        fromV.parents.emplace_back(to);
    }

    std::vector<T> getPreliminaryValues() const
    {
        std::vector<T> ret;
        ret.reserve(pool.size());

        for (const auto& pair : pool)
        {
            ret.emplace_back(pair.second.value);
        }

        return ret;
    }

    std::vector<T>& finalize()
    {
        std::vector<Entry*> all;
        all.reserve(pool.size());

        for (auto& pair : pool)
        {
            all.emplace_back(&pair.second);
        }

        if constexpr (std::is_same_v<T, double>)
        {
            constexpr auto sortPred = [](Entry* a, Entry* b) {
                return a->hits > b->hits ||
                       (a->hits == b->hits &&
                           std::bit_cast<uint64_t>(a->value) < std::bit_cast<uint64_t>(b->value));
            };
            std::sort(all.begin(), all.end(), sortPred);
        }
        else if constexpr (std::is_pointer_v<Key>)
        {
            constexpr auto sortPred = [](Entry* a, Entry* b) {
                return a->hits > b->hits || (a->hits == b->hits && a->addIndex < b->addIndex);
            };
            std::sort(all.begin(), all.end(), sortPred);
        }
        else
        {
            constexpr auto sortPred = [](Entry* a, Entry* b) {
                return a->hits > b->hits || (a->hits == b->hits && a->value < b->value);
            };
            std::sort(all.begin(), all.end(), sortPred);
        }

    // topographical sort
    topSort:

        for (size_t i = 0; i < all.size(); i++)
        {
            all[i]->index = i;
        }

        for (const auto& a : pool)
        {
            for (const auto& parent : a.second.parents)
            {
                assert(pool.contains(parent));
                if (pool[parent].index > a.second.index)
                {
                    all.erase(all.begin() + pool[parent].index);
                    all.insert(all.begin() + a.second.index, &pool[parent]);
                    goto topSort;
                }
            }
        }

        values.reserve(all.size() + (haveNull ? 1 : 0));
        if constexpr (haveNull)
        {
            values.emplace_back();
        }

        for (const Entry* e : all)
        {
            values.emplace_back(e->value);
        }

        return values;
    }

    uint32_t get(T value)
    {
        if (haveNull && isNull(value))
        {
            return 0;
        }
        return pool[toKey(value)].index + (haveNull ? 1 : 0);
    }
};
