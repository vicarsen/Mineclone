#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <mutex>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "transform.h"
#include "blocks.h"
#include "render.h"

DECLARE_LOG_CATEGORY(Chunk);
DECLARE_LOG_CATEGORY(World);

namespace Game
{
    class Chunk
    {
    public:
        Chunk();
        Chunk(Chunk&& other);
        ~Chunk();

        Chunk& operator=(Chunk&& other);

        void __Deprecated_PlaceBlock(const glm::ivec3& position, BlockID block);
        void __Deprecated_RemoveBlock(const glm::ivec3& position);

        inline void __Deprecated_SetGeometryBuilt(bool geometry) noexcept { geometry_built = geometry; }
        inline void __Deprecated_SetLazy(bool lazy) noexcept { lazy_build = lazy; }

        void PlaceBlock(const glm::ivec3& position, BlockID block);
        void RemoveBlock(const glm::ivec3& position);

        void BuildGeometry();

        inline bool IsGeometryBuilt() const noexcept { return geometry_built; }

        inline Render::ChunkMesh& GetMesh() noexcept { return mesh; }
        inline const Render::ChunkMesh& GetMesh() const noexcept { return mesh; }

        inline Transform& GetTransform() noexcept { return transform; }
        inline const Transform& GetTransform() const noexcept { return transform; }

        inline void SetCoordinates(const glm::ivec3& _coords) noexcept { coords = _coords; }
        inline const glm::ivec3& GetCoordinates() const noexcept { return coords; }

    private:
        typedef char BlockFlag;

        static inline const BlockFlag BLOCKFLAG_REPLACED = (1 << 0);

        inline BlockFlag IsBlockNotReplaced(const glm::ivec3& position) const { return (flags[position.x][position.y][position.z] & BLOCKFLAG_REPLACED) == 0; }
        
        BlockID GetFaceBlock(const glm::ivec3& position, const glm::ivec3& next);

    private:
        Render::ChunkMesh mesh;
        BlockID blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
        BlockFlag flags[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
        Game::Transform transform;
        glm::ivec3 coords;

        std::queue<glm::ivec3> places;
        
        std::vector<glm::ivec3> faceRemove[3];
        std::vector<glm::ivec3> facePlace[3];
        std::vector<Face> faceToPlace[3];

        bool geometry_built = false;
        bool lazy_build = false;
    };

    class World
    {
    public:
        World();
        World(World&& other);
        ~World();

        World& operator=(World&& other);

        void __Deprecated_PlaceBlock(const glm::ivec3& position, BlockID block);
        void __Deprecated_RemoveBlock(const glm::ivec3& position);

        void PlaceBlock(const glm::ivec3& position, BlockID block);
        void RemoveBlock(const glm::ivec3& position);

        Chunk& LoadChunk(const glm::ivec3& chunkCoords);
        void LoadChunk(const glm::ivec3& chunkCoords, Chunk&& chunk);
        void UnloadChunk(const glm::ivec3& chunkCoords);

        Chunk& GetChunk(const glm::ivec3& chunkCoords);
        const Chunk& GetChunk(const glm::ivec3& chunkCoords) const;

        inline std::vector<Chunk>& GetChunks() noexcept { return chunks; }
        inline const std::vector<Chunk>& GetChunks() const noexcept { return chunks; }

        inline bool IsChunkLoaded(const glm::ivec3& coordinates) const noexcept { return chunkPositions.find(coordinates) != chunkPositions.end(); }

        inline void lock() { chunksMutex.lock(); }
        inline bool try_lock() { return chunksMutex.try_lock(); }
        inline void unlock() { chunksMutex.unlock(); }

        static glm::ivec3 GetChunkCoordinates(const glm::ivec3& coordinates) noexcept;

    private:
        std::mutex chunksMutex;
        std::vector<Chunk> chunks;
        std::unordered_map<glm::ivec3, unsigned int> chunkPositions;
    };

    class ChunkGenerator
    {
    public:
        virtual void GenerateChunk(Game::Chunk& chunk, const glm::ivec3& coords) = 0;
    };

    class SuperflatChunkGenerator : public ChunkGenerator
    {
    public:
        virtual void GenerateChunk(Game::Chunk& chunk, const glm::ivec3& coords) override;
    };

    class WorldLoader
    {
    public:
        WorldLoader(const std::shared_ptr<World>& world, std::unique_ptr<ChunkGenerator>& generator);
        ~WorldLoader();

        bool LoadChunks(const glm::ivec3& center, unsigned int radius);

    private:
        std::weak_ptr<World> world;
        std::unique_ptr<ChunkGenerator> generator;
        glm::ivec3 previous;

        std::unordered_set<glm::ivec3> on_border;
        std::vector<glm::ivec3> border;
    };
};

