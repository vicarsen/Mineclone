#pragma once

#include "../logger.h"

#include "../mathematics.h"

namespace fmt
{
    template<typename Type>
    struct formatter<::Math::tvec2<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Math::tvec2<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{x:{},y:{}}}", input.x, input.y);
        }
    };

    template<typename Type>
    struct formatter<::Math::tvec3<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Math::tvec3<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{x:{},y:{},z:{}}}", input.x, input.y, input.z);
        }
    };

    template<typename Type>
    struct formatter<::Math::tvec4<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Math::tvec4<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{x:{},y:{},z:{},w:{}}}", input.x, input.y, input.z, input.w);
        }
    };

    template<typename Type>
    struct formatter<::Math::tmat4x4<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Math::tmat4x4<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{v[0]:{},v[1]:{},v[2]:{},v[3]:{}}}", input[0], input[1], input[2], input[3]);
        }
    };

    template<typename Type>
    struct formatter<::Math::tquat<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Math::tquat<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{w:{},x:{},y:{},z:{}}}", input.w, input.x, input.y, input.z);
        }
    };
};

