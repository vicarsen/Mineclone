#pragma once

#include <unordered_map>
#include <unordered_set>

namespace Utils
{
    template<typename Type>
    using Hash = ::std::hash<Type>;

    template<typename Key, typename Hash = Hash<Key>, typename Equals = ::std::equal_to<Key>, typename Alloc = ::std::allocator<Key>>
    using HashSet = ::std::unordered_set<Key, Hash, Equals, Alloc>;

    template<typename Key, typename Value, typename Hash = Hash<Key>, typename Equals = ::std::equal_to<Key>, typename Alloc = ::std::allocator<::std::pair<const Key, Value>>>
    using HashMap = ::std::unordered_map<Key, Value, Hash, Equals, Alloc>;
};

