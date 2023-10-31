#pragma once

#include "../logger.h"

#include "../blocks.h"

namespace fmt
{
    template<>
    struct formatter<::Game::Direction>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Game::Direction& direction, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(direction)
            {
            case ::Game::Direction::NORTH: return ::fmt::format_to(ctx.out(), "NORTH");
            case ::Game::Direction::SOUTH: return ::fmt::format_to(ctx.out(), "SOUTH");
            case ::Game::Direction::EAST:  return ::fmt::format_to(ctx.out(), "EAST");
            case ::Game::Direction::WEST:  return ::fmt::format_to(ctx.out(), "WEST");
            case ::Game::Direction::UP:    return ::fmt::format_to(ctx.out(), "UP");
            case ::Game::Direction::DOWN:  return ::fmt::format_to(ctx.out(), "DOWN");
            }
        }
    };

    template<>
    struct formatter<::Game::BlockMetadata>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Game::BlockMetadata& metadata, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{face_north:{},face_south:{},face_east:{},face_west:{},face_up:{},face_down:{}}}", metadata.face_north, metadata.face_south, metadata.face_east, metadata.face_west, metadata.face_up, metadata.face_down);
        }
    };

    template<>
    struct formatter<::Game::Face>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Game::Face& face, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{block:{},direction:{}}}", face.block, face.direction);
        }
    };

    template<>
    struct formatter<::Game::__detail::BlockData>
    {
        template<typename FormatParseContext>
        constexpr auto parse(FormatParseContext& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const ::Game::__detail::BlockData& block_data, FormatContext& ctx) -> decltype(ctx.out())
        {
            return ::fmt::format_to(ctx.out(), "{{name:{},metadata:{},render_attachment:{}}}", block_data.name, block_data.metadata, block_data.render_attachment);
        }
    };
};

