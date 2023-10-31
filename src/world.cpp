#include "world.h"
#include "GLFW/glfw3.h"

#include "format/glm.h"
#include "format/render.h"
#include "format/blocks.h"

#include "mathematics.h"
#include "files.h"

#include "application.h"

#include "gui/render.h"

#include <unordered_set>
#include <queue>

DEFINE_LOG_CATEGORY(Chunk, FILE_LOGGER(trace, LOGFILE("World/Chunk.txt")));
DEFINE_LOG_CATEGORY(World, FILE_LOGGER(trace, LOGFILE("world/World.txt")));

static constexpr unsigned int encode(unsigned int x, unsigned int y, unsigned int z, unsigned int texture)
{
    return (texture) | (z << 11) | (y << 18) | (x << 25);
}

static int face_count = 0;

static Render::FaceMesh GenerateFaceMesh(const glm::ivec3& position, Render::FaceOrientation orientation, const Game::Face& face)
{
    face_count++;

    Render::FaceMesh mesh;

    Render::RenderThread& render_thread = Application::Get()->GetRenderThread();
    int encoded = encode(position.x, position.y, position.z, render_thread.GetBlockTextureID(face));
    switch(face.direction)
    {
    case Game::Direction::SOUTH:
    case Game::Direction::WEST:
    case Game::Direction::DOWN:
        encoded |= (1 << 10);
        break;
    default:
        break;
    }

    switch(orientation)
    {
    case Render::FaceOrientation::XOY:
        mesh.v[0].encoded = encoded + encode(0, 0, 0, 0);
        mesh.v[1].encoded = encoded + encode(1, 0, 0, 0);
        mesh.v[2].encoded = encoded + encode(1, 1, 0, 0);
        mesh.v[3].encoded = encoded + encode(0, 1, 0, 0);
        break;
    case Render::FaceOrientation::YOZ:
        mesh.v[0].encoded = encoded + encode(0, 0, 0, 0);
        mesh.v[1].encoded = encoded + encode(0, 1, 0, 0);
        mesh.v[2].encoded = encoded + encode(0, 1, 1, 0);
        mesh.v[3].encoded = encoded + encode(0, 0, 1, 0);
        break;
    case Render::FaceOrientation::ZOX:
        mesh.v[0].encoded = encoded + encode(0, 0, 0, 0);
        mesh.v[1].encoded = encoded + encode(0, 0, 1, 0);
        mesh.v[2].encoded = encoded + encode(1, 0, 1, 0);
        mesh.v[3].encoded = encoded + encode(1, 0, 0, 0);
        break;
    }

    switch(face.direction)
    {
    case Game::Direction::SOUTH:
    case Game::Direction::WEST:
    case Game::Direction::DOWN:
        std::swap(mesh.v[1], mesh.v[3]);
        break;
    default:
        break;
    }

    return mesh;
}

namespace Game
{
    Chunk::Chunk()
    {
        TRACE(Chunk, "[{}:constructor] <>", (unsigned long long) this);

        std::memset(blocks, 0, sizeof(blocks));
        mesh_handle = -1;
    }

    Chunk::Chunk(Chunk&& other) :
        transform(other.transform), coords(other.coords), geometry_built(other.geometry_built), is_empty(other.is_empty), mesh_handle(other.mesh_handle)
    {
        TRACE(Chunk, "[{}:move_constructor] (#other:{}) <>", (unsigned long long) this, (unsigned long long) &other);

        std::memcpy(blocks, other.blocks, sizeof(blocks));
        other.mesh_handle = -1;
    }

    Chunk::~Chunk()
    {
        TRACE(Chunk, "[{}:destructor] <>", (unsigned long long) this);

        if(mesh_handle != -1) 
        {
            Render::RenderThread& render_thread = Application::Get()->GetRenderThread();
            
            render_thread.Execute([handle = mesh_handle](auto& render_context)
            {
                render_context.FreeChunkMesh(handle);
            });

        }
    }

    Chunk& Chunk::operator=(Chunk&& other)
    {
        TRACE(Chunk, "[{}:move_assignment] (#other:{})", (unsigned long long) this, (unsigned long long) &other);

        if(mesh_handle != -1) 
        {
            Render::RenderThread& render_thread = Application::Get()->GetRenderThread();
         
            render_thread.Execute([handle = mesh_handle](auto& render_context)
            {
                render_context.FreeChunkMesh(handle);
            });
        }

        transform = other.transform;
        coords = other.coords;

        std::memcpy(blocks, other.blocks, sizeof(blocks));

        geometry_built = other.geometry_built;
        is_empty = other.is_empty;

        mesh_handle = other.mesh_handle;
        other.mesh_handle = -1;

        TRACE(Chunk, "[{}:move_assignment] return", (unsigned long long) this);
        return *this;
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
        PROFILE_FUNCTION();

        static thread_local Face faces[3][CHUNK_SIZE + 1][CHUNK_SIZE][CHUNK_SIZE];
        static thread_local bool empty[3][CHUNK_SIZE + 1];
        
        bool is_chunk_empty = true;
        for(int x = 0; x < CHUNK_SIZE && is_chunk_empty; x++)
            for(int y = 0; y < CHUNK_SIZE && is_chunk_empty; y++)
                for(int z = 0; z < CHUNK_SIZE && is_chunk_empty; z++)
                    is_chunk_empty = (is_chunk_empty && blocks[x][y][z] == 0);

        if(is_chunk_empty)
        {
            if(!is_empty && mesh_handle != -1)
            {
                Render::RenderThread& render_thread = Application::Get()->GetRenderThread();
                render_thread.Execute([mesh_handle = mesh_handle](auto& render_context)
                {
                    if(render_context.IsChunkInRenderQueue(mesh_handle))
                        render_context.RemoveChunkFromRenderQueue(mesh_handle);

                    const auto& mesh = render_context.Get(mesh_handle);
                    mesh->Clear(Render::FaceOrientation::XOY);
                    mesh->Clear(Render::FaceOrientation::YOZ);
                    mesh->Clear(Render::FaceOrientation::ZOX);
                });
            }

            is_empty = true;
            geometry_built = true;
            return;
        }
        
        is_empty = false;

        for(int i = 0; i < 3; i++)
            for(int x = 0; x <= CHUNK_SIZE; x++)
            {
                for(int y = 0; y < CHUNK_SIZE; y++)
                    for(int z = 0; z < CHUNK_SIZE; z++)
                        faces[i][x][y][z] = { 0, Direction::UP };
                empty[i][x] = true;
            }

        for(int y = 0; y < CHUNK_SIZE; y++)
            for(int z = 0; z < CHUNK_SIZE; z++)
            {
                if(blocks[0][y][z] != 0)
                {
                    faces[(int) Render::FaceOrientation::YOZ][0][y][z] = { blocks[0][y][z], Direction::WEST };
                    empty[(int) Render::FaceOrientation::YOZ][0] = false;
                }
                if(blocks[CHUNK_SIZE - 1][y][z] != 0)
                {
                    faces[(int) Render::FaceOrientation::YOZ][CHUNK_SIZE][y][z] = { blocks[CHUNK_SIZE - 1][y][z], Direction::EAST };
                    empty[(int) Render::FaceOrientation::YOZ][CHUNK_SIZE] = false;
                }
            }

        for(int x = 1; x < CHUNK_SIZE; x++)
            for(int y = 0; y < CHUNK_SIZE; y++)
                for(int z = 0; z < CHUNK_SIZE; z++)
                {
                    if(blocks[x][y][z] == 0 && blocks[x - 1][y][z] != 0)
                    {
                        faces[(int) Render::FaceOrientation::YOZ][x][y][z] = { blocks[x - 1][y][z], Direction::EAST };
                        empty[(int) Render::FaceOrientation::YOZ][x] = false;
                    }
                    if(blocks[x][y][z] != 0 && blocks[x - 1][y][z] == 0)
                    {
                        faces[(int) Render::FaceOrientation::YOZ][x][y][z] = { blocks[x][y][z], Direction::WEST };
                        empty[(int) Render::FaceOrientation::YOZ][x] = false;
                    }
                }

        for(int x = 0; x < CHUNK_SIZE; x++)
            for(int z = 0; z < CHUNK_SIZE; z++)
            {
                if(blocks[x][0][z] != 0)
                {
                    faces[(int) Render::FaceOrientation::ZOX][0][x][z] = { blocks[x][0][z], Direction::DOWN };
                    empty[(int) Render::FaceOrientation::ZOX][0] = false;
                }
                if(blocks[x][CHUNK_SIZE - 1][z] != 0)
                {
                    faces[(int) Render::FaceOrientation::ZOX][CHUNK_SIZE][x][z] = { blocks[x][CHUNK_SIZE - 1][z], Direction::UP };
                    empty[(int) Render::FaceOrientation::ZOX][CHUNK_SIZE] = false;
                }
            }

        for(int x = 0; x < CHUNK_SIZE; x++)
            for(int y = 1; y < CHUNK_SIZE; y++)
                for(int z = 0; z < CHUNK_SIZE; z++)
                {
                    if(blocks[x][y][z] == 0 && blocks[x][y - 1][z] != 0)
                    {
                        faces[(int) Render::FaceOrientation::ZOX][y][x][z] = { blocks[x][y - 1][z], Direction::UP };
                        empty[(int) Render::FaceOrientation::ZOX][y] = false;
                    }
                    if(blocks[x][y][z] != 0 && blocks[x][y - 1][z] == 0)
                    {
                        faces[(int) Render::FaceOrientation::ZOX][y][x][z] = { blocks[x][y][z], Direction::DOWN };
                        empty[(int) Render::FaceOrientation::ZOX][y] = false;
                    }
                }

        for(int x = 0; x < CHUNK_SIZE; x++)
            for(int y = 0; y < CHUNK_SIZE; y++)
            {
                if(blocks[x][y][0] != 0)
                {
                    faces[(int) Render::FaceOrientation::XOY][0][x][y] = { blocks[x][y][0], Direction::SOUTH };
                    empty[(int) Render::FaceOrientation::XOY][0] = false;
                }
                if(blocks[x][y][CHUNK_SIZE - 1] != 0)
                {
                    faces[(int) Render::FaceOrientation::XOY][CHUNK_SIZE][x][y] = { blocks[x][y][CHUNK_SIZE - 1], Direction::NORTH };
                    empty[(int) Render::FaceOrientation::XOY][CHUNK_SIZE] = false;
                }
            }

        for(int x = 0; x < CHUNK_SIZE; x++)
            for(int y = 0; y < CHUNK_SIZE; y++)
                for(int z = 1; z < CHUNK_SIZE; z++)
                {
                    if(blocks[x][y][z] == 0 && blocks[x][y][z - 1] != 0)
                    {
                        faces[(int) Render::FaceOrientation::XOY][z][x][y] = { blocks[x][y][z - 1], Direction::NORTH };
                        empty[(int) Render::FaceOrientation::XOY][z] = false;
                    }
                    if(blocks[x][y][z] != 0 && blocks[x][y][z - 1] == 0)
                    {
                        faces[(int) Render::FaceOrientation::XOY][z][x][y] = { blocks[x][y][z], Direction::SOUTH };
                        empty[(int) Render::FaceOrientation::XOY][z] = false;
                    }
                }

        std::vector<Render::FaceMesh> face_meshes[3];

        /*
        face_count = 0;

        for(int x = 0; x <= CHUNK_SIZE; x++)
            for(int y = 0; y < CHUNK_SIZE; y++)
                for(int z = 0; z < CHUNK_SIZE; z++)
                {
                    int i = (int) Render::FaceOrientation::YOZ;
                    if(faces[i][x][y][z].block != 0)
                        face_meshes[i].emplace_back(GenerateFaceMesh({ x, y, z }, Render::FaceOrientation::YOZ, faces[i][x][y][z]));
                }

        for(int x = 0; x < CHUNK_SIZE; x++)
            for(int y = 0; y <= CHUNK_SIZE; y++)
                for(int z = 0; z < CHUNK_SIZE; z++)
                {
                    int i = (int) Render::FaceOrientation::ZOX;
                    if(faces[i][y][x][z].block != 0)
                        face_meshes[i].emplace_back(GenerateFaceMesh({ x, y, z }, Render::FaceOrientation::ZOX, faces[i][y][x][z]));
                }

        for(int x = 0; x < CHUNK_SIZE; x++)
            for(int y = 0; y < CHUNK_SIZE; y++)
                for(int z = 0; z <= CHUNK_SIZE; z++)
                {
                    int i = (int) Render::FaceOrientation::XOY;
                    if(faces[i][z][x][y].block != 0)
                        face_meshes[i].emplace_back(GenerateFaceMesh({ x, y, z }, Render::FaceOrientation::XOY, faces[i][z][x][y]));
                }

        INFO(LogTemp, "Generated {} faces for a chunk", face_count);
        */


        for(int i = 0; i < 3; i++)
            for(int x = 0; x <= CHUNK_SIZE; x++)
                if(!empty[i][x])
                    LazyMesh(x, faces[i][x], (Render::FaceOrientation) i, face_meshes[i]);

        geometry_built = true;
        
        {
            PROFILE_SCOPE(RenderSync);

            Render::RenderThread& render_thread = Application::Get()->GetRenderThread();
            if(mesh_handle == -1)
            {
                std::atomic<bool> done = false;
                render_thread.Execute([&done, handle = &mesh_handle, model_matrix = transform.GetMatrix(), face_meshes](auto& render_context)
                {
                    *handle = render_context.NewChunkMesh();
                    const auto& mesh = render_context.Get(*handle);

                    mesh->SetModelMatrix(model_matrix);
                    for(std::size_t i = 0; i < 3; i++)
                        mesh->SetFaces(face_meshes[i].size(), face_meshes[i].data(), (Render::FaceOrientation) i);

                    render_context.AddChunkToRenderQueue(mesh);
                    
                    done = true;
                });

                while(!done);
            }
            else
            {
                render_thread.Execute([handle = mesh_handle, model_matrix = transform.GetMatrix(), mesh_0 = std::move(face_meshes[0]), mesh_1 = std::move(face_meshes[1]), mesh_2 = std::move(face_meshes[2])](auto& render_context)
                {
                    const auto& mesh = render_context.Get(handle);
                    mesh->SetFaces(mesh_0.size(), mesh_0.data(), (Render::FaceOrientation) 0);
                    mesh->SetFaces(mesh_1.size(), mesh_1.data(), (Render::FaceOrientation) 1);
                    mesh->SetFaces(mesh_2.size(), mesh_2.data(), (Render::FaceOrientation) 2);
                    mesh->SetModelMatrix(model_matrix);

                    if(!render_context.IsChunkInRenderQueue(handle))
                        render_context.AddChunkToRenderQueue(mesh);
                });
            }
        }
    }

    void Chunk::LazyMesh(int layer, const Game::Face slice[CHUNK_SIZE][CHUNK_SIZE], Render::FaceOrientation orientation, std::vector<Render::FaceMesh>& out)
    {
        PROFILE_FUNCTION();

        int cx = 0, cy = 0;
        for(int x = 0; x < CHUNK_SIZE; x++)
        {
            int lastx = 0, lasty = 0;
            for(int y = 1; y < CHUNK_SIZE; y++)
            {
                if(slice[x][lasty] != slice[x][y])
                {
                    if(slice[x][lasty].block != 0) cx++;
                    lasty = y;
                }

                if(slice[lastx][x] != slice[y][x])
                {
                    if(slice[lastx][x].block != 0) cy++;
                    lastx = y;
                }
            }
            if(slice[x][lasty].block != 0)
                cx++;
            if(slice[lastx][x].block != 0)
                cy++;
        }

        if(cx <= cy)
        {
            PROFILE_SCOPE(LazyMeshGenerate);

            out.reserve(out.size() + cx);
            for(int x = 0; x < CHUNK_SIZE; x++)
            {
                int last = 0;
                for(int y = 1; y < CHUNK_SIZE; y++)
                {
                    if(slice[x][last] != slice[x][y])
                    {
                        if(slice[x][last].block != 0)
                            out.emplace_back(GenerateLongFace(layer, slice[x][last], orientation, x, last, x + 1, y));
                        last = y;
                    }
                }
                if(slice[x][last].block != 0)
                    out.emplace_back(GenerateLongFace(layer, slice[x][last], orientation, x, last, x + 1, CHUNK_SIZE));
            }
        }
        else
        {
            PROFILE_SCOPE(LazyMeshGenerate);

            out.reserve(out.size() + cy);
            for(int y = 0; y < CHUNK_SIZE; y++)
            {
                int last = 0;
                for(int x = 1; x < CHUNK_SIZE; x++)
                {
                    if(slice[last][y] != slice[x][y])
                    {
                        if(slice[last][y].block != 0)
                            out.emplace_back(GenerateLongFace(layer, slice[last][y], orientation, last, y, x, y + 1));
                        last = x;
                    }
                }
                if(slice[last][y].block != 0)
                    out.emplace_back(GenerateLongFace(layer, slice[last][y], orientation, last, y, CHUNK_SIZE, y + 1));
            }
        }
    }

    Render::FaceMesh Chunk::GenerateLongFace(int layer, const Game::Face& face, Render::FaceOrientation orientation, int x1, int y1, int x2, int y2)
    {
        PROFILE_FUNCTION();

        Render::FaceMesh mesh;

        Render::RenderThread& render_thread = Application::Get()->GetRenderThread();
        int texture = render_thread.GetBlockTextureID(face);

        switch(orientation)
        {
        case Render::FaceOrientation::XOY:
            {
                mesh.v[0].encoded = encode(x1, y1, layer, texture);
                mesh.v[1].encoded = encode(x2, y1, layer, texture);
                mesh.v[2].encoded = encode(x2, y2, layer, texture);
                mesh.v[3].encoded = encode(x1, y2, layer, texture);
                break;
            }
        case Render::FaceOrientation::YOZ:
            {
                mesh.v[0].encoded = encode(layer, x1, y1, texture);
                mesh.v[1].encoded = encode(layer, x2, y1, texture);
                mesh.v[2].encoded = encode(layer, x2, y2, texture);
                mesh.v[3].encoded = encode(layer, x1, y2, texture);
                break;
            }
        case Render::FaceOrientation::ZOX:
            {
                mesh.v[0].encoded = encode(x1, layer, y1, texture);
                mesh.v[1].encoded = encode(x1, layer, y2, texture);
                mesh.v[2].encoded = encode(x2, layer, y2, texture);
                mesh.v[3].encoded = encode(x2, layer, y1, texture);
                break;
            }
        }

        switch(face.direction)
        {
        case Direction::SOUTH:
        case Direction::WEST:
        case Direction::DOWN:
            {
                std::swap(mesh.v[1], mesh.v[3]);
                mesh.v[0].encoded |= (1 << 10);
                mesh.v[1].encoded |= (1 << 10);
                mesh.v[2].encoded |= (1 << 10);
                mesh.v[3].encoded |= (1 << 10);
                break;
            }
        case Direction::NORTH:
        case Direction::EAST:
        case Direction::UP:
            break;
        }

        return mesh;
    }

    World::World(std::unique_ptr<ChunkGenerator>& chunk_generator)
    {
        TRACE(World, "[{}:constructor] <>", (unsigned long long) this);

        this->chunk_generator = std::move(chunk_generator);
    }

    World::World(World&& other) :
        chunks(std::move(other.chunks)), chunk_index(std::move(other.chunk_index)), chunk_generator(std::move(other.chunk_generator))
    {
        TRACE(World, "[{}:move_constructor] (#other:{}) <>", (unsigned long long) this, (unsigned long long) &other);
    }

    World::~World()
    {
        TRACE(World, "[{}:destructor] <>", (unsigned long long) this);
    }

    World& World::operator=(World&& other)
    {
        TRACE(World, "[{}:move_assignment] (#other:{}) <>", (unsigned long long) this, (unsigned long long) &other);

        chunks = std::move(other.chunks);
        chunk_index = std::move(other.chunk_index);
        chunk_generator = std::move(other.chunk_generator);

        return *this;
    }

    void World::PlaceBlock(const glm::ivec3& position, BlockID block)
    {
        glm::ivec3 chunk_coordinates = GlobalCoordinatesToChunkCoordinates(position);
        glm::ivec3 block_coordinates = GlobalCoordinatesToLocalCoordinates(position);

        chunks.at(chunk_index.at(chunk_coordinates)).PlaceBlock(block_coordinates, block);
    }

    void World::RemoveBlock(const glm::ivec3& position)
    {
        glm::ivec3 chunk_coordinates = GlobalCoordinatesToChunkCoordinates(position);
        glm::ivec3 block_coordinates = GlobalCoordinatesToLocalCoordinates(position);

        chunks.at(chunk_index.at(chunk_coordinates)).RemoveBlock(block_coordinates);
    }

    Chunk& World::LoadChunk(const glm::ivec3& chunk_coordinates)
    {
        TRACE(World, "[{}:LoadChunk] (#chunkCoords:{})", (unsigned long long) this, chunk_coordinates);

        unsigned int chunk_idx = chunks.size();
        chunk_index[chunk_coordinates] = chunk_idx;
        TRACE(World, "[{}:LoadChunk] (chunkPosition:{})", (unsigned long long) this, chunk_idx);

        Chunk& chunk = chunks.emplace_back();
        chunk.SetCoordinates(chunk_coordinates);
        chunk.GetTransform().Position() = chunk_coordinates * CHUNK_SIZE;

        TRACE(World, "[{}:LoadChunk] return", (unsigned long long) this);
        return chunk;
    }

    void World::UnloadChunk(const glm::ivec3& chunk_coordinates)
    {
        unsigned int chunk_idx = chunk_index.at(chunk_coordinates);
        chunk_index.erase(chunk_coordinates);
        
        if(chunk_idx != chunks.size() - 1)
        {
            glm::ivec3 other = chunks.back().GetCoordinates();
            chunks.at(chunk_idx) = std::move(chunks.back());
            chunk_index.at(other) = chunk_idx;
        }

        chunks.pop_back();
    }

    Chunk& World::TransposeChunk(const glm::ivec3& from, const glm::ivec3& to)
    {
        unsigned int chunk_idx = chunk_index.at(from);
        chunk_index.erase(from);
        chunk_index.emplace(to, chunk_idx);

        Chunk& chunk = chunks[chunk_idx];
        chunk.SetCoordinates(to);
        chunk.GetTransform().Position() = to * CHUNK_SIZE;
        return chunk;
    }

    Chunk& World::GetChunk(const glm::ivec3& chunk_coordinates)
    {
        return chunks.at(chunk_index.at(chunk_coordinates));
    }

    const Chunk& World::GetChunk(const glm::ivec3& chunk_coordinates) const
    {
        return chunks.at(chunk_index.at(chunk_coordinates));
    }

    glm::ivec3 World::GlobalCoordinatesToChunkCoordinates(const glm::ivec3& coordinates) noexcept
    {
        int x = coordinates.x / CHUNK_SIZE, y = coordinates.y / CHUNK_SIZE, z = coordinates.z / CHUNK_SIZE;
        if(coordinates.x < 0 && coordinates.x % CHUNK_SIZE != 0) x--;
        if(coordinates.y < 0 && coordinates.y % CHUNK_SIZE != 0) y--;
        if(coordinates.z < 0 && coordinates.z % CHUNK_SIZE != 0) z--;
        return { x, y, z };
    }

    glm::ivec3 World::GlobalCoordinatesToLocalCoordinates(const glm::ivec3& coordinates) noexcept
    {
        glm::ivec3 chunk_coordinates = GlobalCoordinatesToChunkCoordinates(coordinates);
        return coordinates - chunk_coordinates * CHUNK_SIZE;
    }

    static void FillLayer(Game::Chunk& chunk, unsigned int layer, BlockID block)
    {
        for(unsigned int x = 0; x < CHUNK_SIZE; x++)
            for(unsigned int z = 0; z < CHUNK_SIZE; z++)
                chunk.PlaceBlock({ x, layer, z }, block);
    }

    static void FillEmpty(Game::Chunk& chunk)
    {
        for(int x = 0; x < CHUNK_SIZE; x++)
            for(int y = 0; y < CHUNK_SIZE; y++)
                for(int z = 0; z < CHUNK_SIZE; z++)
                    chunk.PlaceBlock({ x, y, z }, Game::VanillaBlocks::AIR_BLOCK);
    }

    void SuperflatChunkGenerator::GenerateChunk(Game::Chunk& chunk, const glm::ivec3& coords)
    {
        if(coords.y == 0)
        {
            FillLayer(chunk, 0, VanillaBlocks::BEDROCK_BLOCK);
            FillLayer(chunk, 1, VanillaBlocks::DIRT_BLOCK);
            FillLayer(chunk, 2, VanillaBlocks::DIRT_BLOCK);
            FillLayer(chunk, 3, VanillaBlocks::DIRT_BLOCK);

            if(coords.x == 0)
                FillLayer(chunk, 4, VanillaBlocks::BEDROCK_BLOCK);
            else FillLayer(chunk, 4, VanillaBlocks::GRASS_BLOCK);
        
            for(int y = 5; y < CHUNK_SIZE; y++)
                FillLayer(chunk, y, VanillaBlocks::AIR_BLOCK);
        }
        else
        {
            for(int y = 0; y < CHUNK_SIZE; y++)
                FillLayer(chunk, y, VanillaBlocks::AIR_BLOCK);
        }
    }

    bool SuperflatChunkGenerator::IsEmpty(const glm::ivec3& position)
    {
        return position.y != 0;
    }

    Perlin2DChunkGenerator::Perlin2DChunkGenerator(const glm::uvec2& size, float plains_height, float hills_height, float mountains_height) :
        size(size), max_height(mountains_height)
    {
        heightmap.resize((CHUNK_SIZE * size.x) * (CHUNK_SIZE * size.y), 0.0f);
        std::atomic<bool> flag = false;

        Render::RenderThread& render_thread = Application::Get()->GetRenderThread();
        render_thread.Execute([&](auto& context)
        {
            std::vector<float> mountains_heightmap(heightmap.size());
            std::vector<float> hills_heightmap(heightmap.size());
            std::vector<float> plains_heightmap(heightmap.size());

            glm::uvec2 resolution = { CHUNK_SIZE, CHUNK_SIZE };
            glm::uvec2 total_size = resolution * size;
            
            context.GeneratePerlin2DNoise(resolution / glm::uvec2(2, 2), total_size, plains_heightmap.data());
            context.GeneratePerlin2DNoise(resolution * glm::uvec2(2, 2), total_size, hills_heightmap.data());
            context.GeneratePerlin2DNoise(resolution * glm::uvec2(4, 4), total_size, mountains_heightmap.data());

            for(unsigned int i = 0; i < heightmap.size(); i++)
                heightmap[i] = plains_heightmap[i] * plains_height + hills_heightmap[i] * (hills_height - plains_height) + mountains_heightmap[i] * (mountains_height - hills_height);

            flag = true;
        });

        while(!flag);
    }

    void Perlin2DChunkGenerator::GenerateChunk(Game::Chunk& chunk, const glm::ivec3& position)
    {
        if(position.y * CHUNK_SIZE > max_height || position.y < 0)
        {
            FillEmpty(chunk);
            return;
        }

        if(position.x < 0 || position.x >= size.x)
        {
            FillEmpty(chunk);
            return;
        }

        if(position.z < 0 || position.z >= size.y)
        {
            FillEmpty(chunk);
            return;
        }

        for(int x = 0; x < CHUNK_SIZE; x++)
            for(int z = 0; z < CHUNK_SIZE; z++)
            {
                int gx = x + position.x * CHUNK_SIZE, gz = z + position.z * CHUNK_SIZE;
                float height = heightmap[gx + gz * size.x * CHUNK_SIZE];

                for(int y = 0; y < CHUNK_SIZE; y++)
                {
                    int gy = y + position.y * CHUNK_SIZE;
                    if(gy <= height && gy + 1 > height)
                        chunk.PlaceBlock({ x, y, z }, Game::VanillaBlocks::GRASS_BLOCK);
                    else if(gy <= height)
                        chunk.PlaceBlock({ x, y, z }, Game::VanillaBlocks::DIRT_BLOCK);
                    else chunk.PlaceBlock({ x, y, z }, Game::VanillaBlocks::AIR_BLOCK);
                }
            }
    }

    bool Perlin2DChunkGenerator::IsEmpty(const glm::ivec3& position)
    {
        if(position.y * CHUNK_SIZE > max_height || position.y < 0)
            return true;

        if(position.x < 0 || position.x >= size.x)
            return true;

        if(position.z < 0 || position.z >= size.y)
            return true;

        bool empty = true;
        for(int x = 0; x < CHUNK_SIZE && empty; x++)
            for(int z = 0; z < CHUNK_SIZE; z++)
            {
                int gx = x + position.x * CHUNK_SIZE, gz = z + position.z * CHUNK_SIZE;
                empty = (empty && heightmap[gx + gz * size.x * CHUNK_SIZE] < position.y * CHUNK_SIZE);
            }

        return empty;
    }

    WorldLoadThread::WorldLoadThread()
    {
        exit = false;
        initialized = false;
        world = nullptr;
        thread = std::move(std::thread([&]() { Run(); }));
    }

    WorldLoadThread::~WorldLoadThread()
    {
        exit = true;
        thread.join();
    }

    void WorldLoadThread::AddPlayer(const std::shared_ptr<Player>& player)
    {
        std::lock_guard<std::mutex> guard(players_mutex);
        players.emplace_back(player);
    }

    void WorldLoadThread::RemovePlayer(const std::shared_ptr<Player>& player)
    {
        std::lock_guard<std::mutex> guard(players_mutex);
        
        int i = 0;
        while(i < players.size() && players[i] != player)
            i++;

        if(i == players.size())
            return;

        if(i != players.size() - 1)
            players[i] = players.back();
        players.pop_back();
    }

    void WorldLoadThread::SetWorld(const std::shared_ptr<World>& world)
    {
        std::lock_guard<std::mutex> guard(world_mutex);
        this->world = world;
    }

    void WorldLoadThread::Run()
    {
        initialized = true;

        float last_time = glfwGetTime();
        int count = 0, chunks_generated_total = 0;
        while(!exit)
        {
            PROFILE_THREAD(WorldLoader);

            float world_load_start_time = glfwGetTime();
            
            chunks_generated_total += RunLoadPass();

            float world_load_end_time = glfwGetTime();
            float world_load_time = world_load_end_time - world_load_start_time;

            float sleep_time = (world_load_time < 1.0f ? 1.0f - world_load_time : 0.0f);

            if(sleep_time != 0.0f)
                ;//std::this_thread::sleep_for(std::chrono::microseconds((long long) (sleep_time * 1'000'000.0f)));

            count++;

            float time = glfwGetTime();
            //if(time - last_time >= 2.0f)
            {
                ::GUI::Render::WorldGenerationInfoEvent event;
                event.fps = count / (time - last_time);
                event.chunks_generated = chunks_generated_total;
                Application::Get()->DispatchEvent(event);

                last_time = time;
                count = 0;
                chunks_generated_total = 0;
            }
        }
    }

    int WorldLoadThread::RunLoadPass()
    {
        std::lock_guard<std::mutex> world_ptr_guard(world_mutex);

        if(world != nullptr)
        {
            std::vector<LoadTarget> load_targets;
            FindLoadTargets(load_targets);

            std::unordered_set<glm::ivec3> required_chunks;
            for(int i = 0; i < load_targets.size(); i++)
                FindChunksInSphere(load_targets[i].center, load_targets[i].radius, required_chunks);
            FilterChunks(required_chunks);

            std::vector<glm::ivec3> to_load, to_unload;
            FindChunksToLoadOrUnload(required_chunks, to_load, to_unload);

            SortChunks(to_load, load_targets.front().center);
            SortChunks(to_unload, load_targets.front().center);
            
            return LoadAndUnloadChunks(to_load, to_unload);
        }
        else return 0;
    }

    void WorldLoadThread::FindLoadTargets(std::vector<LoadTarget>& targets)
    {
        std::lock_guard<std::mutex> guard(players_mutex);
        for(int i = 0; i < players.size(); i++)
            targets.emplace_back(World::GlobalCoordinatesToChunkCoordinates(players[i]->GetTransform().Position()), players[i]->GetRenderDistance());
    }

    void WorldLoadThread::FindChunksInSphere(const glm::ivec3& center, float radius, std::unordered_set<glm::ivec3>& out)
    {
        static const int d[] = { 0, 0, -1, 0, 0, 1, 0, 0 };

        std::queue<glm::ivec3> queue;
        queue.emplace(center);
        out.emplace(center);

        while(!queue.empty())
        {
            glm::ivec3 cur = queue.front(); queue.pop();

            for(int k = 0; k < 6; k++)
            {
                glm::ivec3 next = { cur.x + d[k], cur.y + d[k + 1], cur.z + d[k + 2] };
                if(out.find(next) == out.end() && glm::distance2(glm::vec3(center), glm::vec3(next)) <= radius * radius)
                {
                    queue.emplace(next);
                    out.emplace(next);
                }
            }
        }
    }

    void WorldLoadThread::FilterChunks(std::unordered_set<glm::ivec3>& chunks)
    {
        ChunkGenerator* generator = world->GetChunkGenerator();

        for(auto it = chunks.begin(); it != chunks.end(); )
            if(generator->IsEmpty(*it))
                it = chunks.erase(it);
            else it++;
    }

    void WorldLoadThread::FindChunksToLoadOrUnload(std::unordered_set<glm::ivec3>& required, std::vector<glm::ivec3>& to_load, std::vector<glm::ivec3>& to_unload)
    {
        to_load.reserve(required.size());
        to_unload.reserve(required.size());

        for(auto& chunk : world->GetChunks())
        {
            if(required.find(chunk.GetCoordinates()) == required.end())
                to_unload.emplace_back(chunk.GetCoordinates());
            else required.erase(chunk.GetCoordinates());
        }

        for(const glm::ivec3& chunk_coords : required)
            to_load.emplace_back(chunk_coords);
    }

    void WorldLoadThread::SortChunks(std::vector<glm::ivec3>& chunks, const glm::ivec3& target)
    {
        auto closest_to_target = [&](const glm::ivec3& x, const glm::ivec3& y)
        {
            return glm::distance2(glm::vec3(x), glm::vec3(target)) < glm::distance2(glm::vec3(y), glm::vec3(target));
        };

        std::sort(chunks.begin(), chunks.end(), closest_to_target);
    }

    int WorldLoadThread::LoadAndUnloadChunks(const std::vector<glm::ivec3>& to_load, const std::vector<glm::ivec3>& to_unload)
    {
        std::size_t step = 64, threshold = CHUNK_SIZE * 8;
        std::size_t max_count = std::clamp((std::size_t) ((to_load.size() + to_unload.size())), (std::size_t) 64, (std::size_t) 16384);
        ChunkGenerator* generator = world->GetChunkGenerator();

        glm::vec3 old_position = { 0, 0, 0 };
        {
            std::lock_guard<std::mutex> guard(players_mutex);
            if(players.front() != nullptr)
                old_position = players.front()->GetTransform().Position();
        }

        int total = 0;
        std::size_t i = 0, j = 0, k = 0;
        while(i < to_load.size() && j < to_unload.size() && (k += 2) < max_count)
        {
            if(total % step == 0)
            {
                std::lock_guard<std::mutex> guard(players_mutex);
                if(players.front() != nullptr && glm::distance(old_position, players.front()->GetTransform().Position()) >= players.front()->GetRenderDistance() * CHUNK_SIZE / 8)
                    k = max_count;
            }

            total++;
            std::lock_guard<std::mutex> world_guard(world->GetMutex());

            Chunk& chunk = world->TransposeChunk(to_unload[j++], to_load[i]);
            generator->GenerateChunk(chunk, to_load[i++]);
            chunk.BuildGeometry();

            if(exit)
                break;
        }

        while(i < to_load.size() && k++ < max_count)
        {
            if(total % step == 0)
            {
                std::lock_guard<std::mutex> guard(players_mutex);
                if(players.front() != nullptr && glm::distance(old_position, players.front()->GetTransform().Position()) >= players.front()->GetRenderDistance() * CHUNK_SIZE / 8)
                    k = max_count;
            }

            total++;
            std::lock_guard<std::mutex> world_guard(world->GetMutex());

            Chunk& chunk = world->LoadChunk(to_load[i]);
            generator->GenerateChunk(chunk, to_load[i++]);
            chunk.BuildGeometry();

            if(exit)
                break;
        }

        while(j < to_unload.size() && k++ < max_count)
        {
            if(total % step == 0)
            {
                std::lock_guard<std::mutex> guard(players_mutex);
                if(players.front() != nullptr && glm::distance(old_position, players.front()->GetTransform().Position()) >= players.front()->GetRenderDistance() * CHUNK_SIZE / 8)
                    k = max_count;
            }

            total++;
            std::lock_guard<std::mutex> world_guard(world->GetMutex());

            world->UnloadChunk(to_unload[j++]);

            if(exit)
                break;
        }

        return total;
    }
};

