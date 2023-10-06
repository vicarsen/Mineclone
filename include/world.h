#pragma once

#include <vector>
#include <unordered_map>
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
        Chunk(const Chunk& other) = delete;

        ~Chunk();

        Chunk& operator=(Chunk&& other);
        Chunk& operator=(const Chunk& other) = delete;

        void PlaceBlock(const glm::ivec3& position, BlockID block);
        void RemoveBlock(const glm::ivec3& position);

        void BuildGeometry();

        inline bool IsGeometryBuilt() const noexcept { return geometry_built; }
        inline void SetGeometryBuilt(bool built) noexcept { geometry_built = built; }

        inline Transform& GetTransform() noexcept { return transform; }
        inline const Transform& GetTransform() const noexcept { return transform; }

        inline void SetCoordinates(const glm::ivec3& coords) noexcept { this->coords = coords; }
        inline const glm::ivec3& GetCoordinates() const noexcept { return coords; }

    private:
        void LazyMesh(int layer, const Game::Face slice[CHUNK_SIZE][CHUNK_SIZE], Render::FaceOrientation orientation, std::vector<Render::FaceMesh>& out);
        Render::FaceMesh GenerateLongFace(int layer, const Game::Face& face, Render::FaceOrientation orientation, int x1, int y1, int x2, int y2);

    private:
        BlockID blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

        Game::Transform transform;
        glm::ivec3 coords;

        Render::ChunkMeshHandle mesh_handle;
        
        bool geometry_built = false;
        bool is_empty = true;
    };

    class ChunkGenerator
    {
    public:
        virtual void GenerateChunk(Game::Chunk& chunk, const glm::ivec3& chunk_coordinates) = 0;
        virtual bool IsEmpty(const glm::ivec3& position) = 0;
    };

    class SuperflatChunkGenerator : public ChunkGenerator
    {
    public:
         virtual void GenerateChunk(Game::Chunk& chunk, const glm::ivec3& chunk_coordinates) override;
         virtual bool IsEmpty(const glm::ivec3& position) override;
    };

    class World
    {
    public:
        World(std::unique_ptr<ChunkGenerator>& chunk_generator);
        World(World&& other);
        World(const World& other) = delete;
        ~World();

        World& operator=(World&& other);
        World& operator=(const World& other) = delete;

        void PlaceBlock(const glm::ivec3& position, BlockID block);
        void RemoveBlock(const glm::ivec3& position);

        Chunk& LoadChunk(const glm::ivec3& chunk_coordinates);
        void UnloadChunk(const glm::ivec3& chunk_coordinates);

        Chunk& TransposeChunk(const glm::ivec3& from, const glm::ivec3& to);

        Chunk& GetChunk(const glm::ivec3& chunk_coordinates);
        const Chunk& GetChunk(const glm::ivec3& chunk_coordinates) const;

        inline std::vector<Chunk>& GetChunks() noexcept { return chunks; }
        inline const std::vector<Chunk>& GetChunks() const noexcept { return chunks; }

        inline ChunkGenerator* GetChunkGenerator() { return chunk_generator.get(); }

        inline bool IsChunkLoaded(const glm::ivec3& coordinates) const noexcept { return chunk_index.find(coordinates) != chunk_index.end(); }

        inline std::mutex& GetMutex() const noexcept { return mutex; }

        static glm::ivec3 GlobalCoordinatesToChunkCoordinates(const glm::ivec3& coordinates) noexcept;
        static glm::ivec3 GlobalCoordinatesToLocalCoordinates(const glm::ivec3& coordinates) noexcept;

    private:
        mutable std::mutex mutex;

        std::vector<Chunk> chunks;
        std::unordered_map<glm::ivec3, unsigned int> chunk_index;

        std::unique_ptr<ChunkGenerator> chunk_generator;
    };

    class WorldLoadThread
    {
    public:
        WorldLoadThread();
        ~WorldLoadThread();

        void AddPlayer(const std::shared_ptr<Player>& player);
        void RemovePlayer(const std::shared_ptr<Player>& player);

        void SetWorld(const std::shared_ptr<World>& world);

        inline const std::atomic<bool>& IsInitialized() const noexcept { return initialized; }

    private:
        struct LoadTarget
        {
            glm::ivec3 center;
            float radius;
        };

    private:
        void Run();

        void RunLoadPass();

        void FindLoadTargets(std::vector<LoadTarget>& targets);
        void FindChunksInSphere(const glm::ivec3& center, float radius, std::unordered_set<glm::ivec3>& out);
        void FilterChunks(std::unordered_set<glm::ivec3>& chunks);
        void FindChunksToLoadOrUnload(std::unordered_set<glm::ivec3>& required, std::vector<glm::ivec3>& to_load, std::vector<glm::ivec3>& to_unload);
        void SortChunks(std::vector<glm::ivec3>& chunks, const glm::ivec3& target);
        void LoadAndUnloadChunks(const std::vector<glm::ivec3>& to_load, const std::vector<glm::ivec3>& to_unload);

    private:
        std::thread thread;
        std::atomic<bool> exit;
        std::atomic<bool> initialized;

        std::shared_ptr<World> world;
        mutable std::mutex world_mutex;

        std::vector<std::shared_ptr<Player>> players;
        mutable std::mutex players_mutex;
    };
};

