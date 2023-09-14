#include "world.h"

#include "mathematics.h"

#include "files.h"

DEFINE_LOG_CATEGORY(Chunk, spdlog::level::trace, LOGFILE("World/Chunk.txt"));
DEFINE_LOG_CATEGORY(World, spdlog::level::trace, LOGFILE("World/World.txt"));

namespace Game
{
    Chunk::Chunk()
    {
        TRACE(Chunk, "[{}:constructor] <>", (unsigned long long) this);

        std::memset(blocks, 0, sizeof(blocks));
        std::memset(flags, 0, sizeof(flags));
    }

    Chunk::Chunk(Chunk&& other) :
        transform(other.transform), coords(other.coords), mesh(std::move(other.mesh)), places(std::move(other.places)), 
        faceRemove{std::move(other.faceRemove[0]), std::move(other.faceRemove[1]), std::move(other.faceRemove[2])},
        facePlace{std::move(other.facePlace[0]), std::move(other.facePlace[1]), std::move(other.facePlace[2])},
        faceToPlace{std::move(other.faceToPlace[0]), std::move(other.faceToPlace[1]), std::move(other.faceToPlace[2])},
        geometry_built(other.geometry_built)//, lazy_build(other.lazy_build)
    {
        TRACE(Chunk, "[{}:move_constructor] (#other:{}) <>", (unsigned long long) this, (unsigned long long) &other);

        std::memcpy(blocks, other.blocks, sizeof(blocks));
        std::memcpy(flags, other.flags, sizeof(flags));
    }

    Chunk::~Chunk()
    {
        TRACE(Chunk, "[{}:destructor] <>", (unsigned long long) this);
    }

    Chunk& Chunk::operator=(Chunk&& other)
    {
        TRACE(Chunk, "[{}:move_assignment] (#other:{})", (unsigned long long) this, (unsigned long long) &other);

        transform = other.transform;
        coords = other.coords;
        mesh = std::move(other.mesh);
        std::memcpy(blocks, other.blocks, sizeof(blocks));
        std::memcpy(flags, other.flags, sizeof(flags));

        places = std::move(other.places);

        for(int i = 0; i < 3; i++)
        {
            faceRemove[i] = std::move(other.faceRemove[i]);
            facePlace[i] = std::move(other.facePlace[i]);
            faceToPlace[i] = std::move(other.faceToPlace[i]);
        }

        geometry_built = other.geometry_built;
        lazy_build = other.lazy_build;

        TRACE(Chunk, "[{}:move_assignment] return", (unsigned long long) this);
        return *this;
    }

    void Chunk::__Deprecated_PlaceBlock(const glm::ivec3& position, BlockID block)
    {
        if((flags[position.x][position.y][position.z] & BLOCKFLAG_REPLACED) == 0)
        {
            places.emplace(position);

            if(mesh.HasFace(Render::FaceOrientation::XOY, position))
                faceRemove[(int) Render::FaceOrientation::XOY].emplace_back(position);

            if(mesh.HasFace(Render::FaceOrientation::YOZ, position))
                faceRemove[(int) Render::FaceOrientation::YOZ].emplace_back(position);

            if(mesh.HasFace(Render::FaceOrientation::ZOX, position))
                faceRemove[(int) Render::FaceOrientation::ZOX].emplace_back(position);

            if(mesh.HasFace(Render::FaceOrientation::XOY, { position.x, position.y, position.z + 1 }))
                faceRemove[(int) Render::FaceOrientation::XOY].emplace_back(position.x, position.y, position.z + 1);

            if(mesh.HasFace(Render::FaceOrientation::YOZ, { position.x + 1, position.y, position.z }))
                faceRemove[(int) Render::FaceOrientation::YOZ].emplace_back(position.x + 1, position.y, position.z);

            if(mesh.HasFace(Render::FaceOrientation::ZOX, { position.x, position.y + 1, position.z }))
                faceRemove[(int) Render::FaceOrientation::ZOX].emplace_back(position.x, position.y + 1, position.z);
            
            flags[position.x][position.y][position.z] |= BLOCKFLAG_REPLACED;
        }

        blocks[position.x][position.y][position.z] = block;
    }

    void Chunk::__Deprecated_RemoveBlock(const glm::ivec3& position)
    {
        PlaceBlock(position, 0);
    }

    void Chunk::PlaceBlock(const glm::ivec3& position, BlockID block)
    {
        blocks[position.x][position.y][position.z] = block;
        geometry_built = false;
    }

    void Chunk::RemoveBlock(const glm::ivec3& position)
    {
        blocks[position.x][position.y][position.z] = 0;
        geometry_built = false;
    }

    void Chunk::BuildGeometry()
    {
        TRACE(Chunk, "[{}:BuildGeometry]", (unsigned long long) this);
        //if(lazy_build)
        //{
            for(int y = 0; y < CHUNK_SIZE; y++)
                for(int z = 0; z < CHUNK_SIZE; z++)
                {
                    if(blocks[0][y][z] != 0)
                    {
                        facePlace[(int) Render::FaceOrientation::YOZ].emplace_back(0, y, z);
                        faceToPlace[(int) Render::FaceOrientation::YOZ].emplace_back(BlockBase::GetBlockFace(blocks[0][y][z], Direction::WEST));
                    }
                
                    if(blocks[CHUNK_SIZE - 1][y][z] != 0)
                    {
                        facePlace[(int) Render::FaceOrientation::YOZ].emplace_back(CHUNK_SIZE, y, z);
                        faceToPlace[(int) Render::FaceOrientation::YOZ].emplace_back(BlockBase::GetBlockFace(blocks[CHUNK_SIZE - 1][y][z], Direction::EAST));
                    }
                }

            for(int x = 1; x < CHUNK_SIZE; x++)
                for(int y = 0; y < CHUNK_SIZE; y++)
                    for(int z = 0; z < CHUNK_SIZE; z++)
                    {
                        if(blocks[x][y][z] == 0 && blocks[x - 1][y][z] != 0)
                        {
                            facePlace[(int) Render::FaceOrientation::YOZ].emplace_back(x, y, z);
                            faceToPlace[(int) Render::FaceOrientation::YOZ].emplace_back(BlockBase::GetBlockFace(blocks[x - 1][y][z], Direction::EAST));
                        }
                        else if(blocks[x][y][z] != 0 && blocks[x - 1][y][z] == 0)
                        {
                            facePlace[(int) Render::FaceOrientation::YOZ].emplace_back(x, y, z);
                            faceToPlace[(int) Render::FaceOrientation::YOZ].emplace_back(BlockBase::GetBlockFace(blocks[x][y][z], Direction::WEST));
                        }
                    }
            
            for(int x = 0; x < CHUNK_SIZE; x++)
                for(int z = 0; z < CHUNK_SIZE; z++)
                {
                    if(blocks[x][0][z] != 0)
                    {
                        facePlace[(int) Render::FaceOrientation::ZOX].emplace_back(x, 0, z);
                        faceToPlace[(int) Render::FaceOrientation::ZOX].emplace_back(BlockBase::GetBlockFace(blocks[x][0][z], Direction::DOWN));
                    }
                
                    if(blocks[x][CHUNK_SIZE - 1][z] != 0)
                    {
                        facePlace[(int) Render::FaceOrientation::ZOX].emplace_back(x, CHUNK_SIZE, z);
                        faceToPlace[(int) Render::FaceOrientation::ZOX].emplace_back(BlockBase::GetBlockFace(blocks[x][CHUNK_SIZE - 1][z], Direction::UP));
                    }
                }
            
            for(int x = 0; x < CHUNK_SIZE; x++)
                for(int y = 1; y < CHUNK_SIZE; y++)
                    for(int z = 0; z < CHUNK_SIZE; z++)
                    {
                        if(blocks[x][y][z] == 0 && blocks[x][y - 1][z] != 0)
                        {
                            facePlace[(int) Render::FaceOrientation::ZOX].emplace_back(x, y, z);
                            faceToPlace[(int) Render::FaceOrientation::ZOX].emplace_back(BlockBase::GetBlockFace(blocks[x][y - 1][z], Direction::UP));
                        }
                        else if(blocks[x][y][z] != 0 && blocks[x][y - 1][z] == 0)
                        {
                            facePlace[(int) Render::FaceOrientation::ZOX].emplace_back(x, y, z);
                            faceToPlace[(int) Render::FaceOrientation::ZOX].emplace_back(BlockBase::GetBlockFace(blocks[x][y][z], Direction::DOWN));
                        }
                    }

            for(int x = 0; x < CHUNK_SIZE; x++)
                for(int y = 0; y < CHUNK_SIZE; y++)
                {
                    if(blocks[x][y][0] != 0)
                    {
                        facePlace[(int) Render::FaceOrientation::XOY].emplace_back(x, y, 0);
                        faceToPlace[(int) Render::FaceOrientation::XOY].emplace_back(BlockBase::GetBlockFace(blocks[x][y][0], Direction::SOUTH));
                    }

                    if(blocks[x][y][CHUNK_SIZE - 1] != 0)
                    {
                        facePlace[(int) Render::FaceOrientation::XOY].emplace_back(x, y, CHUNK_SIZE);
                        faceToPlace[(int) Render::FaceOrientation::XOY].emplace_back(BlockBase::GetBlockFace(blocks[x][y][CHUNK_SIZE - 1], Direction::NORTH));
                    }
                }

            for(int x = 0; x < CHUNK_SIZE; x++)
                for(int y = 0; y < CHUNK_SIZE; y++)
                    for(int z = 1; z < CHUNK_SIZE; z++)
                    {
                        if(blocks[x][y][z] == 0 && blocks[x][y][z - 1] != 0)
                        {
                            facePlace[(int) Render::FaceOrientation::XOY].emplace_back(x, y, z);
                            faceToPlace[(int) Render::FaceOrientation::XOY].emplace_back(BlockBase::GetBlockFace(blocks[x][y][z - 1], Direction::NORTH));
                        }
                        else if(blocks[x][y][z] != 0 && blocks[x][y][z - 1] == 0)
                        {
                            facePlace[(int) Render::FaceOrientation::XOY].emplace_back(x, y, z);
                            faceToPlace[(int) Render::FaceOrientation::XOY].emplace_back(BlockBase::GetBlockFace(blocks[x][y][z], Direction::SOUTH));
                        }
                    }

            for(int i = 0; i < 3; i++)
            {
                mesh.ClearFaces((Render::FaceOrientation) i);
                
                if(facePlace[i].size())
                {
                    TRACE(Chunk, "[{}:BuildGeometry] (i:{}) (facePlace[i].size():{})", (unsigned long long) this, i, facePlace[i].size());

                    mesh.PlaceManyFaces(facePlace[i].size(), facePlace[i].data(), (Render::FaceOrientation) i, faceToPlace[i].data());
                    facePlace[i].clear(); faceToPlace[i].clear();
                }
            }

            geometry_built = true;

            TRACE(Chunk, "[{}:BuildGeometry] return", (unsigned long long) this);
            //lazy_build = false;
            //return;
        //}

    /*
        while(!places.empty())
        {
            glm::ivec3 position = places.front(); places.pop();
            BlockID block = blocks[position.x][position.y][position.z];

            if(block == 0)
            {
                if(position.x != 0 && IsBlockNotReplaced({ position.x - 1, position.y, position.z }) && blocks[position.x - 1][position.y][position.z] != 0)
                {
                    facePlace[(int) Render::FaceOrientation::YOZ].emplace_back(position);
                    faceToPlace[(int) Render::FaceOrientation::YOZ].emplace_back(BlockBase::GetBlockFace(blocks[position.x - 1][position.y][position.z], Direction::EAST));
                }

                if(position.x != CHUNK_SIZE - 1 && IsBlockNotReplaced({ position.x + 1, position.y, position.z }) && blocks[position.x + 1][position.y][position.z] != 0)
                {
                    facePlace[(int) Render::FaceOrientation::YOZ].emplace_back(position.x + 1, position.y, position.z);
                    faceToPlace[(int) Render::FaceOrientation::YOZ].emplace_back(BlockBase::GetBlockFace(blocks[position.x + 1][position.y][position.z], Direction::WEST));
                }

                if(position.y != 0 && IsBlockNotReplaced({ position.x, position.y - 1, position.z }) && blocks[position.x][position.y - 1][position.z] != 0)
                {
                    facePlace[(int) Render::FaceOrientation::ZOX].emplace_back(position);
                    faceToPlace[(int) Render::FaceOrientation::ZOX].emplace_back(BlockBase::GetBlockFace(blocks[position.x][position.y - 1][position.z], Direction::UP));
                }

                if(position.y != CHUNK_SIZE - 1 && IsBlockNotReplaced({ position.x, position.y + 1, position.z }) && blocks[position.x][position.y + 1][position.z] != 0)
                {
                    facePlace[(int) Render::FaceOrientation::ZOX].emplace_back(position.x, position.y + 1, position.z);
                    faceToPlace[(int) Render::FaceOrientation::ZOX].emplace_back(BlockBase::GetBlockFace(blocks[position.x][position.y + 1][position.z], Direction::DOWN));
                }

                if(position.z != 0 && IsBlockNotReplaced({ position.x, position.y, position.z - 1 }) && blocks[position.x][position.y][position.z - 1] != 0)
                {
                    facePlace[(int) Render::FaceOrientation::XOY].emplace_back(position);
                    faceToPlace[(int) Render::FaceOrientation::XOY].emplace_back(BlockBase::GetBlockFace(blocks[position.x][position.y][position.z - 1], Direction::NORTH));
                }

                if(position.z != CHUNK_SIZE - 1 && IsBlockNotReplaced({ position.x, position.y, position.z + 1 }) && blocks[position.x][position.y][position.z + 1] != 0)
                {
                    facePlace[(int) Render::FaceOrientation::XOY].emplace_back(position.x, position.y, position.z + 1);
                    faceToPlace[(int) Render::FaceOrientation::XOY].emplace_back(BlockBase::GetBlockFace(blocks[position.x][position.y][position.z + 1], Direction::SOUTH));
                }
            }
            else
            {
                if(position.x == 0 || (IsBlockNotReplaced({ position.x - 1, position.y, position.z }) && blocks[position.x - 1][position.y][position.z] == 0))
                {
                    facePlace[(int) Render::FaceOrientation::YOZ].emplace_back(position);
                    faceToPlace[(int) Render::FaceOrientation::YOZ].emplace_back(BlockBase::GetBlockFace(block, Direction::WEST));
                }

                if(position.x == CHUNK_SIZE - 1 || (IsBlockNotReplaced({ position.x + 1, position.y, position.z }) && blocks[position.x + 1][position.y][position.z] == 0))
                {
                    facePlace[(int) Render::FaceOrientation::YOZ].emplace_back(position.x + 1, position.y, position.z);
                    faceToPlace[(int) Render::FaceOrientation::YOZ].emplace_back(BlockBase::GetBlockFace(block, Direction::EAST));
                }

                if(position.y == 0 || (IsBlockNotReplaced({ position.x, position.y - 1, position.z }) && blocks[position.x][position.y - 1][position.z] == 0))
                {
                    facePlace[(int) Render::FaceOrientation::ZOX].emplace_back(position);
                    faceToPlace[(int) Render::FaceOrientation::ZOX].emplace_back(BlockBase::GetBlockFace(block, Direction::DOWN));
                }

                if(position.y == CHUNK_SIZE - 1 || (IsBlockNotReplaced({ position.x, position.y + 1, position.z }) && blocks[position.x][position.y + 1][position.z] == 0))
                {
                    facePlace[(int) Render::FaceOrientation::ZOX].emplace_back(position.x, position.y + 1, position.z);
                    faceToPlace[(int) Render::FaceOrientation::ZOX].emplace_back(BlockBase::GetBlockFace(block, Direction::UP));
                }

                if(position.z == 0 || (IsBlockNotReplaced({ position.x, position.y, position.z - 1 }) && blocks[position.x][position.y][position.z - 1] == 0))
                {
                    facePlace[(int) Render::FaceOrientation::XOY].emplace_back(position);
                    faceToPlace[(int) Render::FaceOrientation::XOY].emplace_back(BlockBase::GetBlockFace(block, Direction::SOUTH));
                }

                if(position.z == CHUNK_SIZE - 1 || (IsBlockNotReplaced({ position.x, position.y, position.z + 1}) && blocks[position.x][position.y][position.z + 1] == 0))
                {
                    facePlace[(int) Render::FaceOrientation::XOY].emplace_back(position.x, position.y, position.z + 1);
                    faceToPlace[(int) Render::FaceOrientation::XOY].emplace_back(BlockBase::GetBlockFace(block, Direction::NORTH));
                }
            }

            flags[position.x][position.y][position.z] &= (~BLOCKFLAG_REPLACED);
        }

        for(unsigned int i = 0; i < 3; i++)
        {
            if(faceRemove[i].size())
            {
                mesh.RemoveManyFaces(faceRemove[i].size(), faceRemove[i].data(), (Render::FaceOrientation) i);
                faceRemove[i].clear();
            }

            if(facePlace[i].size())
            {
                mesh.PlaceManyFaces(facePlace[i].size(), facePlace[i].data(), (Render::FaceOrientation) i, faceToPlace[i].data());
                facePlace[i].clear(); faceToPlace[i].clear();
            }
        }

        geometry_built = true;
    */
    }

    World::World()
    {
        TRACE(World, "[{}:constructor] <>", (unsigned long long) this);
    }

    World::~World()
    {
        TRACE(World, "[{}:destructor] <>", (unsigned long long) this);
    }

    World::World(World&& other) :
        chunks(std::move(other.chunks)), chunkPositions(std::move(other.chunkPositions))
    {
        TRACE(World, "[{}:move_constructor] (#other:{}) <>", (unsigned long long) this, (unsigned long long) &other);
    }

    World& World::operator=(World&& other)
    {
        TRACE(World, "[{}:move_assignment] (#other:{}) <>", (unsigned long long) this, (unsigned long long) &other);

        chunks = std::move(other.chunks);
        chunkPositions = std::move(other.chunkPositions);
        return *this;
    }

    void World::__Deprecated_PlaceBlock(const glm::ivec3& position, BlockID block)
    {
        glm::ivec3 chunk = GetChunkCoordinates(position);
        glm::ivec3 blockPosition = position - chunk * CHUNK_SIZE;

        chunks[chunkPositions[chunk]].PlaceBlock(blockPosition, block);
    }

    void World::__Deprecated_RemoveBlock(const glm::ivec3& position)
    {
        glm::ivec3 chunk = GetChunkCoordinates(position);
        glm::ivec3 blockPosition = position - chunk * CHUNK_SIZE;

        chunks[chunkPositions[chunk]].RemoveBlock(blockPosition);
    }

    void World::PlaceBlock(const glm::ivec3& position, BlockID block)
    {
        std::lock_guard<std::mutex> guard(chunksMutex);

        glm::ivec3 chunk = GetChunkCoordinates(position);
        glm::ivec3 blockPosition = position - chunk * CHUNK_SIZE;

        chunks[chunkPositions[chunk]].PlaceBlock(blockPosition, block);
    }

    void World::RemoveBlock(const glm::ivec3& position)
    {
        std::lock_guard<std::mutex> guard(chunksMutex);

        glm::ivec3 chunk = GetChunkCoordinates(position);
        glm::ivec3 blockPosition = position - chunk * CHUNK_SIZE;

        chunks[chunkPositions[chunk]].RemoveBlock(blockPosition);
    }

    Chunk& World::LoadChunk(const glm::ivec3& chunkCoords)
    {
        std::lock_guard<std::mutex> guard(chunksMutex);

        TRACE(World, "[{}:LoadChunk] (#chunkCoords:{})", (unsigned long long) this, chunkCoords);

        unsigned int chunkPosition = chunks.size();
        chunkPositions[chunkCoords] = chunkPosition;
        TRACE(World, "[{}:LoadChunk] (chunkPosition:{})", (unsigned long long) this, chunkPosition);

        Chunk& chunk = chunks.emplace_back();
        chunk.SetCoordinates(chunkCoords);
        chunk.GetTransform().Position() = chunkCoords * CHUNK_SIZE;

        TRACE(World, "[{}:LoadChunk] return", (unsigned long long) this);
        return chunk;
    }

    void World::LoadChunk(const glm::ivec3& chunkCoords, Chunk&& chunk)
    {
        std::lock_guard<std::mutex> guard(chunksMutex);

        unsigned int chunkPosition = chunks.size();
        chunkPositions[chunkCoords] = chunkPosition;

        Chunk& finalChunk = chunks.emplace_back(std::move(chunk));
        finalChunk.SetCoordinates(chunkCoords);
        finalChunk.GetTransform().Position() = chunkCoords * CHUNK_SIZE;
    }

    void World::UnloadChunk(const glm::ivec3& chunkCoords)
    {
        std::lock_guard<std::mutex> guard(chunksMutex);

        unsigned int position = chunkPositions[chunkCoords];
        chunkPositions.erase(chunkCoords);
        
        if(position != chunks.size() - 1)
        {
            glm::ivec3 other = chunks.back().GetCoordinates();
            chunks[position] = std::move(chunks.back());
            chunkPositions[other] = position;
        }

        chunks.pop_back();
    }

    Chunk& World::GetChunk(const glm::ivec3& chunkCoords)
    {
        return chunks[chunkPositions[chunkCoords]];
    }

    const Chunk& World::GetChunk(const glm::ivec3& chunkCoords) const
    {
        return chunks[chunkPositions.at(chunkCoords)];
    }

    glm::ivec3 World::GetChunkCoordinates(const glm::ivec3& coordinates) noexcept
    {
        int x = coordinates.x / CHUNK_SIZE, y = coordinates.y / CHUNK_SIZE, z = coordinates.z / CHUNK_SIZE;
        if(coordinates.x < 0 && coordinates.x % CHUNK_SIZE != 0) x--;
        if(coordinates.y < 0 && coordinates.y % CHUNK_SIZE != 0) y--;
        if(coordinates.z < 0 && coordinates.z % CHUNK_SIZE != 0) z--;
        return { x, y, z };
    }

    static void FillLayer(Game::Chunk& chunk, unsigned int layer, BlockID block)
    {
        for(unsigned int x = 0; x < CHUNK_SIZE; x++)
            for(unsigned int z = 0; z < CHUNK_SIZE; z++)
                chunk.PlaceBlock({ x, layer, z }, block);
    }

    void SuperflatChunkGenerator::GenerateChunk(Game::Chunk& chunk, const glm::ivec3& coords)
    {
        if(coords.y == 0)
        {
            FillLayer(chunk, 0, VanillaBlocks::BEDROCK_BLOCK);
            FillLayer(chunk, 1, VanillaBlocks::DIRT_BLOCK);
            FillLayer(chunk, 2, VanillaBlocks::DIRT_BLOCK);
            FillLayer(chunk, 3, VanillaBlocks::DIRT_BLOCK);
            FillLayer(chunk, 4, VanillaBlocks::GRASS_BLOCK);
        }
    }

    WorldLoader::WorldLoader(const std::shared_ptr<World>& _world, std::unique_ptr<ChunkGenerator>& _generator) :
        world(_world), generator(std::move(_generator)), previous(1000000000, 1000000000, 1000000000)
    {
    }

    WorldLoader::~WorldLoader()
    {
    }

    bool WorldLoader::LoadChunks(const glm::ivec3& center, unsigned int radius)
    {
        static const int d[] = { 0, 0, 1, 0, 0, -1, 0, 0 };

        if(previous == center)
            return true;

        std::shared_ptr<World> world = this->world.lock();
        if(!world)
            return false;

        std::queue<glm::ivec3> to_load;

        if(std::abs(previous.x - center.x) + std::abs(previous.y - center.y) + std::abs(previous.z - center.z) == 1)
        {
            INFO(LogTemp, "Lazy load");

            std::queue<glm::ivec3> to_unload;

            for(unsigned int i = 0; i < border.size(); )
            {
                glm::ivec3 cur = border[i];
                if(glm::distance(glm::vec3(cur), glm::vec3(center)) > radius)
                {
                    if(world->IsChunkLoaded(cur))
                        to_unload.emplace(cur);

                    on_border.erase(cur);
                    if(i != border.size() - 1)
                        border[i] = border.back();
                    border.pop_back();
                
                    for(unsigned int k = 0; k < 6; k++)
                    {
                        glm::ivec3 next = { cur.x + d[k], cur.y + d[k + 1], cur.z + d[k + 2] };
                        if(!on_border.contains(next) && glm::distance(glm::vec3(next), glm::vec3(center)) <= radius)
                        {
                            if(!world->IsChunkLoaded(next))
                                to_load.emplace(next);

                            on_border.emplace(next);
                            border.emplace_back(next);
                        }
                    }
                }
                else i++;
            }

            for(unsigned int i = 0; i < border.size(); i++)
            {
                glm::ivec3 cur = border[i];

                for(unsigned int k = 0; k < 6; k++)
                {
                    glm::ivec3 next = { cur.x + d[k], cur.y + d[k + 1], cur.z + d[k + 2] };
                    if(!on_border.contains(next) && glm::distance(glm::vec3(next), glm::vec3(center)) <= radius)
                    {
                        if(!world->IsChunkLoaded(next))
                            to_load.emplace(next);

                        on_border.emplace(next);
                        border.emplace_back(next);
                    }
                }
            }

            for(unsigned int i = 0; i < border.size(); )
            {
                glm::ivec3 cur = border[i];
                unsigned int count = 0;
                for(unsigned int k = 0; k < 6; k++)
                {
                    glm::ivec3 next = { cur.x + d[k], cur.y + d[k + 1], cur.z + d[k + 2] };
                    if(!on_border.contains(next))
                        count++;
                }

                if(count == 0)
                {
                    on_border.erase(cur);
                    if(i != border.size() - 1)
                        border[i] = border.back();
                    border.pop_back();
                }
                else i++;
            }

            while(!to_unload.empty())
            {
                glm::ivec3 cur = to_unload.front(); to_unload.pop();
                world->UnloadChunk(cur);
            }
        }
        else
        {
            std::unordered_set<glm::ivec3> to_unload;
            for(const Chunk& chunk : world->GetChunks())
                to_unload.emplace(chunk.GetCoordinates());

            std::unordered_set<glm::ivec3> visited;

            on_border.clear();
            border.clear();

            visited.emplace(center);
            border.emplace_back(center);

            for(unsigned int i = 0; i < border.size(); )
            {
                glm::ivec3 cur = border[i];
                unsigned int count = 0;
                for(unsigned int k = 0; k < 6; k++)
                {
                    glm::ivec3 next = { cur.x + d[k], cur.y + d[k + 1], cur.z + d[k + 2] };
                    if(!visited.contains(next) && glm::distance(glm::vec3(next), glm::vec3(center)) <= radius)
                    {
                        visited.emplace(next);
                        border.emplace_back(next);
                    }
                    else if(glm::distance(glm::vec3(next), glm::vec3(center)) > radius)
                        count++;
                }

                if(!world->IsChunkLoaded(cur))
                    to_load.emplace(cur);
                else to_unload.erase(cur);

                if(count == 0)
                {
                    if(i != border.size() - 1)
                        border[i] = border.back();
                    border.pop_back();
                }
                else
                {
                    on_border.emplace(cur);
                    i++;
                }
            }

            for(const glm::ivec3& cur : to_unload)
                world->UnloadChunk(cur);
        }

        while(!to_load.empty())
        {
            glm::ivec3 cur = to_load.front(); to_load.pop();
            
            Chunk& chunk = world->LoadChunk(cur);
            generator->GenerateChunk(chunk, cur);
            chunk.BuildGeometry();
        }

        previous = center;
        return true;
    }
};

