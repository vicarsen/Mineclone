#pragma once

#include "utils/type.h"
#include "utils/hash.h"

#include <cstdint>
#include <string>
#include <algorithm>

namespace Utils
{
    using String = ::std::string;
    
    constexpr USize Length(const char* str)
    {
        USize size = 0;
        while(str[size] != '\0')
            size++;
        return size;
    }

    template<USize size>
    class StringLiteral
    {
    public:
        constexpr StringLiteral(const char (&str)[size])
        {
            ::std::copy_n(str, size, this->str);
        }

    public:
        char str[size];
    };

    class StringID
    {
    public:
        StringID() = default;
        StringID(StringID&& other) = default;
        StringID(const StringID& other) = default;

        inline StringID(USize ID) : ID(ID) {}
        inline StringID(const String& str) : ID(string_to_id[str]) {}

        ~StringID() = default;

        StringID& operator=(StringID&& other) = default;
        StringID& operator=(const StringID& other) = default;

        inline String ToString() const { return id_to_string[ID]; }

        inline bool operator==(const StringID& other) const noexcept { return ID == other.ID; }
        inline bool operator!=(const StringID& other) const noexcept { return ID != other.ID; }
        inline bool operator==(USize other) const noexcept { return ID == other; }
        inline bool operator!=(USize other) const noexcept { return ID != other; }

        inline operator USize() const noexcept { return ID; }

        static inline StringID Register(const String& str)
        {
            static USize ID = 1;
            string_to_id[str] = ID;
            id_to_string[ID] = str;
            return ID++;
        }

    private:
        USize ID;

        inline static HashMap<String, USize> string_to_id;
        inline static HashMap<USize, String> id_to_string;
    };

    template<StringLiteral literal>
    struct StringIdentifier
    {
        inline static StringID Get()
        {
            static StringID ID = StringID::Register(literal.str);
            return ID;
        }
    };

    template<StringLiteral literal>
    inline StringID StringIDOf()
    {
        return StringIdentifier<literal>::Get();
    }
};

namespace std
{
    template<>
    struct hash<::Utils::StringID>
    {
        uint64_t operator()(const ::Utils::StringID& string_id) const
        {
            return hash<::Utils::USize>()(string_id);
        }
    };
};

