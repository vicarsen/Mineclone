#include "blocks.h"
#include "gui/blocks.h"
#include "format/blocks.h"

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
            return BlockRegistry::RegisterBlock("mineclone:bedrock", metadata);
        })();

        BlockID DIRT_BLOCK = ([]()
        {
            BlockMetadata metadata;
            metadata.SetAllFaces(PATH("textures/dirt.png"));
            return BlockRegistry::RegisterBlock("mineclone:dirt", metadata);
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

            return BlockRegistry::RegisterBlock("mineclone:grass_block", metadata);
        })();
    };
};

namespace GUI
{
    namespace Game
    {
        void DirectionCombo(const char* name, ::Game::Direction& direction)
        {
            if(BeginCombo(name, ToString(direction)))
            {
                for(int i = 0; i < 6; i++)
                {
                    ::Game::Direction current = (::Game::Direction) i;
                    bool is_selected = (current == direction);
                    if(Selectable(ToString(current), is_selected))
                        direction = current;
                    if(is_selected)
                        SetItemDefaultFocus();
                }

                EndCombo();
            }
        }

        void BlocksCombo(const char* name, ::Game::BlockID& block)
        {
            if(BeginCombo(name, ::Game::BlockRegistry::CFind(block)->name.c_str()))
            {
                for(auto it = ::Game::BlockRegistry::CBegin(); it != ::Game::BlockRegistry::CEnd(); it++)
                {
                    bool is_selected = (it == block);
                    if(Selectable(it->name.c_str(), is_selected))
                        block = it;
                    if(is_selected)
                        SetItemDefaultFocus();
                }

                EndCombo();
            }
        }

        BlocksDemoWindow::BlocksDemoWindow() :
            Window("Blocks Demo Window")
        {
        }

        void BlocksDemoWindow::Draw()
        {
            static ::Game::Direction direction = ::Game::Direction::UP;
            static ::Game::BlockID block = ::Game::VanillaBlocks::AIR_BLOCK;

            if(Begin())
            {
                DirectionCombo("Direction", direction);
                BlocksCombo("Block", block);
                End();
            }
        }
    };
};

const char* ToString(::Game::Direction direction)
{
    switch(direction)
    {
    case ::Game::Direction::UP: return "UP";
    case ::Game::Direction::DOWN: return "DOWN";
    case ::Game::Direction::NORTH: return "NORTH";
    case ::Game::Direction::SOUTH: return "SOUTH";
    case ::Game::Direction::EAST: return "EAST";
    case ::Game::Direction::WEST: return "WEST";
    default: return "";
    }
}

