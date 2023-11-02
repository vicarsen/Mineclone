#pragma once

#include "../logger.h"

#include <glm/glm.hpp>

namespace fmt
{
    template<typename Type>
    struct formatter<::glm::tvec2<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::glm::tvec2<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{x:{},y:{}}}", input.x, input.y);
        }
    };

    template<typename Type>
    struct formatter<::glm::tvec3<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::glm::tvec3<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{x:{},y:{},z:{}}}", input.x, input.y, input.z);
        }
    };

    template<typename Type>
    struct formatter<::glm::tvec4<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::glm::tvec4<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{x:{},y:{},z:{},w:{}}}", input.x, input.y, input.z, input.w);
        }
    };

    template<typename Type>
    struct formatter<::glm::tmat4x4<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::glm::tmat4x4<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{v[0]:{},v[1]:{},v[2]:{},v[3]:{}}}", input[0], input[1], input[2], input[3]);
        }
    };

    template<typename Type>
    struct formatter<::glm::tquat<Type>>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::glm::tquat<Type>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{w:{},x:{},y:{},z:{}}}", input.w, input.x, input.y, input.z);
        }
    };
};

