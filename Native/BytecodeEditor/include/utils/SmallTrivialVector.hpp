#pragma once

#include <array>
#include <initializer_list>
#include <type_traits>

template <typename T, size_t Size>
requires std::is_trivial_v<T> class SmallTrivialVector
{
private:
    std::array<T, Size> data{};
    size_t populated{0};

public:
    consteval SmallTrivialVector() = default;

    consteval SmallTrivialVector(std::initializer_list<T> in) : data(), populated(in.size())
    {
        for (size_t i = 0; i < in.size(); i++)
        {
            data[i] = *(in.begin() + i);
        }
    }

    consteval SmallTrivialVector(std::array<T, Size> in) : data(), populated(Size)
    {
        for (size_t i = 0; i < Size; i++)
        {
            data[i] = in[i];
        }
    }

    template <size_t ArraySize>
    consteval SmallTrivialVector(std::array<T, ArraySize> in) : data(), populated(ArraySize)
    {
        for (size_t i = 0; i < ArraySize; i++)
        {
            data[i] = in[i];
        }
    }

    constexpr const T& operator[](size_t i) const { return data[i]; }
    constexpr T& operator[](size_t i) { return data[i]; }

    constexpr size_t size() const { return populated; }
};
