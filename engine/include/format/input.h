#pragma once

#include "../logger.h"

#include "../input.h"

namespace fmt
{
    template<>
    struct formatter<::Input::InputAction>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Input::InputAction& input_action, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(input_action)
            {
            case ::Input::InputAction::NONE: return ::fmt::format_to(ctx.out(), "NONE");
            case ::Input::InputAction::PRESSED: return ::fmt::format_to(ctx.out(), "PRESSED");
            case ::Input::InputAction::RELEASED: return ::fmt::format_to(ctx.out(), "RELEASED");
            }
        }
    };

    template<>
    struct formatter<::Input::CursorMode>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Input::CursorMode& cursor_mode, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(cursor_mode)
            {
            case ::Input::CursorMode::DISABLED: return ::fmt::format_to(ctx.out(), "DISABLED");
            case ::Input::CursorMode::ENABLED: return ::fmt::format_to(ctx.out(), "ENABLED");
            }
        }
    };

    template<>
    struct formatter<::Input::KeyEvent>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Input::KeyEvent& key_event, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{key:{},action:{}}}", key_event.key, key_event.action);
        }
    };

    template<>
    struct formatter<::Input::CursorMovedEvent>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Input::CursorMovedEvent& cursor_moved_event, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{position:{}}}", cursor_moved_event.position);
        }
    };
};

