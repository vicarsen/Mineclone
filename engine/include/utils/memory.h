#pragma once

#include <memory>

namespace Utils
{
    template<typename Type>
    using SharedPointer = ::std::shared_ptr<Type>;

    template<typename Type>
    using WeakPointer = ::std::weak_ptr<Type>;

    template<typename Type>
    using UniquePointer = ::std::unique_ptr<Type>;

    template<typename Type, typename... Args>
    inline SharedPointer<Type> MakeShared(Args&&... args)
    {
        return ::std::make_shared<Type>(::std::forward<Args>(args)...);
    }

    template<typename Type, typename... Args>
    inline UniquePointer<Type> MakeUnique(Args&&... args)
    {
        return ::std::make_unique<Type>(::std::forward<Args>(args)...);
    }

    template<typename T, typename U>
    SharedPointer<T> StaticPointerCast(const SharedPointer<U>& r) noexcept
    {
        return ::std::static_pointer_cast<T, U>(r);
    }

    template<typename T, typename U>
    SharedPointer<T> StaticPointerCast(SharedPointer<U>&& r) noexcept
    {
        return ::std::static_pointer_cast<T, U>(r);
    }

    template<typename T, typename U>
    SharedPointer<T> DynamicPointerCast(const SharedPointer<U>& r) noexcept
    {
        return ::std::dynamic_pointer_cast<T, U>(r);
    }

    template<typename T, typename U>
    SharedPointer<T> DynamicPointerCast(SharedPointer<U>&& r) noexcept
    {
        return ::std::dynamic_pointer_cast<T, U>(r);
    }

    template<typename T, typename U>
    SharedPointer<T> ConstPointerCast(const SharedPointer<U>& r) noexcept
    {
        return ::std::const_pointer_cast<T, U>(r);
    }

    template<typename T, typename U>
    SharedPointer<T> ConstPointerCast(SharedPointer<U>&& r) noexcept
    {
        return ::std::const_pointer_cast<T, U>(r);
    }

    template<typename T, typename U>
    SharedPointer<T> ReinterpretPointerCast(const SharedPointer<U>& r) noexcept
    {
        return ::std::reinterpret_pointer_cast<T, U>(r);
    }

    template<typename T, typename U>
    SharedPointer<T> ReinterpretPointerCast(SharedPointer<U>&& r) noexcept
    {
        return ::std::reinterpret_pointer_cast<T, U>(r);
    }
};

