#pragma once

#include <memory>
#include <thread>
#include <queue>
#include <vector>
#include <functional>
#include <unordered_set>

#include <glm/glm.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "config.h"
#include "logger.h"
#include "events.h"

#include "frustum.h"

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
        unsigned int encoded;
    };

    struct FaceMesh
    {
        Vertex v[4];
    };

    enum class FaceOrientation { XOY, YOZ, ZOX };

    typedef int ChunkMeshHandle;

    class Buffer
    {
    public:
        Buffer();
        Buffer(unsigned int usage);
        Buffer(Buffer&& other);
        Buffer(const Buffer& other);

        ~Buffer();

        Buffer& operator=(Buffer&& other);
        Buffer& operator=(const Buffer& other);

        void SetData(unsigned int bytes, const void* data);
        void Clear();

        void Bind(unsigned int target) const;
        void Unbind(unsigned int target) const;

        void BindBase(unsigned int target, unsigned int index) const;
        void UnbindBase(unsigned int target, unsigned int index) const;

        inline unsigned int GetUsage() const noexcept { return usage; }
        inline void SetUsage(unsigned int usage) noexcept { this->usage = usage; }

        inline unsigned int Get() const noexcept { return buffer; }
    
    private:
        unsigned int bytes, buffer, usage;
    };

    class VertexArray
    {
    public:
        VertexArray();
        VertexArray(VertexArray&& other);
        VertexArray(const VertexArray& other) = delete;

        ~VertexArray();

        VertexArray& operator=(VertexArray&& other);
        VertexArray& operator=(const VertexArray& other) = delete;

        unsigned int AddVertexBuffer(const Buffer& vbo, unsigned int offset, unsigned int stride);
        void SetElementBuffer(const Buffer& ebo);

        void AddVertexArrayAttrib(unsigned int buffer, int size, unsigned int type, unsigned int offset);

        void Clear();

        void Bind() const;
        void Unbind() const;

        inline unsigned int Get() const noexcept { return vao; }

    private:
        unsigned int vao, vertex_buffers, attribs;
    };
    
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
        void Clear();

        inline void Bind() const { vao.Bind(); }
        inline void Unbind() const { vao.Unbind(); }

        inline unsigned int Size() const noexcept { return size; }

    private:
        void BuildVAO();

    private:
        VertexArray vao;
        Buffer buffer;
        unsigned int size;
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
        void Clear(FaceOrientation orientation);

        inline void Bind(unsigned int buffer) const { buffers[buffer].Bind(); }
        inline void Unbind(unsigned int buffer) const { buffers[buffer].Unbind(); }

        inline unsigned int Size() const noexcept { return 3; }
        inline unsigned int BufferSize(unsigned int i) const noexcept { return buffers[i].Size(); }

        inline void SetModelMatrix(const glm::mat4& model_matrix) noexcept { this->model_matrix = model_matrix; }
        inline const glm::mat4& GetModelMatrix() const noexcept { return model_matrix; }

        inline void SetHandle(ChunkMeshHandle handle) noexcept { this->handle = handle; }
        inline ChunkMeshHandle GetHandle() const noexcept { return handle; }

    private:
        FaceBuffer buffers[3];
        glm::mat4 model_matrix;
        ChunkMeshHandle handle;
    };

    class Shader
    {
    public:
        Shader(unsigned int type);
        Shader(Shader&& other);
        Shader(const Shader& other) = delete;

        ~Shader();

        Shader& operator=(Shader&& other);
        Shader& operator=(const Shader& other) = delete;

        void SetSource(const char* src);
        bool Compile();

        inline unsigned int Get() const noexcept { return shader; }

    private:
        unsigned int shader;
    };

    class Program
    {
    public:
        Program();
        Program(Program&& other);
        Program(const Program& other) = delete;

        ~Program();

        Program& operator=(Program&& other);
        Program& operator=(const Program& other) = delete;

        void Attach(const Shader& shader);
        void Detach(const Shader& shader);

        bool Link();

        int GetUniformLocation(const char* name) const;
        unsigned int GetUniformBlockIndex(const char* name) const;
        unsigned int GetShaderStorageBlockIndex(const char* name) const;

        void BindUniformBlock(unsigned int target, unsigned int binding) const;
        void UnbindUniformBlock(unsigned int target) const;

        void BindShaderStorageBlock(unsigned int target, unsigned int binding) const;
        void UnbindShaderStorageBlock(unsigned int target) const;

        void Bind() const;
        void Unbind() const;

        inline unsigned int Get() const noexcept { return program; }

    private:
        unsigned int program;
    };

    class VFShader : public Program
    {
    public:
        VFShader(const char* vertex_src, const char* fragment_src);
        VFShader(VFShader&& other) = default;
        VFShader(const VFShader& other) = delete;

        ~VFShader() = default;

        VFShader& operator=(VFShader&& other) = default;
        VFShader& operator=(const VFShader& other) = delete;
    };

    class ComputeShader : public Program
    {
    public:
        ComputeShader(const char* src);
        ComputeShader(ComputeShader&& other) = default;
        ComputeShader(const ComputeShader& other) = delete;

        ~ComputeShader() = default;

        ComputeShader& operator=(ComputeShader&& other) = default;
        ComputeShader& operator=(const ComputeShader& other) = delete;
    };

    enum class TextureStyle { PIXELATED, SMOOTH };

    enum class TextureFormatType { RED, RGB, RGBA };
    enum class TextureFormat { R8, R32, RGB8, RGB32, RGBA8, RGBA32 };

    class Texture2D
    {
    public:
        Texture2D(unsigned int levels, TextureFormat format, unsigned int width, unsigned int height);
        Texture2D(Texture2D&& other);
        Texture2D(const Texture2D& other) = delete;

        ~Texture2D();

        Texture2D& operator=(Texture2D&& other);
        Texture2D& operator=(const Texture2D& other) = delete;

        void SetStyle(TextureStyle style);

        void SetData(const unsigned char* data, unsigned int level, TextureFormatType format);
        void SetData(const unsigned char* data, unsigned int level, TextureFormatType format, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
        
        void CopyData(const Texture2D& other, unsigned int src_level, unsigned int src_x, unsigned int src_y, unsigned int dst_level, unsigned int dst_x, unsigned int dst_y, unsigned int width, unsigned int height);
        void GetData(unsigned int level, TextureFormat format, unsigned int size, void* out);

        void GenerateMipmaps();

        void BindImage(unsigned int unit, unsigned int level, unsigned int access) const;
        void UnbindImage(unsigned int unit) const;

        void Bind(unsigned int slot) const;
        void Unbind(unsigned int slot) const;

        inline unsigned int GetLevels() const noexcept { return levels; }
        inline unsigned int GetWidth() const noexcept { return width; }
        inline unsigned int GetHeight() const noexcept { return height; }
        inline TextureFormat GetFormat() const noexcept { return format; }
        inline TextureStyle GetStyle() const noexcept { return style; }

        inline unsigned int GetInternalTexture() const noexcept { return texture; }

    private:
        unsigned int texture;
        TextureFormat format;
        unsigned int width, height, levels;
        TextureStyle style;
    };

    class TextureAtlas
    {
    public:
        TextureAtlas();
        TextureAtlas(TextureAtlas&& other);
        TextureAtlas(const TextureAtlas& other) = delete;
        
        ~TextureAtlas();

        TextureAtlas& operator=(TextureAtlas&& other);
        TextureAtlas& operator=(const TextureAtlas& other) = delete;

        void SetTextures(unsigned int count, const Texture2D* textures);

        inline void Bind(unsigned int slot) const { texture->Bind(slot); }
        inline void Unbind(unsigned int slot) const { texture->Unbind(slot); }

        inline glm::ivec2 GetTextureSize() const noexcept { return { subwidth, subheight }; }
        inline glm::ivec2 GetTableSize() const noexcept { return { width, height }; }
        inline glm::ivec2 GetAtlasSize() const noexcept { return { width * subwidth, height * subheight }; }

        inline Texture2D& GetTexture() { return texture.value(); }

    private:
        glm::ivec2 CalculateDimensions(unsigned int count) const;

    private:
        std::optional<Texture2D> texture;
        unsigned int subwidth, subheight;
        unsigned int width, height, size;
    };

    struct Perlin2DGradient
    {
        glm::vec2 gradient;
    };

    class Perlin2DGenerator
    {
    public:
        Perlin2DGenerator();
        Perlin2DGenerator(Perlin2DGenerator&& other);
        Perlin2DGenerator(const Perlin2DGenerator& other) = delete;

        ~Perlin2DGenerator();

        Perlin2DGenerator& operator=(Perlin2DGenerator&& other);
        Perlin2DGenerator& operator=(const Perlin2DGenerator& other) = delete;

        void SetGradients(unsigned int x, unsigned int y, const Perlin2DGradient* gradients);
        void Generate(unsigned int width, unsigned int height);

        inline Texture2D& GetTexture() { return texture.value(); }

    private:
        unsigned int gradients_x, gradients_y;
        
        std::optional<Buffer> gradient_buffer;
        std::optional<ComputeShader> shader;
        std::optional<Texture2D> texture;

        unsigned int gradients_index;
        int resolution_location, gradients_dimensions_location;
    };


    class ImGuiWindow
    {
    public:
        virtual void Draw(bool* opened) = 0;

        void Render();

        inline bool IsOpened() const noexcept { return opened; }
        inline void SetOpened(bool opened) noexcept { this->opened = opened; }

    protected:
        std::atomic<bool> opened = false;
    };

    class ImGuiDemoWindow : public ImGuiWindow
    {
    public:
        virtual void Draw(bool* opened) override;
    };

    class ImGuiPerlin2DNoiseVisualizerWindow : public ImGuiWindow
    {
    public:
        ImGuiPerlin2DNoiseVisualizerWindow();
        ImGuiPerlin2DNoiseVisualizerWindow(ImGuiPerlin2DNoiseVisualizerWindow&& other) = delete;
        ImGuiPerlin2DNoiseVisualizerWindow(const ImGuiPerlin2DNoiseVisualizerWindow& other) = delete;

        ~ImGuiPerlin2DNoiseVisualizerWindow();

        ImGuiPerlin2DNoiseVisualizerWindow& operator=(ImGuiPerlin2DNoiseVisualizerWindow&& other) = delete;
        ImGuiPerlin2DNoiseVisualizerWindow& operator=(const ImGuiPerlin2DNoiseVisualizerWindow& other) = delete;

        virtual void Draw(bool* opened) override;

    private:
        void Generate();
    
    private:
        Perlin2DGenerator generator;
        int resolution;
    };

    class ImGuiRenderer
    {
    public:
        ImGuiRenderer(GLFWwindow* window);
        ImGuiRenderer(ImGuiRenderer&& other) = delete;
        ImGuiRenderer(const ImGuiRenderer& other) = delete;

        ~ImGuiRenderer();

        ImGuiRenderer& operator=(ImGuiRenderer&& other) = delete;
        ImGuiRenderer& operator=(const ImGuiRenderer& other) = delete;

        void Begin();
        void End();

    private:
        GLFWwindow* window;
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

        inline TextureAtlas& GetAtlas() { return atlas.value(); }

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

    class ImGuiTextureWindow : public ImGuiWindow
    {
    public:
        ImGuiTextureWindow(const char* name, const Texture2D& texture);

        virtual void Draw(bool* opened) override;

    private:
        std::string name;
        unsigned int texture;
        unsigned int width, height;
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

        inline ImGuiRenderer& GetImGuiRenderer() { return imgui_renderer.value(); }
        inline ChunkRenderer& GetChunkRenderer() { return chunk_renderer.value(); }

    private:
        std::optional<ImGuiRenderer> imgui_renderer;
        std::optional<ChunkRenderer> chunk_renderer;

        GLFWwindow* window;
    };

    class RenderThread;

    namespace __detail
    {
        class ChunkMeshPool
        {
        public:
            typedef ChunkMeshHandle Handle;

            ChunkMeshPool();
            ChunkMeshPool(ChunkMeshPool&& other);
            ChunkMeshPool(const ChunkMeshPool& other);

            ~ChunkMeshPool();

            ChunkMeshPool& operator=(ChunkMeshPool&& other);
            ChunkMeshPool& operator=(const ChunkMeshPool& other);

            Handle New();
            void Free(Handle handle);

            inline bool Has(Handle handle) const { return meshes.find(handle) != meshes.end(); }

            inline std::shared_ptr<ChunkMesh>& At(Handle handle) { return meshes.at(handle); }
            inline const std::shared_ptr<ChunkMesh>& At(Handle handle) const { return meshes.at(handle); }

        private:
            std::unordered_map<Handle, std::shared_ptr<ChunkMesh>> meshes;
        };
 
        class RenderContext
        {
        public:
            RenderContext();
            RenderContext(RenderContext&& other) = delete;
            RenderContext(const RenderContext& other) = delete;

            ~RenderContext();

            RenderContext& operator=(RenderContext&& other) = delete;
            RenderContext& operator=(const RenderContext& other) = delete;
        
            ChunkMeshHandle NewChunkMesh();
            void FreeChunkMesh(ChunkMeshHandle handle);

            inline const std::shared_ptr<ChunkMesh>& Get(ChunkMeshHandle handle) const { return chunk_mesh_pool.At(handle); }

            inline bool IsChunkInRenderQueue(ChunkMeshHandle handle) const noexcept { return in_render_queue.find(handle) != in_render_queue.end(); }

            void AddChunkToRenderQueue(const std::shared_ptr<ChunkMesh>& mesh);
            void AddChunkToRenderQueue(ChunkMeshHandle handle);

            void RemoveChunkFromRenderQueue(const std::shared_ptr<ChunkMesh>& mesh);
            void RemoveChunkFromRenderQueue(ChunkMeshHandle handle);

            void AddWindowToRenderQueue(const std::shared_ptr<ImGuiWindow>& window);
            void RemoveWindowFromRenderQueue(const std::shared_ptr<ImGuiWindow>& window);

            void GeneratePerlin2DNoise(const glm::uvec2& resolution, const glm::uvec2& output_size, float* out);

        private:
            void InternalRemoveChunkFromRenderQueue(std::size_t idx);
            void InternalRemoveWindowFromRenderQueue(std::size_t idx);

        private:
            ChunkMeshPool chunk_mesh_pool;
            std::vector<std::pair<ChunkMeshHandle, std::weak_ptr<ChunkMesh>>> render_queue;
            std::unordered_map<ChunkMeshHandle, std::size_t> in_render_queue;

            std::vector<std::weak_ptr<ImGuiWindow>> window_queue;

            std::optional<Perlin2DGenerator> perlin2D_generator;

            friend class ::Render::RenderThread;
        };
    };

    struct FramebufferEvent
    {
        int x, y, width, height;
    };

    class RenderThread : public Events::EventHandler<FramebufferEvent>
    {
    public:
        typedef __detail::RenderContext RenderContext;
        typedef std::function<void(RenderContext&)> Command;

        RenderThread(GLFWwindow* window);
        RenderThread(RenderThread&& other) = delete;
        RenderThread(const RenderThread& other) = delete;

        ~RenderThread();

        RenderThread& operator=(RenderThread&& other) = delete;
        RenderThread& operator=(const RenderThread& other) = delete;

        virtual void Handle(const FramebufferEvent& framebuffer_event) override;

        void Execute(Command&& command);

        void SetViewport(float x, float y, float width, float height);
        void SetPlayer(const std::shared_ptr<Game::Player>& player);

        int GetBlockTextureID(const Game::Face& face);

        void SetChunkDrawMode(ChunkRenderer::DrawMode draw_mode);

        inline const std::atomic<bool>& IsInitialized() const noexcept { return initialized; }
    
    private:
        void Run(GLFWwindow* window);

        void InitViewport(GLFWwindow* window);
        void InitImGuiWindows();

        void ProcessCommands();
        void UpdateViewport();
        glm::mat4 GetVPMatrix();
        int RenderChunks(const Math::Frustum& frustum);
        bool IsChunkOnScreen(const std::shared_ptr<ChunkMesh>& mesh, const Math::Frustum& frustum);

        void RenderImGui();

        void DestroyImGuiWindows();

    private:
        std::thread thread;
        std::atomic<bool> exit;
        std::atomic<bool> initialized;
        
        RenderContext render_context;
        std::queue<Command> command_queue;
        std::mutex command_queue_mutex;

        std::optional<Renderer> renderer;
        mutable std::mutex imgui_render_mutex;

        std::shared_ptr<ImGuiDemoWindow> imgui_demo_window;
        std::shared_ptr<ImGuiTextureWindow> imgui_block_texture_atlas_window;
        std::shared_ptr<ImGuiPerlin2DNoiseVisualizerWindow> imgui_perlin2D_noise_visualizer_window;
        mutable std::mutex chunk_render_mutex;

        std::shared_ptr<Game::Player> player;
        mutable std::mutex player_mutex;

        std::atomic<bool> viewport_changed;
        int vp_x, vp_y, vp_width, vp_height;
        mutable std::mutex viewport_mutex;
    };
};

