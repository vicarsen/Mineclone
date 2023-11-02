#pragma once

#include "../logger.h"

#include "../render.h"

namespace fmt
{
    template<>
    struct formatter<::Render::Vertex>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Render::Vertex& vertex, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{encoded:{}}}", vertex.encoded);
        }
    };

    template<>
    struct formatter<::Render::FaceMesh>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Render::FaceMesh& face_mesh, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{a:{},b:{},c:{},d:{}}}", face_mesh.v[0], face_mesh.v[1], face_mesh.v[2], face_mesh.v[3]);
        }
    };

    template<>
    struct formatter<::Render::FaceOrientation>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Render::FaceOrientation& face_orientation, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(face_orientation)
            {
            case ::Render::FaceOrientation::XOY: return ::fmt::format_to(ctx.out(), "XOY");
            case ::Render::FaceOrientation::YOZ: return ::fmt::format_to(ctx.out(), "YOZ");
            case ::Render::FaceOrientation::ZOX: return ::fmt::format_to(ctx.out(), "ZOX");
            }
        }
    };

    template<>
    struct formatter<::Render::TextureStyle>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Render::TextureStyle& texture_style, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(texture_style)
            {
            case ::Render::TextureStyle::PIXELATED: return ::fmt::format_to(ctx.out(), "PIXELATED");
            case ::Render::TextureStyle::SMOOTH: return ::fmt::format_to(ctx.out(), "SMOOTH");
            }
        }
    };

    template<>
    struct formatter<::Render::TextureFormatType>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Render::TextureFormatType& texture_format_type, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(texture_format_type)
            {
            case ::Render::TextureFormatType::RED: return ::fmt::format_to(ctx.out(), "RED");
            case ::Render::TextureFormatType::RGB: return ::fmt::format_to(ctx.out(), "RGB");
            case ::Render::TextureFormatType::RGBA: return ::fmt::format_to(ctx.out(), "RGBA");
            }
        }
    };

    template<>
    struct formatter<::Render::TextureFormat>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Render::TextureFormat& texture_format, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(texture_format)
            {
            case ::Render::TextureFormat::R8: return ::fmt::format_to(ctx.out(), "R8");
            case ::Render::TextureFormat::R32: return ::fmt::format_to(ctx.out(), "R32");
            case ::Render::TextureFormat::RGB8: return ::fmt::format_to(ctx.out(), "RGB8");
            case ::Render::TextureFormat::RGB32: return ::fmt::format_to(ctx.out(), "RGB32");
            case ::Render::TextureFormat::RGBA8: return ::fmt::format_to(ctx.out(), "RGBA8");
            case ::Render::TextureFormat::RGBA32: return ::fmt::format_to(ctx.out(), "RGBA32");
            }
        }
    };

    template<>
    struct formatter<::Render::ChunkRenderer::DrawMode>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Render::ChunkRenderer::DrawMode& draw_mode, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(draw_mode)
            {
            case ::Render::ChunkRenderer::DrawMode::NORMAL: return ::fmt::format_to(ctx.out(), "NORMAL");
            case ::Render::ChunkRenderer::DrawMode::WIREFRAME: return ::fmt::format_to(ctx.out(), "WIREFRAME");
            }
        }
    };

    template<>
    struct formatter<::Render::FramebufferEvent>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Render::FramebufferEvent& framebuffer_event, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{x:{},y:{},width:{},height:{}}}", framebuffer_event.x, framebuffer_event.y, framebuffer_event.width, framebuffer_event.height);
        }
    };
};

