#pragma once

#include "../logger.h"
#include "../gui.h"

#include <GLFW/glfw3.h>

DECLARE_LOG_CATEGORY(GUIRenderer);

namespace Render
{
    namespace GUI
    {
        namespace __detail
        {
            class RenderContext
            {
            public:
                RenderContext();
                RenderContext(RenderContext&& other) = delete;
                RenderContext(const RenderContext& other) = delete;

                ~RenderContext();

                RenderContext& operator=(RenderContext&& other) = delete;
                RenderContext& operator=(const RenderContext& other) = delete;

                inline ::GUI::WindowManager& GetWindowManager() noexcept { return window_manager; }
                inline const ::GUI::WindowManager& GetWindowManager() const noexcept { return window_manager; }

            private:
                ::GUI::WindowManager window_manager;
                GLFWwindow* window;
                ImFont* font;
            };
        };

        class Renderer
        {
        public:
            typedef __detail::RenderContext RenderContext;

            Renderer(GLFWwindow* window);
            Renderer(Renderer&& other) = delete;
            Renderer(const Renderer& other) = delete;

            ~Renderer();

            Renderer& operator=(Renderer&& other) = delete;
            Renderer& operator=(const Renderer& other) = delete;

            void Begin();
            void End();

            inline RenderContext& GetRenderContext() noexcept { return render_context; }
            inline const RenderContext& GetRenderContext() const noexcept { return render_context; }

        private:
            RenderContext render_context;
        };
    };
};

