#pragma once

#include "../logger.h"

#include "../player.h"

namespace fmt
{
    template<>
    struct formatter<::Game::Player>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Game::Player& player, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{transform:{},projection:{},fov:{},aspect_ratio:{},near:{},fa:{},render_distance:{}}}", player.GetTransform(), player.GetProjection(), player.GetFOV(), player.GetAspectRatio(), player.GetNear(), player.GetFar(), player.GetRenderDistance());
        }
    };
};

