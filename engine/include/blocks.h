#pragma once

#include "logger.h"

#include "utils/array.h"
#include "utils/string.h"

DECLARE_LOG_CATEGORY(BlockRegistry);

namespace Game
{
    typedef unsigned char BlockID;

    // Direction of a block face.
    enum class Direction { UP, DOWN, NORTH, SOUTH, EAST, WEST };

    // Describes a block's face (general block, not specific block placed in a world).
    struct Face
    {
        BlockID block;
        Direction direction;

        inline bool operator!=(const Face& other) const noexcept
        {
            return block != other.block || direction != other.direction;
        }

        inline bool operator==(const Face& other) const noexcept
        {
            return block == other.block && direction == other.direction;
        }
    };

    // The metadata of a block.
    struct BlockMetadata
    {
        ::Utils::String face_north;
        ::Utils::String face_south;
        ::Utils::String face_east;
        ::Utils::String face_west;
        ::Utils::String face_up;
        ::Utils::String face_down;

        // Sets all the faces to have the same texture, given by "path".
        void SetAllFaces(const ::Utils::String& path);
    };

    namespace __detail
    {
        // All the data about a block stored by the block registry.
        struct BlockData
        {
            ::Utils::String name;
            BlockMetadata metadata;

            // Identifier of the block's render data.
            unsigned int render_attachment;
        };

        // An bidirectional iterator that iterates over the blocks in the registry.
        class BlockIterator
        {
        public:
            inline BlockIterator(BlockID block) noexcept : block(block) {}
            inline BlockIterator(BlockIterator&& other) noexcept : block(other.block) {}
            inline BlockIterator(const BlockIterator& other) noexcept : block(other.block) {}

            ~BlockIterator() noexcept = default;

            inline BlockIterator& operator=(BlockIterator&& other) noexcept { block = other.block; return *this; }
            inline BlockIterator& operator=(const BlockIterator& other) noexcept { block = other.block; return *this; }

            inline BlockIterator& operator++() noexcept { return Inc(); }
            inline BlockIterator operator++(int) noexcept { BlockIterator cpy(*this); Inc(); return cpy; }

            inline BlockIterator& operator--() noexcept { return Dec(); }
            inline BlockIterator operator--(int) noexcept { BlockIterator cpy(*this); Dec(); return cpy; }

            inline BlockIterator Next() const noexcept { return BlockIterator(*this).Inc(); }
            inline BlockIterator Prev() const noexcept { return BlockIterator(*this).Dec(); }

            inline BlockIterator& Inc() noexcept { ++block; return *this; }
            inline BlockIterator& Dec() noexcept { --block; return *this; }

            inline BlockData& operator*() const { return Get(); }
            inline BlockData* operator->() const { return &Get(); }

            inline bool operator==(const BlockIterator& other) const noexcept { return block == other.block; }
            inline bool operator!=(const BlockIterator& other) const noexcept { return block != other.block; }
            
            inline bool operator==(BlockID other) const noexcept { return block == other; }
            inline bool operator!=(BlockID other) const noexcept { return block != other; }

            inline operator BlockID() const noexcept { return block; }

            BlockData& Get() const;

        private:
            BlockID block;
        };

        // A constant bidirectional iterator that iterates over the blocks in the registry exposing the block data.
        class ConstBlockIterator
        {
        public:
            inline ConstBlockIterator(BlockID block) noexcept : block(block) {}
            inline ConstBlockIterator(ConstBlockIterator&& other) noexcept : block(other.block) {}
            inline ConstBlockIterator(const ConstBlockIterator& other) noexcept : block(other.block) {}

            ~ConstBlockIterator() noexcept = default;

            inline ConstBlockIterator& operator=(ConstBlockIterator&& other) noexcept { block = other.block; return *this; }
            inline ConstBlockIterator& operator=(const ConstBlockIterator& other) noexcept { block = other.block; return *this; }

            inline ConstBlockIterator& operator++() noexcept { return Inc(); }
            inline ConstBlockIterator operator++(int) noexcept { ConstBlockIterator cpy(*this); Inc(); return cpy; }
            
            inline ConstBlockIterator& operator--() noexcept { return Dec(); }
            inline ConstBlockIterator operator--(int) noexcept { ConstBlockIterator cpy(*this); Dec(); return cpy; }

            inline ConstBlockIterator Next() const noexcept { return ConstBlockIterator(*this).Inc(); }
            inline ConstBlockIterator Prev() const noexcept { return ConstBlockIterator(*this).Dec(); }

            inline ConstBlockIterator& Inc() noexcept { ++block; return *this; }
            inline ConstBlockIterator& Dec() noexcept { --block; return *this; }

            inline const BlockData& operator*() const { return Get(); }
            inline const BlockData* operator->() const { return &Get(); }

            const BlockData& Get() const;

            inline bool operator==(const ConstBlockIterator& other) const noexcept { return block == other.block; }
            inline bool operator!=(const ConstBlockIterator& other) const noexcept { return block != other.block; }

            inline bool operator==(BlockID other) const noexcept { return block == other; }
            inline bool operator!=(BlockID other) const noexcept { return block != other; }

            inline operator BlockID() const noexcept { return block; }

        private:
            BlockID block;
        };

        // A reverse bidirectional iterator that iterates over the blocks in the registry exposing the block data.
        class ReverseBlockIterator
        {
        public:
            inline ReverseBlockIterator(BlockID block) noexcept : block(block) {}
            inline ReverseBlockIterator(ReverseBlockIterator&& other) noexcept : block(other.block) {}
            inline ReverseBlockIterator(const ReverseBlockIterator& other) noexcept : block(other.block) {}

            ~ReverseBlockIterator() noexcept = default;

            inline ReverseBlockIterator& operator=(ReverseBlockIterator&& other) noexcept { block = other.block; return *this; }
            inline ReverseBlockIterator& operator=(const ReverseBlockIterator& other) noexcept { block = other.block; return *this; }

            inline ReverseBlockIterator& operator++() noexcept { return Inc(); }
            inline ReverseBlockIterator operator++(int) noexcept { ReverseBlockIterator cpy(*this); Inc(); return cpy; }
            
            inline ReverseBlockIterator& operator--() noexcept { return Dec(); }
            inline ReverseBlockIterator operator--(int) noexcept { ReverseBlockIterator cpy(*this); Dec(); return cpy; }

            inline ReverseBlockIterator Next() const noexcept { return ReverseBlockIterator(*this).Inc(); }
            inline ReverseBlockIterator Prev() const noexcept { return ReverseBlockIterator(*this).Dec(); }

            inline ReverseBlockIterator& Inc() noexcept { --block; return *this; }
            inline ReverseBlockIterator& Dec() noexcept { ++block; return *this; }

            inline BlockData& operator*() const { return Get(); }
            inline BlockData* operator->() const { return &Get(); }

            BlockData& Get() const;

            inline bool operator==(const ReverseBlockIterator& other) const noexcept { return block == other.block; }
            inline bool operator!=(const ReverseBlockIterator& other) const noexcept { return block != other.block; }

            inline bool operator==(BlockID other) const noexcept { return block == other; }
            inline bool operator!=(BlockID other) const noexcept { return block != other; }

            inline operator BlockID() const noexcept { return block; }

        private:
            BlockID block;
        };

        // A constant reverse block iterator that iterates over the blocks in the registry, exposing the block data.
        class ConstReverseBlockIterator
        {
        public:
            inline ConstReverseBlockIterator(BlockID block) noexcept : block(block) {}
            inline ConstReverseBlockIterator(ConstReverseBlockIterator&& other) noexcept : block(other.block) {}
            inline ConstReverseBlockIterator(const ConstReverseBlockIterator& other) noexcept : block(other.block) {}

            ~ConstReverseBlockIterator() noexcept = default;

            inline ConstReverseBlockIterator& operator=(ConstReverseBlockIterator&& other) noexcept { block = other.block; return *this; }
            inline ConstReverseBlockIterator& operator=(const ConstReverseBlockIterator& other) noexcept { block = other.block; return *this; }

            inline ConstReverseBlockIterator& operator++() noexcept { return Inc(); }
            inline ConstReverseBlockIterator operator++(int) noexcept { ConstReverseBlockIterator cpy(*this); Inc(); return cpy; }

            inline ConstReverseBlockIterator& operator--() noexcept { return Dec(); }
            inline ConstReverseBlockIterator operator--(int) noexcept { ConstReverseBlockIterator cpy(*this); Dec(); return cpy; }

            inline ConstReverseBlockIterator Next() const noexcept { return ConstReverseBlockIterator(*this).Inc(); }
            inline ConstReverseBlockIterator Prev() const noexcept { return ConstReverseBlockIterator(*this).Dec(); }

            inline ConstReverseBlockIterator& Inc() noexcept { --block; return *this; }
            inline ConstReverseBlockIterator& Dec() noexcept { ++block; return *this; }

            inline const BlockData& operator*() const { return Get(); }
            inline const BlockData* operator->() const { return &Get(); }

            const BlockData& Get() const;

            inline bool operator==(const ConstReverseBlockIterator& other) const noexcept { return block == other.block; }
            inline bool operator!=(const ConstReverseBlockIterator& other) const noexcept { return block != other.block; }

            inline bool operator==(BlockID other) const noexcept { return block == other; }
            inline bool operator!=(BlockID other) const noexcept { return block != other; }

            inline operator BlockID() const noexcept { return block; }

        private:
            BlockID block;
        };
    };

    // The block registry holds all data about all the blocks in the game.
    class BlockRegistry
    {
    public:
        typedef __detail::BlockIterator Iterator;
        typedef __detail::ConstBlockIterator CIterator;
        typedef __detail::ReverseBlockIterator RIterator;
        typedef __detail::ConstReverseBlockIterator CRIterator;
        typedef __detail::BlockData BlockData;

        BlockRegistry() = delete;

        // Registers a new block, with a given name and some given metadata, returning an ID to refer to it later.
        static BlockID RegisterBlock(const ::Utils::String& name, const BlockMetadata& metadata);

        // Returns the filepath of a block face's texture.
        static const ::Utils::String& GetBlockFaceTexture(BlockID block, Direction face);

        inline static Iterator Begin() noexcept { return Iterator(0); }
        inline static Iterator End() noexcept { return Iterator(blocks.size()); }

        inline static CIterator CBegin() noexcept { return CIterator(0); }
        inline static CIterator CEnd() noexcept { return CIterator(blocks.size()); }

        inline static RIterator RBegin() noexcept { return RIterator(blocks.size() - 1); }
        inline static RIterator REnd() noexcept { return RIterator(-1); }

        inline static CRIterator CRBegin() noexcept { return CRIterator(blocks.size() - 1); }
        inline static CRIterator CREnd() noexcept { return CRIterator(-1); }

        inline static Iterator Find(BlockID block) noexcept { return Iterator(block); }
        inline static CIterator CFind(BlockID block) noexcept { return CIterator(block); }
        inline static RIterator RFind(BlockID block) noexcept { return RIterator(block); }
        inline static CRIterator CRFind(BlockID block) noexcept { return CRIterator(block); }

    private:
        static ::Utils::Array<BlockData> blocks;

        friend class __detail::BlockIterator;
        friend class __detail::ConstBlockIterator;
        friend class __detail::ReverseBlockIterator;
        friend class __detail::ConstReverseBlockIterator;
    };

    namespace VanillaBlocks
    {
        extern BlockID AIR_BLOCK;
        extern BlockID BEDROCK_BLOCK;
        extern BlockID DIRT_BLOCK;
        extern BlockID GRASS_BLOCK;
    };
};

const char* ToString(::Game::Direction direction);

