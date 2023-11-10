#pragma once

#include <queue>
#include <functional>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "config.h"
#include "logger.h"
#include "events.h"
#include "gui.h"

#include "mathematics.h"
#include "frustum.h"

#include "utils/array.h"
#include "utils/string.h"
#include "utils/hash.h"
#include "utils/memory.h"
#include "utils/thread.h"

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

            void GeneratePerlin2DNoise(const glm::uvec2& resolution, const glm::uvec2& output_size, float* out);
            void GeneratePerlin2DNoise(const glm::uvec2& resolution, Texture2D& out);

            inline ::GUI::WindowManager& GetGUIWindowManager() { return gui_window_manager.value(); }

        private:
            void InternalRemoveChunkFromRenderQueue(std::size_t idx);

        private:
            ChunkMeshPool chunk_mesh_pool;
            ::Utils::Array<::std::pair<ChunkMeshHandle, ::Utils::WeakPointer<ChunkMesh>>> render_queue;
            ::Utils::HashMap<ChunkMeshHandle, ::Utils::USize> in_render_queue;

            ::std::optional<Perlin2DGenerator> perlin2D_generator;
            ::std::optional<::GUI::WindowManager> gui_window_manager;

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
        void InitGUIWindows();

        void ProcessCommands();
        void UpdateViewport();
        glm::mat4 GetVPMatrix();
        int RenderChunks(const Math::Frustum& frustum);
        bool IsChunkOnScreen(const std::shared_ptr<ChunkMesh>& mesh, const Math::Frustum& frustum);

        void RenderGUI();

        void DestroyGUIWindows();

    private:
        std::thread thread;
        std::atomic<bool> exit;
        std::atomic<bool> initialized;
        
        RenderContext render_context;
        std::queue<Command> command_queue;
        std::mutex command_queue_mutex;

        std::optional<Renderer> renderer;
        mutable std::mutex imgui_render_mutex;
        mutable std::mutex chunk_render_mutex;

        std::shared_ptr<Game::Player> player;
        mutable std::mutex player_mutex;

        std::atomic<bool> viewport_changed;
        int vp_x, vp_y, vp_width, vp_height;
        mutable std::mutex viewport_mutex;
    };
};

