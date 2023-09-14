#pragma once

#include "logger.h"

DECLARE_LOG_CATEGORY(BlockBase);

namespace Game
{
    typedef unsigned char BlockID;

    enum class Direction { UP, DOWN, NORTH, SOUTH, EAST, WEST };

    class Face
    {
    public:
        Face() = default;
        ~Face() = default;

        inline unsigned int GetBlock() const noexcept { return block; }
        inline void SetBlock(BlockID _block) noexcept { block = _block; }

        inline Direction GetDirection() const noexcept { return direction; }
        inline void SetDirection(Direction _direction) noexcept { direction = _direction; }

    private:
        BlockID block;
        Direction direction;
    };

    class BlockMetadata
    {
    public:
        BlockMetadata() = default;
        ~BlockMetadata() = default;

        inline void SetTransparent(bool _transparent) noexcept { transparent = _transparent; }
        inline bool IsTransparent() const noexcept { return transparent; }

        void SetFaces(const std::string& path);
        void SetFaces(const std::string& north, const std::string& south, const std::string& east, const std::string& west, const std::string& up, const std::string& down);

        inline const std::string& GetNorthFace() const noexcept { return north; }
        inline const std::string& GetSouthFace() const noexcept { return south; }
        inline const std::string& GetEastFace() const noexcept { return east; }
        inline const std::string& GetWestFace() const noexcept { return west; }
        inline const std::string& GetUpFace() const noexcept { return up; }
        inline const std::string& GetDownFace() const noexcept { return down; }

    private:
        bool transparent = false;
        std::string north, south, east, west, up, down;
    };

    class BlockBase
    {
    public:
        BlockBase() = delete;
        
        static BlockID RegisterBlock(const std::string& name, const BlockMetadata& metadata);
        static Face GetBlockFace(BlockID block, Direction face);
        static const std::string& GetFaceTexture(BlockID block, Direction face);

        static inline bool IsBlockTransparent(BlockID block) { return blocks[block].transparent; }

    private:
        struct BlockData
        {
            std::string name;
            bool transparent;
            std::string north, south, east, west, up, down;
        };

    private:
        static std::vector<BlockData> blocks;
    };

    namespace VanillaBlocks
    {
        extern BlockID AIR_BLOCK;
        extern BlockID BEDROCK_BLOCK;
        extern BlockID DIRT_BLOCK;
        extern BlockID GRASS_BLOCK;
    };
};

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
            return fmt::format_to(ctx.out(), "{{transparent:{}}}", metadata.IsTransparent());
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
            return fmt::format_to(ctx.out(), "{{block:{}, direction:{}}}", face.GetBlock(), face.GetDirection());
        }
    };
};

