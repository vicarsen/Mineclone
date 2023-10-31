#pragma once

#include "../logger.h"

#include "../window.h"

namespace fmt
{
    template<>
    struct formatter<::Window::Window>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Window::Window& window, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{width:{},height:{},title:{}}}", window.GetWidth(), window.GetHeight(), window.GetTitle());
        }
    };
};

