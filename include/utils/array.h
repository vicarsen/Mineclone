#pragma once

#include <vector>
#include <cstring>

#include "type.h"

namespace Utils
{
    template<typename Type, typename Allocator = Allocator<Type>>
    class Array
    {
    public:
        typedef Type ValueType;
        typedef Array<Type> ArrayType;
        typedef Allocator AllocatorType;

        constexpr Array() :
            size(0), capacity(0), buff(nullptr), allocator()
        {
        }

        constexpr Array(USize capacity) :
            size(0), capacity(capacity), allocator()
        {
            buff = allocator.Allocate(capacity);
        }

        constexpr Array(ArrayType&& other) :
            size(other.size), capacity(other.capacity), buff(other.buff), allocator(::std::move(other.allocator))
        {
            other.buff = nullptr;
        }

        constexpr Array(const ArrayType& other) :
            size(other.size), capacity(other.capacity), allocator(other.allocator)
        {
            buff = allocator.Allocate(capacity);
            for(USize i = 0; i < size; i++)
                allocator.ConstructAt(buff + i, *(other.buff + i));
        }

        constexpr ~Array()
        {
            Cleanup();
        }

        constexpr ArrayType& operator=(ArrayType&& other)
        { 
            Cleanup();

            size = other.size; capacity = other.capacity; allocator = ::std::move(other.allocator);
            buff = other.buff; other.buff = nullptr; 
        }
        constexpr ArrayType& operator=(const ArrayType& other)
        {
            Cleanup();

            size = other.size; capacity = other.capacity; allocator = other.allocator;
            buff = allocator.Allocate(capacity);
            for(USize i = 0; i < size; i++)
                allocator.ConstructAt(buff + i, *(other.buff + i));
        }

        constexpr void Reserve(USize capacity)
        {
            Resize(capacity);
        }
        constexpr void Shrink()
        {
            Resize(size);
        }

        template<typename... Args>
        constexpr ValueType& Push(Args&&... args)
        {
            if(size == capacity)
                Resize((capacity + 1) * 3 / 2);

            allocator.ConstructAt(size, ::std::forward(args)...);
            return *(buff + (size++));
        }

        constexpr void PushMany(USize n, const ValueType& value = ValueType())
        {
            if(size + n > capacity)
                Resize((size + n) * 3 / 2);

            for(USize i = 0; i < n; i++)
                allocator.ConstructAt(size++, value);
        }
        
        constexpr void Pop()
        {
            allocator.DestructAt(buff + (--size));
        }

        constexpr void PopMany(USize n)
        {
            for(USize i = 0; i < n; i++)
                allocator.DestructAt(buff + (--size));
        }
        
        constexpr void Clear()
        {
            for(USize i = 0; i < size; i++)
                allocator.DestructAt(buff + i);
            size = 0;
        }

        constexpr inline USize Size() const noexcept { return size; }
        constexpr inline USize Capacity() const noexcept { return capacity; }
        constexpr inline bool Empty() const noexcept { return size == 0; }

        constexpr inline ValueType* Data() noexcept { return buff; }
        constexpr inline const ValueType* Data() const noexcept { return buff; }

        constexpr inline ValueType& Front() noexcept { return *buff; }
        constexpr inline const ValueType& Front() const noexcept { return *buff; }

        constexpr inline ValueType& Back() noexcept { return *(buff + size - 1); }
        constexpr inline const ValueType& Back() const noexcept { return *(buff + size - 1); }

        constexpr inline ValueType& operator[](USize i) { return *(buff + i); }
        constexpr inline const ValueType& operator[](USize i) const { return *(buff + i); }

    private:
        constexpr inline void Cleanup()
        {
            if(buff != nullptr)
            {
                for(USize i = 0; i < size; i++)
                    allocator.DestructAt(buff + i);
                allocator.Deallocate(buff, capacity);
            }
        }

        constexpr inline void Resize(USize n)
        {
            ValueType* new_buff = allocator.Allocate(n);
            ::std::memcpy(new_buff, buff, size * sizeof(ValueType));
            allocator.Deallocate(buff);

            buff = new_buff;
            capacity = n;
        }

    private:
        USize size, capacity;
        ValueType* buff;
        AllocatorType allocator;
    };

    namespace __deprecated
    {
        template<typename Type>
        class Array
        {
        public:
            typedef Type ValueType;
            typedef Array<Type> ArrayType;

            Array() = default;
            Array(ArrayType&& other) = default;
            Array(const ArrayType& other) = default;

            Array(USize size, const ValueType& value = ValueType()) : array(size, value) {}

            ~Array() = default;

            ArrayType& operator=(ArrayType&& other) = default;
            ArrayType& operator=(const ArrayType& other) = default;

            inline void Reserve(USize size) { array.reserve(size); }
            inline void Resize(USize size, const ValueType& value = ValueType()) { array.assign(size, value); }

            inline void Push(const ValueType& value) { array.emplace_back(value); }
            inline void Push(ValueType&& value) { array.emplace_back(value); }
            inline void PushMany(USize n, const ValueType& value = ValueType()) 
            { 
                for(USize i = 0; i != n; i++)
                    array.emplace_back(value);
            }

            inline void Pop() { array.pop_back(); }
            inline void PopMany(USize n)
            {
                for(USize i = 0; i != n; i++)
                    array.pop_back();
            }

            inline void Clear() { array.clear(); }

            inline USize Size() const noexcept { return array.size(); }
            inline USize Capacity() const noexcept { return array.capacity(); }

            inline bool Empty() const noexcept { return array.empty(); }

            inline ValueType* Data() noexcept { return array.data(); }
            inline const ValueType* Data() const noexcept { return array.data(); }

            inline ValueType& operator[](USize i) { return array[i]; }
            inline const ValueType& operator[](USize i) const { return array[i]; }

        private:
            ::std::vector<ValueType> array;
        };
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
        __deprecated::Array<ValueType> array;
    };
};

