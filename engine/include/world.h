#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "mathematics.h"
#include "transform.h"
#include "blocks.h"
#include "render.h"

#include "utils/array.h"
#include "utils/hash.h"
#include "utils/thread.h"

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

        void PlaceBlock(const ::Math::ivec3& position, BlockID block);
        void RemoveBlock(const ::Math::ivec3& position);

        void BuildGeometry();

        inline bool IsGeometryBuilt() const noexcept { return geometry_built; }
        inline void SetGeometryBuilt(bool built) noexcept { geometry_built = built; }

        inline ::Math::Transform& GetTransform() noexcept { return transform; }
        inline const ::Math::Transform& GetTransform() const noexcept { return transform; }

        inline void SetCoordinates(const ::Math::ivec3& coords) noexcept { this->coords = coords; }
        inline const ::Math::ivec3& GetCoordinates() const noexcept { return coords; }

    private:
        void LazyMesh(int layer, const Face slice[CHUNK_SIZE][CHUNK_SIZE], ::Render::FaceOrientation orientation, ::Utils::Array<::Render::FaceMesh>& out);
        ::Render::FaceMesh GenerateLongFace(int layer, const Face& face, ::Render::FaceOrientation orientation, int x1, int y1, int x2, int y2);

    private:
        BlockID blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

        ::Math::Transform transform;
        ::Math::ivec3 coords;

        ::Render::ChunkMeshHandle mesh_handle;
        
        bool geometry_built = false;
        bool is_empty = true;
    };

    class ChunkGenerator
    {
    public:
        virtual void GenerateChunk(Chunk& chunk, const ::Math::ivec3& chunk_coordinates) = 0;
        virtual bool IsEmpty(const ::Math::ivec3& position) = 0;
    };

    class SuperflatChunkGenerator : public ChunkGenerator
    {
    public:
         virtual void GenerateChunk(Chunk& chunk, const ::Math::ivec3& chunk_coordinates) override;
         virtual bool IsEmpty(const ::Math::ivec3& position) override;
    };

    class Perlin2DChunkGenerator : public ChunkGenerator
    {
    public:
        Perlin2DChunkGenerator(const ::Math::uvec2& size, float plains_height, float hills_height, float mountains_height);
        ~Perlin2DChunkGenerator() = default;

        virtual void GenerateChunk(Chunk& chunk, const ::Math::ivec3& chunk_coordinates) override;
        virtual bool IsEmpty(const ::Math::ivec3& position) override;

    private:
        ::Utils::Array<float> heightmap;
        ::Math::uvec2 size;
        float max_height;
    };

    class World
    {
    public:
        World(::Utils::UniquePointer<ChunkGenerator>& chunk_generator);
        World(World&& other);
        World(const World& other) = delete;
        ~World();

        World& operator=(World&& other);
        World& operator=(const World& other) = delete;

        void PlaceBlock(const ::Math::ivec3& position, BlockID block);
        void RemoveBlock(const ::Math::ivec3& position);

        Chunk& LoadChunk(const ::Math::ivec3& chunk_coordinates);
        void UnloadChunk(const ::Math::ivec3& chunk_coordinates);

        Chunk& TransposeChunk(const ::Math::ivec3& from, const ::Math::ivec3& to);

        Chunk& GetChunk(const ::Math::ivec3& chunk_coordinates);
        const Chunk& GetChunk(const ::Math::ivec3& chunk_coordinates) const;

        inline ::Utils::Array<Chunk>& GetChunks() noexcept { return chunks; }
        inline const ::Utils::Array<Chunk>& GetChunks() const noexcept { return chunks; }

        inline ChunkGenerator* GetChunkGenerator() { return chunk_generator.get(); }

        inline bool IsChunkLoaded(const ::Math::ivec3& coordinates) const noexcept { return chunk_index.find(coordinates) != chunk_index.end(); }

        inline ::Utils::Mutex& GetMutex() const noexcept { return mutex; }

        static ::Math::ivec3 GlobalCoordinatesToChunkCoordinates(const ::Math::ivec3& coordinates) noexcept;
        static ::Math::ivec3 GlobalCoordinatesToLocalCoordinates(const ::Math::ivec3& coordinates) noexcept;

    private:
        mutable ::Utils::Mutex mutex;

        ::Utils::Array<Chunk> chunks;
        ::Utils::HashMap<::Math::ivec3, unsigned int> chunk_index;

        ::Utils::UniquePointer<ChunkGenerator> chunk_generator;
    };

    class WorldLoadThread
    {
    public:
        WorldLoadThread();
        ~WorldLoadThread();

        void AddPlayer(const ::Utils::SharedPointer<Player>& player);
        void RemovePlayer(const ::Utils::SharedPointer<Player>& player);

        void SetWorld(const ::Utils::SharedPointer<World>& world);

        inline const ::std::atomic<bool>& IsInitialized() const noexcept { return initialized; }

    private:
        struct LoadTarget
        {
            ::Math::ivec3 center;
            float radius;
        };

    private:
        void Run();

        int RunLoadPass();

        void FindLoadTargets(::Utils::Array<LoadTarget>& targets);
        void FindChunksInSphere(const ::Math::ivec3& center, float radius, ::Utils::HashSet<::Math::ivec3>& out);
        void FilterChunks(::Utils::HashSet<::Math::ivec3>& chunks);
        void FindChunksToLoadOrUnload(::Utils::HashSet<::Math::ivec3>& required, ::Utils::Array<::Math::ivec3>& to_load, ::Utils::Array<::Math::ivec3>& to_unload);
        void SortChunks(::Utils::Array<::Math::ivec3>& chunks, const ::Math::ivec3& target);
        int LoadAndUnloadChunks(const ::Utils::Array<::Math::ivec3>& to_load, const ::Utils::Array<::Math::ivec3>& to_unload);

    private:
        ::Utils::Thread thread;
        ::std::atomic<bool> exit;
        ::std::atomic<bool> initialized;

        ::Utils::SharedPointer<World> world;
        mutable ::Utils::Mutex world_mutex;

        ::Utils::Array<::Utils::SharedPointer<Player>> players;
        mutable ::Utils::Mutex players_mutex;
    };
};

