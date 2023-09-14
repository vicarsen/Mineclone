#pragma once

#include <spdlog/fmt/bundled/format.h>
#include <glm/glm.hpp>

namespace fmt
{
    template<typename Tp>
    struct formatter<glm::tvec2<Tp>>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const glm::tvec2<Tp>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{x:{}, y:{}}}", input.x, input.y);
        }
    };

    template<typename Tp>
    struct formatter<glm::tvec3<Tp>>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const glm::tvec3<Tp>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{x:{}, y:{}, z:{}}}", input.x, input.y, input.z);
        }
    };

    template<typename Tp>
    struct formatter<glm::tvec4<Tp>>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const glm::tvec4<Tp>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{x:{}, y:{}, z:{}, w:{}}}", input.x, input.y, input.z, input.w);
        }
    };

    template<typename Tp>
    struct formatter<glm::tmat4x4<Tp>>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const glm::tmat4x4<Tp>& input, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{v[0]:{}, v[1]:{}, v[2]:{}, v[3]:{}}}", input[0], input[1], input[2], input[3]);
        }
    };
};

