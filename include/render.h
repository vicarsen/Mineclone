#pragma once

#include <memory>
#include <thread>
#include <queue>
#include <vector>
#include <unordered_set>

#include <glm/glm.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "config.h"
#include "logger.h"
#include "events.h"

DECLARE_LOG_CATEGORY(OpenGLInternal);
DECLARE_LOG_CATEGORY(Shaders);
DECLARE_LOG_CATEGORY(FaceBuffer);
DECLARE_LOG_CATEGORY(ChunkMesh);
DECLARE_LOG_CATEGORY(Renderer);
DECLARE_LOG_CATEGORY(ChunkRenderer);

namespace Game
{
    class Chunk;
    class World;
    class Player;
    class Face;
    class Transform;
};

namespace Render
{
    struct Vertex
    {
        int encoded;
    };

    struct FaceMesh
    {
        Vertex v[4];
    };

    enum class FaceOrientation { XOY, YOZ, ZOX };
    
    class FaceBuffer
    {
    public:
        FaceBuffer();
        FaceBuffer(FaceBuffer&& other);
        FaceBuffer(const FaceBuffer& other);
        ~FaceBuffer();

        FaceBuffer& operator=(FaceBuffer&& other);
        FaceBuffer& operator=(const FaceBuffer& other);

        void SetData(unsigned int count, const FaceMesh* meshes);

        void Bind() const;

        inline unsigned int Size() const noexcept { return size; }

    private:
        unsigned int size;
        unsigned int vao, buffer;
    };

    class ChunkMesh
    {
    public:
        ChunkMesh();
        ChunkMesh(ChunkMesh&& other);
        ChunkMesh(const ChunkMesh& other);

        ~ChunkMesh();

        ChunkMesh& operator=(ChunkMesh&& other);
        ChunkMesh& operator=(const ChunkMesh& other);

        void SetFaces(unsigned int count, const FaceMesh* meshes, FaceOrientation orientation);

        void Bind(unsigned int buffer) const;

        inline unsigned int Size() const noexcept { return 3; }
        inline unsigned int BufferSize(unsigned int i) const noexcept { return buffers[i].Size(); }

        inline void SetModelMatrix(const glm::mat4& model_matrix) noexcept { this->model_matrix = model_matrix; }
        inline const glm::mat4& GetModelMatrix() const noexcept { return model_matrix; }

        inline void SetHandle(int handle) noexcept { this->handle = handle; }
        inline int GetHandle() const noexcept { return handle; }

    private:
        FaceBuffer buffers[3];
        glm::mat4 model_matrix;
        int handle;
    };

    class VFShader
    {
    public:
        VFShader(const char* vertex_src, const char* fragment_src);
        VFShader(VFShader&& other);
        VFShader(const VFShader& other) = delete;

        ~VFShader();

        VFShader& operator=(VFShader&& other);
        VFShader& operator=(const VFShader& other) = delete;

        void Bind() const;

        int GetUniformLocation(const char* name);

    private:
        unsigned int program;
    };

    enum class TextureStyle { PIXELATED, SMOOTH };

    class Texture2D
    {
    public:
        Texture2D(unsigned int levels, unsigned int channels, unsigned int width, unsigned int height);
        Texture2D(Texture2D&& other);
        Texture2D(const Texture2D& other) = delete;

        ~Texture2D();

        Texture2D& operator=(Texture2D&& other);
        Texture2D& operator=(const Texture2D& other) = delete;

        void SetStyle(TextureStyle style);

        void SetData(const unsigned char* data, unsigned int level, unsigned int channels);
        void SetData(const unsigned char* data, unsigned int level, unsigned int channels, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
        
        void CopyData(const Texture2D& other, unsigned int src_level, unsigned int src_x, unsigned int src_y, unsigned int dst_level, unsigned int dst_x, unsigned int dst_y, unsigned int width, unsigned int height);

        void GenerateMipmaps();

        void Bind(unsigned int slot) const;

        inline unsigned int GetLevels() const noexcept { return levels; }
        inline unsigned int GetWidth() const noexcept { return width; }
        inline unsigned int GetHeight() const noexcept { return height; }
        inline unsigned int GetChannelCount() const noexcept { return channels; }
        inline TextureStyle GetStyle() const noexcept { return style; }

    private:
        unsigned int texture;
        unsigned int width, height, channels, levels, format;
        TextureStyle style;
    };

    class TextureAtlas
    {
    public:
        static inline const unsigned int STEP = 16;
        static inline const unsigned int HALF_STEP = 8;

        TextureAtlas();
        TextureAtlas(TextureAtlas&& other);
        TextureAtlas(const TextureAtlas& other) = delete;
        
        ~TextureAtlas();

        TextureAtlas& operator=(TextureAtlas&& other);
        TextureAtlas& operator=(const TextureAtlas& other) = delete;

        void SetTextures(unsigned int count, const Texture2D* textures);

        void Bind(unsigned int slot) const;

        inline glm::ivec2 GetTextureSize() const noexcept { return { subwidth, subheight }; }
        inline glm::ivec2 GetTableSize() const noexcept { return { width, height }; }
        inline glm::ivec2 GetAtlasSize() const noexcept { return { width * subwidth, height * subheight }; }

    private:
        glm::ivec2 CalculateDimensions(unsigned int count) const;

    private:
        std::optional<Texture2D> texture;
        unsigned int subwidth, subheight;
        unsigned int width, height, size;
    };

    class ChunkRenderer
    {
    public:
        enum class DrawMode { NORMAL, WIREFRAME };

    public:
        ChunkRenderer();
        ChunkRenderer(ChunkRenderer&& other) = delete;
        ChunkRenderer(const ChunkRenderer& other) = delete;

        ~ChunkRenderer();

        ChunkRenderer& operator=(ChunkRenderer&& other) = delete;
        ChunkRenderer& operator=(const ChunkRenderer& other) = delete;

        void Begin();
        void Begin(FaceOrientation orientation);
        void Render(const ChunkMesh& mesh);
        void End();

        int GetTextureID(const Game::Face& face);

        inline void SetVPMatrix(const glm::mat4& vp_matrix) noexcept { this->vp_matrix = vp_matrix; }

        inline void SetDrawMode(DrawMode mode) noexcept { draw_mode = mode; }

    private:
        struct BlockData
        {
            int north = -1, south = -1, east = -1, west = -1, up = -1, down = -1;
        };

        struct ChunkShader
        {
            VFShader shader;
            int model_location;
            int vp_location;
            int atlas_dimensions_location;
            int texture_size_location;
            int texture_location;
        };

    private:
        std::optional<Texture2D> LoadTexture(const char* path);
        std::optional<ChunkShader> LoadShader(const char* vertex_path, const char* fragment_path);

    private:
        std::optional<TextureAtlas> atlas;
        std::optional<ChunkShader> chunk_shaders[3];
        std::optional<ChunkShader> wireframe_shader;

        glm::mat4 vp_matrix;

        std::vector<BlockData> blocks;

        unsigned int active_pipeline;
        DrawMode draw_mode;
    };

    struct FramebufferEvent
    {
        int x, y, width, height;
    };

    class Renderer
    {
    public:
        Renderer(GLFWwindow* window);
        Renderer(Renderer&& other) = delete;
        Renderer(const Renderer& other) = delete;

        ~Renderer();

        Renderer& operator=(Renderer&& other) = delete;
        Renderer& operator=(const Renderer& other) = delete;

        void Begin();
        void End();

        void SetViewport(float x, float y, float width, float height);
        void SetVPMatrix(const glm::mat4& vp_matrix) noexcept;

        inline ChunkRenderer& GetChunkRenderer() { return chunk_renderer.value(); }

    private:
        std::optional<ChunkRenderer> chunk_renderer;
        GLFWwindow* window;
    };

    namespace __detail
    {
        class ChunkMeshPool
        {
        public:
            typedef int Handle;

            ChunkMeshPool();
            ChunkMeshPool(ChunkMeshPool&& other);
            ChunkMeshPool(const ChunkMeshPool& other);

            ~ChunkMeshPool();

            ChunkMeshPool& operator=(ChunkMeshPool&& other);
            ChunkMeshPool& operator=(const ChunkMeshPool& other);

            // "Client" side; called from any thread.
            Handle New();
            void Free(Handle handle);

            inline bool Has(Handle handle) const { return sparse[handle] != -1 && meshes.size() > sparse[handle]; }

            void SetFaces(Handle handle, std::vector<FaceMesh>& faces, FaceOrientation orientation);
            void SetTransform(Handle handle, const Game::Transform& transform);
            
            // "Server" side; called from the render thread only.
            void GarbageCollect();
            void HandleRequests();

            inline ChunkMesh& operator[](Handle handle) { return meshes[sparse[handle]]; }
            inline const ChunkMesh& operator[](Handle handle) const { return meshes[sparse[handle]]; }

            inline ChunkMesh& at(Handle handle) { return meshes.at(sparse.at(handle)); }
            inline const ChunkMesh& at(Handle handle) const { return meshes.at(sparse.at(handle)); }

            inline std::mutex& GetMutex() const { return mutex; }

        private:
            struct AllocRequest
            {
                std::atomic<bool>* fence;
                Handle* value;
            };

            struct SetFacesRequest
            {
                Handle handle;
                std::vector<FaceMesh> faces;
                FaceOrientation orientation;
            };

        private:
            std::vector<ChunkMesh> meshes;
            std::vector<int> sparse;
            std::queue<int> available;

            std::queue<AllocRequest> alloc_requests;
            std::queue<SetFacesRequest> set_faces_requests;

            mutable std::mutex mutex;
        };
    };

    class RenderThread : public Events::EventHandler<FramebufferEvent>
    {
    public:
        typedef __detail::ChunkMeshPool ChunkMeshPool;
        typedef ChunkMeshPool::Handle ChunkMeshHandle;

        RenderThread(GLFWwindow* window);
        RenderThread(RenderThread&& other) = delete;
        RenderThread(const RenderThread& other) = delete;

        ~RenderThread();

        RenderThread& operator=(RenderThread&& other) = delete;
        RenderThread& operator=(const RenderThread& other) = delete;

        virtual void Handle(const FramebufferEvent& framebuffer_event) override;

        void SetViewport(float x, float y, float width, float height);

        void SetPlayer(const std::shared_ptr<Game::Player>& player);

        ChunkMeshHandle NewChunkMesh();
        void FreeChunkMesh(ChunkMeshHandle handle);

        void SetChunkFaces(ChunkMeshHandle handle, std::vector<FaceMesh>& faces, FaceOrientation orientation);
        void SetChunkTransform(ChunkMeshHandle handle, const Game::Transform& transform);

        int GetBlockTextureID(const Game::Face& face);

        void AddChunkToDrawCall(ChunkMeshHandle handle);
        void RemoveChunkFromDrawCall(ChunkMeshHandle handle);

        void SetChunkDrawMode(ChunkRenderer::DrawMode draw_mode);

        inline const std::atomic<bool>& IsInitialized() const noexcept { return initialized; }
    
    private:
        void Run(GLFWwindow* window);

    private:
        std::thread thread;
        std::atomic<bool> exit;

        std::atomic<bool> initialized;

        ChunkMeshPool chunk_mesh_pool;

        std::optional<Renderer> renderer;

        std::unordered_set<ChunkMeshHandle> chunks_draw_set;
        mutable std::mutex chunks_draw_set_mutex;

        std::shared_ptr<Game::Player> player;
        mutable std::mutex player_mutex;

        mutable std::mutex chunk_render_mutex;

        std::atomic<bool> viewport_changed;
        int vp_x, vp_y, vp_width, vp_height;
        mutable std::mutex viewport_mutex;
    };
};

