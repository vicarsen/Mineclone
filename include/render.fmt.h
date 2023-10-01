#pragma once

#include "render.h"

namespace fmt
{
    template<>
    struct formatter<Render::Vertex>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const Render::Vertex& vertex, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{encoded:{}}}", vertex.encoded);
        }
    };

    template<>
    struct formatter<Render::FaceMesh>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const Render::FaceMesh& face, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{v[0]:{},v[1]:{},v[2]:{},v[3]:{}}}", face.v[0], face.v[1], face.v[2], face.v[3]);
        }
    };

    template<>
    struct formatter<Render::FaceOrientation>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const Render::FaceOrientation& orientation, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(orientation)
            {
            case Render::FaceOrientation::XOY: return fmt::format_to(ctx.out(), "XOY");
            case Render::FaceOrientation::YOZ: return fmt::format_to(ctx.out(), "YOZ");
            case Render::FaceOrientation::ZOX: return fmt::format_to(ctx.out(), "ZOX");
            default: return fmt::format_to(ctx.out(), "");
            }
        }
    };
};

