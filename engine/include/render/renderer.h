#pragma once

#include "../logger.h"
#include "../mathematics.h"

#include "../utils/optional.h"

#include "texture.h"

#include "noise.h"

#include "chunks.h"
#include "gui.h"

DECLARE_LOG_CATEGORY(OpenGLInternal);
DECLARE_LOG_CATEGORY(Renderer);

namespace Render
{
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
        void SetVPMatrix(const ::Math::mat4& vp_matrix) noexcept;

        inline GUI::Renderer& GetGUIRenderer() noexcept { return gui_renderer; }
        inline const GUI::Renderer& GetGUIRenderer() const noexcept { return gui_renderer; }

        inline Chunks::Renderer& GetChunkRenderer() noexcept { return chunk_renderer; }
        inline const Chunks::Renderer& GetChunkRenderer() const noexcept { return chunk_renderer; }

    private:
        GUI::Renderer gui_renderer;
        Chunks::Renderer chunk_renderer;

        GLFWwindow* window;
    };

    namespace __detail
    {
        class RenderContext
        {
        public:
            typedef GUI::__detail::RenderContext GUIRenderContext;
            typedef Chunks::__detail::RenderContext ChunkRenderContext;

            RenderContext();
            RenderContext(RenderContext&& other) = delete;
            RenderContext(const RenderContext& other) = delete;

            ~RenderContext();

            RenderContext& operator=(RenderContext&& other) = delete;
            RenderContext& operator=(const RenderContext& other) = delete;

            inline GUIRenderContext& GetGUIRenderContext() noexcept { return gui_render_context; }
            inline const GUIRenderContext& GetGUIRenderContext() const noexcept { return gui_render_context; }

            inline ChunkRenderContext& GetChunkRenderContext() noexcept { return chunk_render_context; }
            inline const ChunkRenderContext& GetChunkRenderContext() const noexcept { return chunk_render_context; }

        private:
            GUIRenderContext gui_render_context;
            ChunkRenderContext chunk_render_context;
        };
    };
};

