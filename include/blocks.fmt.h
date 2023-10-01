#pragma once

#include "blocks.h"

namespace fmt
{
    template<>
    struct formatter<Game::Direction>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const Game::Direction& direction, FormatContext& ctx) -> decltype(ctx.out())
        {
            switch(direction)
            {
            case Game::Direction::NORTH: return fmt::format_to(ctx.out(), "NORTH");
            case Game::Direction::SOUTH: return fmt::format_to(ctx.out(), "SOUTH");
            case Game::Direction::EAST: return fmt::format_to(ctx.out(), "EAST");
            case Game::Direction::WEST: return fmt::format_to(ctx.out(), "WEST");
            case Game::Direction::UP: return fmt::format_to(ctx.out(), "UP");
            case Game::Direction::DOWN: return fmt::format_to(ctx.out(), "DOWN");
            }
        }
    };

    template<>
    struct formatter<Game::BlockMetadata>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const Game::BlockMetadata& metadata, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{face_north:{},face_south:{},face_east:{},face_west:{},face_up:{},face_down:{}}}", metadata.face_north, metadata.face_south, metadata.face_east, metadata.face_west, metadata.face_up, metadata.face_down);
        }
    };

    template<>
    struct formatter<Game::Face>
    {
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
        {
            return ctx.end();
        }

        template<typename FormatContext>
        auto format(const Game::Face& face, FormatContext& ctx) -> decltype(ctx.out())
        {
            return fmt::format_to(ctx.out(), "{{block:{},direction:{}}}", face.block, face.direction);
        }
    };
};

