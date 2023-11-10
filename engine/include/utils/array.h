#pragma once

#include <vector>
#include <cstring>

#include "type.h"

namespace Utils
{
    namespace __detail
    {
        namespace __iterator
        {
            template<typename Type>
            class ArrayConstIterator
            {
            public:
                typedef Type ValueType;
                typedef ArrayConstIterator<Type> ArrayConstIteratorType;

                ALWAYS_INLINE constexpr ArrayConstIterator(const ValueType* v) noexcept : v(v) {}
                ALWAYS_INLINE constexpr ArrayConstIterator(ArrayConstIteratorType&& other) noexcept = default;
                ALWAYS_INLINE constexpr ArrayConstIterator(const ArrayConstIteratorType& other) noexcept = default;

                ALWAYS_INLINE constexpr ~ArrayConstIterator() noexcept = default;

                ALWAYS_INLINE constexpr ArrayConstIteratorType& operator=(ArrayConstIteratorType&& other) noexcept = default;
                ALWAYS_INLINE constexpr ArrayConstIteratorType& operator=(const ArrayConstIteratorType& other) noexcept = default;

                ALWAYS_INLINE constexpr ArrayConstIteratorType& operator+=(USize dt) noexcept { v += dt; return *this; }
                ALWAYS_INLINE constexpr ArrayConstIteratorType& operator-=(USize dt) noexcept { v -= dt; return *this; }

                ALWAYS_INLINE constexpr ArrayConstIteratorType& operator++() noexcept { ++v; return *this; }
                ALWAYS_INLINE constexpr ArrayConstIteratorType operator++(int) noexcept { ArrayConstIteratorType copy(*this); v++; return copy; }

                ALWAYS_INLINE constexpr ArrayConstIteratorType& operator--() noexcept { --v; return *this; }
                ALWAYS_INLINE constexpr ArrayConstIteratorType operator--(int) noexcept { ArrayConstIteratorType copy(*this); v--; return copy; }

                ALWAYS_INLINE constexpr ArrayConstIteratorType operator+(USize dt) const noexcept { return ArrayIteratorType(v + dt); }
                ALWAYS_INLINE constexpr ArrayConstIteratorType operator-(USize dt) const noexcept { return ArrayIteratorType(v - dt); }

                ALWAYS_INLINE constexpr USize operator-(const ArrayConstIteratorType& other) const noexcept { return v - other.v; }

                ALWAYS_INLINE constexpr const ValueType* operator->() const noexcept { return v; }
                ALWAYS_INLINE constexpr const ValueType& operator*() const noexcept { return *v; }

                ALWAYS_INLINE constexpr const ValueType& operator[](USize i) const noexcept { return v[i]; }

                ALWAYS_INLINE constexpr operator const ValueType*() const noexcept { return v; }

                ALWAYS_INLINE constexpr bool operator==(const ArrayConstIteratorType& other) const noexcept { return v == other.v; }
                ALWAYS_INLINE constexpr bool operator!=(const ArrayConstIteratorType& other) const noexcept { return v != other.v; }

                ALWAYS_INLINE constexpr bool operator<(const ArrayConstIteratorType& other) const noexcept { return v < other.v; }
                ALWAYS_INLINE constexpr bool operator>(const ArrayConstIteratorType& other) const noexcept { return v > other.v; }
                ALWAYS_INLINE constexpr bool operator<=(const ArrayConstIteratorType& other) const noexcept { return v <= other.v; }
                ALWAYS_INLINE constexpr bool operator>=(const ArrayConstIteratorType& other) const noexcept { return v >= other.v; }

            public:
                const ValueType* v;
            };

            template<typename Type>
            class ArrayIterator
            {
            public:
                typedef Type ValueType;
                typedef ArrayIterator<Type> ArrayIteratorType;
                typedef ArrayConstIterator<Type> ArrayConstIteratorType;

                ALWAYS_INLINE constexpr ArrayIterator(ValueType* v) noexcept : v(v) {}
                ALWAYS_INLINE constexpr ArrayIterator(ArrayIteratorType&& other) noexcept = default;
                ALWAYS_INLINE constexpr ArrayIterator(const ArrayIteratorType& other) noexcept = default;

                ALWAYS_INLINE constexpr ~ArrayIterator() noexcept = default;

                ALWAYS_INLINE constexpr ArrayIteratorType& operator=(ArrayIteratorType&& other) noexcept = default;
                ALWAYS_INLINE constexpr ArrayIteratorType& operator=(const ArrayIteratorType& other) noexcept = default;

                ALWAYS_INLINE constexpr ArrayIteratorType& operator+=(USize dt) noexcept { v += dt; return *this; }
                ALWAYS_INLINE constexpr ArrayIteratorType& operator-=(USize dt) noexcept { v -= dt; return *this; }

                ALWAYS_INLINE constexpr ArrayIteratorType& operator++() noexcept { ++v; return *this; }
                ALWAYS_INLINE constexpr ArrayIteratorType operator++(int) noexcept { ArrayIteratorType copy(*this); v++; return copy; }

                ALWAYS_INLINE constexpr ArrayIteratorType& operator--() noexcept { --v; return *this; }
                ALWAYS_INLINE constexpr ArrayIteratorType operator--(int) noexcept { ArrayIteratorType copy(*this); v--; return copy; }

                ALWAYS_INLINE constexpr ArrayIteratorType operator+(USize dt) const noexcept { return ArrayIteratorType(v + dt); }
                ALWAYS_INLINE constexpr ArrayIteratorType operator-(USize dt) const noexcept { return ArrayIteratorType(v - dt); }

                ALWAYS_INLINE constexpr USize operator-(const ArrayIteratorType& other) const noexcept { return v - other.v; }

                ALWAYS_INLINE constexpr ValueType* operator->() const noexcept { return v;; }
                ALWAYS_INLINE constexpr ValueType& operator*() const noexcept { return *v; }

                ALWAYS_INLINE constexpr ValueType& operator[](USize i) const noexcept { return v[i]; }

                ALWAYS_INLINE constexpr operator ValueType*() const noexcept { return v; }

                ALWAYS_INLINE constexpr bool operator==(const ArrayIteratorType& other) const noexcept { return v == other.v; }
                ALWAYS_INLINE constexpr bool operator!=(const ArrayIteratorType& other) const noexcept { return v != other.v; }

                ALWAYS_INLINE constexpr bool operator<(const ArrayIteratorType& other) const noexcept { return v < other.v; }
                ALWAYS_INLINE constexpr bool operator>(const ArrayIteratorType& other) const noexcept { return v > other.v; }
                ALWAYS_INLINE constexpr bool operator<=(const ArrayIteratorType& other) const noexcept { return v <= other.v; }
                ALWAYS_INLINE constexpr bool operator>=(const ArrayIteratorType& other) const noexcept { return v >= other.v; }

            public:
                ValueType* v;
            };

            template<typename Type>
            class ArrayConstReverseIterator
            {
            public:
                typedef Type ValueType;
                typedef ArrayConstReverseIterator<Type> ArrayConstReverseIteratorType;
                typedef ArrayConstIterator<Type> ArrayConstIteratorType;

                ALWAYS_INLINE constexpr ArrayConstReverseIterator(const ValueType* v) noexcept : v(v) {}
                ALWAYS_INLINE constexpr ArrayConstReverseIterator(ArrayConstReverseIteratorType&& other) noexcept = default;
                ALWAYS_INLINE constexpr ArrayConstReverseIterator(const ArrayConstReverseIteratorType& other) noexcept = default;

                ALWAYS_INLINE constexpr ArrayConstReverseIterator() noexcept = default;

                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType& operator=(ArrayConstReverseIteratorType&& other) noexcept = default;
                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType& operator=(const ArrayConstReverseIteratorType& other) noexcept = default;

                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType& operator+=(USize dt) noexcept { v -= dt; return *this; }
                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType& operator-=(USize dt) noexcept { v += dt; return *this; }

                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType& operator++() noexcept { --v; return *this; }
                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType operator++(int) noexcept { ArrayConstReverseIteratorType copy(*this); v--; return copy; }

                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType& operator--() noexcept { ++v; return *this; }
                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType operator--(int) noexcept { ArrayConstReverseIteratorType copy(*this); v++; return copy; }

                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType operator+(USize dt) const noexcept { return ArrayConstReverseIteratorType(v - dt); }
                ALWAYS_INLINE constexpr ArrayConstReverseIteratorType operator-(USize dt) const noexcept { return ArrayConstReverseIteratorType(v + dt); }

                ALWAYS_INLINE constexpr USize operator-(const ArrayConstReverseIteratorType& other) const noexcept { return v - other.v; }

                ALWAYS_INLINE constexpr const ValueType* operator->() const noexcept { return v; }
                ALWAYS_INLINE constexpr const ValueType& operator*() const noexcept { return *v; }

                ALWAYS_INLINE constexpr const ValueType& operator[](USize i) const noexcept { return v[i]; }

                ALWAYS_INLINE constexpr operator const ValueType*() const noexcept { return v; }

                ALWAYS_INLINE constexpr bool operator==(const ArrayConstReverseIteratorType& other) const noexcept { return v == other.v; }
                ALWAYS_INLINE constexpr bool operator!=(const ArrayConstReverseIteratorType& other) const noexcept { return v != other.v; }

                ALWAYS_INLINE constexpr bool operator<(const ArrayConstReverseIteratorType& other) const noexcept { return v < other.v; }
                ALWAYS_INLINE constexpr bool operator>(const ArrayConstReverseIteratorType& other) const noexcept { return v > other.v; }
                ALWAYS_INLINE constexpr bool operator<=(const ArrayConstReverseIteratorType& other) const noexcept { return v <= other.v; }
                ALWAYS_INLINE constexpr bool operator>=(const ArrayConstReverseIteratorType& other) const noexcept { return v >= other.v; }

            public:
                const ValueType* v;
            };

            template<typename Type>
            class ArrayReverseIterator
            {
            public:
                typedef Type ValueType;
                typedef ArrayReverseIterator<Type> ArrayReverseIteratorType;

                ALWAYS_INLINE constexpr ArrayReverseIterator(ValueType* v) noexcept : v(v) {}
                ALWAYS_INLINE constexpr ArrayReverseIterator(ArrayReverseIteratorType&& other) noexcept = default;
                ALWAYS_INLINE constexpr ArrayReverseIterator(const ArrayReverseIteratorType& other) noexcept = default;

                ALWAYS_INLINE constexpr ArrayReverseIterator() noexcept = default;

                ALWAYS_INLINE constexpr ArrayReverseIteratorType& operator=(ArrayReverseIteratorType&& other) noexcept = default;
                ALWAYS_INLINE constexpr ArrayReverseIteratorType& operator=(const ArrayReverseIteratorType& other) noexcept = default;

                ALWAYS_INLINE constexpr ArrayReverseIteratorType& operator+=(USize dt) noexcept { v -= dt; return *this; }
                ALWAYS_INLINE constexpr ArrayReverseIteratorType& operator-=(USize dt) noexcept { v += dt; return *this; }

                ALWAYS_INLINE constexpr ArrayReverseIteratorType& operator++() noexcept { --v; return *this; }
                ALWAYS_INLINE constexpr ArrayReverseIteratorType operator++(int) noexcept { ArrayReverseIteratorType copy(*this); v--; return copy; }

                ALWAYS_INLINE constexpr ArrayReverseIteratorType& operator--() noexcept { ++v; return *this; }
                ALWAYS_INLINE constexpr ArrayReverseIteratorType operator--(int) noexcept { ArrayReverseIteratorType copy(*this); v++; return copy; }

                ALWAYS_INLINE constexpr ArrayReverseIteratorType operator+(USize dt) const noexcept { return ArrayConstReverseIteratorType(v - dt); }
                ALWAYS_INLINE constexpr ArrayReverseIteratorType operator-(USize dt) const noexcept { return ArrayConstReverseIteratorType(v + dt); }

                ALWAYS_INLINE constexpr USize operator-(const ArrayReverseIteratorType& other) const noexcept { return v - other.v; }

                ALWAYS_INLINE constexpr ValueType* operator->() const noexcept { return v; }
                ALWAYS_INLINE constexpr ValueType& operator*() const noexcept { return *v; }

                ALWAYS_INLINE constexpr ValueType& operator[](USize i) const noexcept { return v[i]; }

                ALWAYS_INLINE constexpr operator ValueType*() const noexcept { return v; }

                ALWAYS_INLINE constexpr bool operator==(const ArrayReverseIteratorType& other) const noexcept { return v == other.v; }
                ALWAYS_INLINE constexpr bool operator!=(const ArrayReverseIteratorType& other) const noexcept { return v != other.v; }

                ALWAYS_INLINE constexpr bool operator<(const ArrayReverseIteratorType& other) const noexcept { return v < other.v; }
                ALWAYS_INLINE constexpr bool operator>(const ArrayReverseIteratorType& other) const noexcept { return v > other.v; }
                ALWAYS_INLINE constexpr bool operator<=(const ArrayReverseIteratorType& other) const noexcept { return v <= other.v; }
                ALWAYS_INLINE constexpr bool operator>=(const ArrayReverseIteratorType& other) const noexcept { return v >= other.v; }

            public:
                ValueType* v;
            };
        };
    };

    template<typename Type, typename Allocator = Allocator<Type>>
    class Array
    {
    public:
        typedef Type ValueType;
        typedef Array<Type> ArrayType;
        typedef Allocator AllocatorType;

        typedef __detail::__iterator::ArrayIterator<Type> Iterator;
        typedef __detail::__iterator::ArrayConstIterator<Type> ConstIterator;
        typedef __detail::__iterator::ArrayReverseIterator<Type> ReverseIterator;
        typedef __detail::__iterator::ArrayConstReverseIterator<Type> ConstReverseIterator;

        constexpr Array() noexcept :
            start(nullptr), finish(nullptr), alloc_finish(nullptr)
        {
        }

        constexpr Array(USize capacity) noexcept
        {
            start = AllocatorType::Allocate(capacity);
            finish = start;
            alloc_finish = start + capacity;
        }

        constexpr Array(ArrayType&& other) noexcept :
            start(other.start), finish(other.finish), alloc_finish(other.alloc_finish)
        {
            other.start = nullptr;
            other.finish = nullptr;
            other.alloc_finish = nullptr;
        }

        constexpr Array(const ArrayType& other) noexcept
        {
            USize size = other.finish - other.start, capacity = other.alloc_finish - other.start;
            start = AllocatorType::Allocate(capacity);
            finish = start + size;
            alloc_finish = start + capacity;
            for(USize i = 0; i < size; i++)
                AllocatorType::ConstructAt(start + i, *(other.start + i));
        }

        constexpr ~Array() noexcept
        {
            Cleanup();
        }

        constexpr ArrayType& operator=(ArrayType&& other) noexcept
        { 
            Cleanup();

            start = other.start; other.start = nullptr;
            finish = other.finish; other.finish = nullptr;
            alloc_finish = other.alloc_finish; other.alloc_finish = nullptr;

            return *this;
        }

        constexpr ArrayType& operator=(const ArrayType& other) noexcept
        {
            Cleanup();
            
            USize size = other.finish - other.start, capacity = other.alloc_finish - other.start;
            start = AllocatorType::Allocate(capacity);
            finish = start + size;
            alloc_finish = start + capacity;
            for(USize i = 0; i < size; i++)
                AllocatorType::ConstructAt(start + i, *(other.start + i));
            
            return *this;
        }

        constexpr void Reserve(USize capacity) noexcept
        {
            Resize(capacity);
        }

        constexpr void Shrink() noexcept
        {
            Resize(finish - start);
        }

        template<typename... Args>
        constexpr ValueType& Push(Args&&... args) noexcept
        {
            if(finish == alloc_finish)
                Resize((finish - start + 1) * 3 / 2);

            AllocatorType::ConstructAt(finish, ::std::forward<Args>(args)...);
            return *(finish++);
        }

        constexpr void PushMany(USize n, const ValueType& value = ValueType()) noexcept
        {
            if(finish + n > alloc_finish)
                Resize(((finish - start) + n) * 3 / 2);

            ValueType* new_finish = finish + n;
            while(finish != new_finish)
                AllocatorType::ConstructAt(finish++, value);
        }
        
        constexpr void Pop() noexcept
        {
            AllocatorType::DestructAt(--finish);
        }

        constexpr void PopMany(USize n) noexcept
        {
            USize new_finish = finish - n;
            while(finish != new_finish)
                AllocatorType::DestructAt(--finish);
        }
        
        constexpr void Clear() noexcept
        {
            while(finish != start)
                AllocatorType::DestructAt(--finish);
        }

        ALWAYS_INLINE constexpr Iterator Begin() noexcept { return Iterator(start); }
        ALWAYS_INLINE constexpr Iterator End() noexcept { return Iterator(finish); }

        ALWAYS_INLINE constexpr ConstIterator CBegin() const noexcept { return ConstIterator(start); }
        ALWAYS_INLINE constexpr ConstIterator CEnd() const noexcept { return ConstIterator(finish); }

        ALWAYS_INLINE constexpr ReverseIterator RBegin() const noexcept { return ReverseIterator(finish - 1); }
        ALWAYS_INLINE constexpr ReverseIterator REnd() const noexcept { return ReverseIterator(start - 1); }

        ALWAYS_INLINE constexpr ConstReverseIterator CRBegin() const noexcept { return ConstReverseIterator(finish - 1); }
        ALWAYS_INLINE constexpr ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(start - 1); }

        ALWAYS_INLINE constexpr USize Size() const noexcept { return finish - start; }
        ALWAYS_INLINE constexpr USize Capacity() const noexcept { return alloc_finish - start; }
        ALWAYS_INLINE constexpr bool Empty() const noexcept { return finish == start; }

        ALWAYS_INLINE constexpr ValueType* Data() noexcept { return start; }
        ALWAYS_INLINE constexpr const ValueType* Data() const noexcept { return start; }

        ALWAYS_INLINE constexpr ValueType& Front() noexcept { return *start; }
        ALWAYS_INLINE constexpr const ValueType& Front() const noexcept { return *start; }

        ALWAYS_INLINE constexpr ValueType& Back() noexcept { return *(finish - 1); }
        ALWAYS_INLINE constexpr const ValueType& Back() const noexcept { return *(finish - 1); }

        ALWAYS_INLINE constexpr ValueType& operator[](USize i) { return *(start + i); }
        ALWAYS_INLINE constexpr const ValueType& operator[](USize i) const { return *(start + i); }

        // for C++ range for
        ALWAYS_INLINE constexpr Iterator begin() noexcept { return Iterator(start); }
        ALWAYS_INLINE constexpr Iterator end() noexcept { return Iterator(finish); }

        ALWAYS_INLINE constexpr ConstIterator begin() const noexcept { return ConstIterator(start); }
        ALWAYS_INLINE constexpr ConstIterator end() const noexcept { return ConstIterator(finish); }

    private:
        ALWAYS_INLINE constexpr void Cleanup()
        {
            if(start != nullptr)
            {
                Clear();
                AllocatorType::Deallocate(start, alloc_finish - start);
            }
        }

        ALWAYS_INLINE constexpr void Resize(USize n)
        {
            USize size = finish - start;
            ValueType* new_start = AllocatorType::Allocate(n);

            if(start != nullptr)
            {
                for(USize i = 0; i < size; i++)
                {
                    AllocatorType::ConstructAt(new_start + i, ::std::move(*(start + i)));
                    AllocatorType::DestructAt(start + i);
                }
                AllocatorType::Deallocate(start, alloc_finish - start);
            }

            start = new_start;
            finish = new_start + size;
            alloc_finish = new_start + n;
        }

    private:
        ValueType* start;
        ValueType* finish;
        ValueType* alloc_finish;
    };

    template<typename Type>
    class CircularArray
    {
    public:
        typedef Type ValueType;
        typedef CircularArray<Type> CircularArrayType;

        CircularArray() :
            offset(0), array()
        {
        }
        
        CircularArray(CircularArrayType&& other) = default;
        CircularArray(const CircularArrayType& other) = default;

        CircularArray(USize max_size) : 
            offset(0)
        { 
            array.Reserve(max_size); 
        }
        
        ~CircularArray() = default;

        CircularArrayType& operator=(CircularArrayType&& other) = default;
        CircularArrayType& operator=(const CircularArrayType& other) = default;

        void Reset(USize max_size)
        {
            offset = 0;
            array.Clear();
            array.Reserve(max_size);
        }

        void Push(const ValueType& value)
        {
            if(array.Size() != array.Capacity())
                array.Push(value);
            else
            {
                array[offset++] = value;
                if(offset == array.Capacity())
                    offset = 0;
            }
        }

        void PushMany(USize n, const ValueType& value = ValueType())
        {
            for(USize i = 0; i != n; i++)
                Push(value);
        }

        void Push(ValueType&& value)
        {
            if(array.Size() != array.Capacity())
                array.Push(value);
            else
            {
                array[offset++] = value;
                if(offset == array.Capacity())
                    offset = 0;
            }
        }

        void Clear() 
        { 
            offset = 0; 
            array.Clear(); 
        }

        inline USize Size() const noexcept { return array.Size(); }
        inline USize Capacity() const noexcept { return array.Capacity(); }
        inline USize Offset() const noexcept { return offset; }

        inline bool Empty() const noexcept { return array.Empty(); }
        inline bool Full() const noexcept { return array.Size() == array.Capacity(); }

        inline ValueType* Data() noexcept { return array.data(); }
        inline const ValueType* Data() const noexcept { return array.data(); }

        inline ValueType& operator[](USize i) { return array[i]; }
        inline const ValueType& operator[](USize i) const { return array[i]; }

    private:
        USize offset;
        Array<ValueType> array;
    };
};

