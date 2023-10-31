#pragma once

#include <cstddef>
#include <utility>

namespace Utils
{
    typedef std::size_t USize;

    template<typename Type>
    class Allocator
    {
    public:
        typedef Type ValueType;
        typedef Allocator<Type> AllocatorType;

        Allocator() = default;
        Allocator(AllocatorType&& other) = default;
        Allocator(const AllocatorType& other) = default;

        ~Allocator() = default;

        AllocatorType& operator=(AllocatorType&& other) = default;
        AllocatorType& operator=(const AllocatorType& other) = default;

        inline ValueType* Allocate() { return reinterpret_cast<ValueType*>(new unsigned char[sizeof(ValueType)]); }
        inline ValueType* Allocate(USize n) { return reinterpret_cast<ValueType*>(new unsigned char[sizeof(ValueType) * n]); }
        
        inline void Deallocate(ValueType* ptr) { delete reinterpret_cast<unsigned char*>(ptr); }
        inline void Deallocate(ValueType* ptr, USize n) { delete[] reinterpret_cast<unsigned char*>(ptr); }

        template<typename... Args>
        inline void ConstructAt(ValueType* ptr, Args&&... args) { new (ptr) ValueType(::std::forward(args)...); }
        inline void DestructAt(ValueType* ptr) { ptr->~ValueType(); }
    };
};

