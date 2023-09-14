#include "blocks.h"

#include "files.h"
#include "render.h"

DEFINE_LOG_CATEGORY(BlockBase, spdlog::level::trace, LOGFILE("Blocks/BlockBase.txt"));

namespace Game
{
    std::vector<BlockBase::BlockData> BlockBase::blocks;

    void BlockMetadata::SetFaces(const std::string& path)
    {
        north = path; south = path;
        east = path; west = path;
        up = path; down = path;
    }

    void BlockMetadata::SetFaces(const std::string& _north, const std::string& _south, const std::string& _east, const std::string& _west, const std::string& _up, const std::string& _down)
    {
        north = _north; south = _south;
        east = _east; west = _west;
        up = _up; down = _down;
    }

    BlockID BlockBase::RegisterBlock(const std::string& name, const BlockMetadata& metadata)
    {
        BlockID block = blocks.size();
        TRACE(BlockBase, "[RegisterBlock] (#name:{}) (#metadata:{}) (block:{})", name, metadata, block);

        blocks.emplace_back(name, metadata.IsTransparent(),
                            metadata.GetNorthFace(), metadata.GetSouthFace(),
                            metadata.GetEastFace(), metadata.GetWestFace(),
                            metadata.GetUpFace(), metadata.GetDownFace());
        return block;
    }

    Face BlockBase::GetBlockFace(BlockID block, Direction direction)
    {
        Face face;
        face.SetBlock(block);
        face.SetDirection(direction);
        return face;
    }

    const std::string& BlockBase::GetFaceTexture(BlockID block, Direction direction)
    {
        switch(direction)
        {
        case Direction::NORTH: return blocks[block].north;
        case Direction::SOUTH: return blocks[block].south;
        case Direction::EAST: return blocks[block].east;
        case Direction::WEST: return blocks[block].west;
        case Direction::UP: return blocks[block].up;
        case Direction::DOWN: return blocks[block].down;
        }
    }

    namespace VanillaBlocks
    {
        BlockID AIR_BLOCK = ([]() 
        {
            BlockMetadata metadata;
            metadata.SetTransparent(true);
            return BlockBase::RegisterBlock("mineclone:air", metadata);
        })();

        BlockID BEDROCK_BLOCK = ([]()
        {
            BlockMetadata metadata;
            metadata.SetFaces(PATH("textures/bedrock.png"));
            return BlockBase::RegisterBlock("minecraft:bedrock", metadata);
        })();

        BlockID DIRT_BLOCK = ([]()
        {
            BlockMetadata metadata;
            metadata.SetFaces(PATH("textures/dirt.png"));
            return BlockBase::RegisterBlock("minecraft:dirt", metadata);
        })();

        BlockID GRASS_BLOCK = ([]()
        {
            BlockMetadata metadata;
            metadata.SetFaces(PATH("textures/grass_block_side.png"), PATH("textures/grass_block_side.png"), PATH("textures/grass_block_side.png"), PATH("textures/grass_block_side.png"), PATH("textures/grass_block_top.png"), PATH("textures/dirt.png"));
            return BlockBase::RegisterBlock("minecraft:grass_block", metadata);
        })();
    };
};

