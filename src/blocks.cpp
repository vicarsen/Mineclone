#include "blocks.h"

#include "files.h"

DEFINE_LOG_CATEGORY(BlockBase, FILE_LOGGER(trace, LOGFILE("Blocks/BlockBase.txt")));

namespace Game
{
    std::vector<BlockRegistry::BlockData> BlockRegistry::blocks;

    namespace __detail
    {
        BlockData& BlockIterator::Get() const
        {
            return BlockRegistry::blocks[block];
        }

        const BlockData& ConstBlockIterator::Get() const
        {
            return BlockRegistry::blocks[block];
        }

        BlockData& ReverseBlockIterator::Get() const
        {
            return BlockRegistry::blocks[block];
        }

        const BlockData& ConstReverseBlockIterator::Get() const
        {
            return BlockRegistry::blocks[block];
        }
    };

    void BlockMetadata::SetAllFaces(const std::string& path)
    {
        face_north = path; face_south = path;
        face_east = path; face_west = path;
        face_up = path; face_down = path;
    }

    BlockID BlockRegistry::RegisterBlock(const std::string& name, const BlockMetadata& metadata)
    {
        BlockID block = blocks.size();
        TRACE(BlockBase, "[RegisterBlock] (#name:{}) (block:{})", name, block);

        blocks.emplace_back(name, metadata);
        return block;
    }

    const std::string& BlockRegistry::GetBlockFaceTexture(BlockID block, Direction direction)
    {
        static const std::string empty_string = "";

        switch(direction)
        {
        case Direction::NORTH: return blocks[block].metadata.face_north;
        case Direction::SOUTH: return blocks[block].metadata.face_south;
        case Direction::EAST: return blocks[block].metadata.face_east;
        case Direction::WEST: return blocks[block].metadata.face_west;
        case Direction::UP: return blocks[block].metadata.face_up;
        case Direction::DOWN: return blocks[block].metadata.face_down;
        default: return empty_string;
        }
    }

    namespace VanillaBlocks
    {
        BlockID AIR_BLOCK = ([]() 
        {
            BlockMetadata metadata;
            return BlockRegistry::RegisterBlock("mineclone:air", metadata);
        })();

        BlockID BEDROCK_BLOCK = ([]()
        {
            BlockMetadata metadata;
            metadata.SetAllFaces(PATH("textures/bedrock.png"));
            return BlockRegistry::RegisterBlock("minecraft:bedrock", metadata);
        })();

        BlockID DIRT_BLOCK = ([]()
        {
            BlockMetadata metadata;
            metadata.SetAllFaces(PATH("textures/dirt.png"));
            return BlockRegistry::RegisterBlock("minecraft:dirt", metadata);
        })();

        BlockID GRASS_BLOCK = ([]()
        {
            BlockMetadata metadata;
            
            metadata.face_north = PATH("textures/grass_block_side.png");
            metadata.face_south = PATH("textures/grass_block_side.png");
            metadata.face_east = PATH("textures/grass_block_side.png");
            metadata.face_west = PATH("textures/grass_block_side.png");
            metadata.face_up = PATH("textures/grass_block_top.png");
            metadata.face_down = PATH("textures/dirt.png");

            return BlockRegistry::RegisterBlock("minecraft:grass_block", metadata);
        })();
    };
};

