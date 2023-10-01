#pragma once

#include <tuple>

namespace std_ext
{
    /*
     * Tuple type index. Source: https://stackoverflow.com/questions/18063451/get-index-of-a-tuple-elements-type
    */
    template<class T, class Tuple>
    struct Index;

    template<class T, class... Types>
    struct Index<T, std::tuple<T, Types...>>
    {
        static const std::size_t value = 0;
    };

    template<class T, class U, class... Types>
    struct Index<T, std::tuple<U, Types...>>
    {
        static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
    };
    /*
     * Here ends code from stackoverflow.
    */

    template<class T, class Tuple>
    T& get(Tuple& tuple)
    {
        return std::get<Index<T, Tuple>::value>(tuple);
    }
};

