#pragma once

#include <tuple>

#include "type.h"

namespace Utils
{
    template<typename... Types>
    using Tuple = ::std::tuple<Types...>;

    /*
     * Tuple type index. Source: https://stackoverflow.com/questions/18063451/get-index-of-a-tuple-elements-type
    */
    template<class T, class Tuple>
    struct Index;

    template<class T, class... Types>
    struct Index<T, Tuple<T, Types...>>
    {
        static const USize value = 0;
    };

    template<class T, class U, class... Types>
    struct Index<T, Tuple<U, Types...>>
    {
        static const USize value = 1 + Index<T, Tuple<Types...>>::value;
    };
    /*
     * Here ends code from stackoverflow.
    */
    
    template<class T, class Tuple>
    T& get(Tuple& tuple)
    {
        return ::std::get<Index<T, Tuple>::value>(tuple);
    }

    template<typename... Args>
    struct Pack
    {
    };
};

