#pragma once

#include <cstddef>
#include <utility>

#define ALWAYS_INLINE inline __attribute__((always_inline))

namespace Utils
{
    typedef ::std::size_t USize;

    template<typename Type>
    class Allocator
    {
    public:
        typedef Type ValueType;
        typedef Allocator<Type> AllocatorType;

        static ALWAYS_INLINE constexpr ValueType* Allocate() noexcept { return reinterpret_cast<ValueType*>(new unsigned char[sizeof(ValueType)]); }
        static ALWAYS_INLINE constexpr ValueType* Allocate(USize n) noexcept { return reinterpret_cast<ValueType*>(new unsigned char[sizeof(ValueType) * n]); }
        
        static ALWAYS_INLINE constexpr void Deallocate(ValueType* ptr) noexcept { delete reinterpret_cast<unsigned char*>(ptr); }
        static ALWAYS_INLINE constexpr void Deallocate(ValueType* ptr, [[maybe_unused]] USize n) noexcept { delete[] reinterpret_cast<unsigned char*>(ptr); }

        template<typename... Args>
        static ALWAYS_INLINE constexpr void ConstructAt(ValueType* ptr, Args&&... args) noexcept { new (ptr) ValueType(::std::forward<Args>(args)...); }
        static ALWAYS_INLINE constexpr void DestructAt(ValueType* ptr) noexcept { ptr->~ValueType(); }
    };

    template<>
    class Allocator<int>
    {
    public:
        typedef int ValueType;
        typedef Allocator<int> AllocatorType;
        
        static ALWAYS_INLINE constexpr ValueType* Allocate() noexcept { return new int; }
        static ALWAYS_INLINE constexpr ValueType* Allocate(USize n) noexcept { return new int[n]; }

        static ALWAYS_INLINE constexpr void Deallocate(int* ptr) noexcept { delete ptr; }
        static ALWAYS_INLINE constexpr void Deallocate(int* ptr, [[maybe_unused]] USize n) noexcept { delete[] ptr; }

        static ALWAYS_INLINE constexpr void ConstructAt([[maybe_unused]] int* ptr) noexcept {}
        static ALWAYS_INLINE constexpr void ConstructAt(int* ptr, int v) noexcept { new (ptr) int(v); }
        static ALWAYS_INLINE constexpr void DestructAt([[maybe_unused]] int* ptr) noexcept {}
    };
};

