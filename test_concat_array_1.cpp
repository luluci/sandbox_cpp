#include <iostream>
#include <array>

// Helper function to concatenate two arrays
template <std::size_t... Indices1, std::size_t... Indices2, typename T, std::size_t N1, std::size_t N2>
constexpr auto concatenateArraysImpl3(const std::array<T, N1> &arr1, const std::array<T, N2> &arr2, std::index_sequence<Indices1...>, std::index_sequence<Indices2...>)
{
    return std::array<T, N1 + N2>{arr1[Indices1]..., arr2[Indices2]...};
}

// Recursive function to concatenate multiple arrays
template <typename T, size_t TN, typename U, size_t UN>
constexpr auto concatenateArraysImpl2(const std::array<T, TN> &arr1, const std::array<U, UN> &arr2)
{
    return concatenateArraysImpl3(arr1, arr2, std::make_index_sequence<TN>(), std::make_index_sequence<UN>());
}
// Recursive function to concatenate multiple arrays
template <typename T, typename U>
constexpr auto concatenateArraysImpl1(const T &arr1, const U &arr2)
{
    return concatenateArraysImpl2(arr1, arr2);
}

// Recursive function to concatenate multiple arrays
template <typename T, typename U, typename... Arrays>
constexpr auto concatenateArraysImpl1(const T &arr1, const U &arr2, const Arrays &...arrays)
{
    return concatenateArraysImpl1(concatenateArraysImpl3(arr1, arr2), concatenateArrays(arrays...));
}

// Recursive function to concatenate multiple arrays
template <typename T>
constexpr auto concatenateArrays(const T &arr)
{
    return arr;
}

// Recursive function to concatenate multiple arrays
template <typename T, typename... Arrays>
constexpr auto concatenateArrays(const T &arr, const Arrays &...arrays)
{
    return concatenateArraysImpl1(arr, concatenateArrays(arrays...));
}

int main()
{
    constexpr std::size_t arraySize1 = 3;
    constexpr std::array<int, arraySize1> array1 = {1, 2, 3};

    constexpr std::size_t arraySize2 = 4;
    constexpr std::array<int, arraySize2> array2 = {4, 5, 6, 7};

    constexpr std::size_t arraySize3 = 2;
    constexpr std::array<int, arraySize3> array3 = {8, 9};

    constexpr auto concatenatedArray = concatenateArrays(array1, array2, array3);

    for (const auto &element : concatenatedArray)
    {
        std::cout << element << " ";
    }

    return 0;
}
