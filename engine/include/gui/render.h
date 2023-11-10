#pragma once

#include "../gui.h"
#include "../render.h"

#include "../events.h"

#include "../utils/string.h"

namespace GUI
{
    namespace Render
    {
        class RenderDemoWindow : public Window
        {
        public:
            RenderDemoWindow(::Render::RenderThread::RenderContext* render_context);
            virtual ~RenderDemoWindow() = default;

            virtual void Draw() override final;

        private:
            ::Render::RenderThread::RenderContext* render_context;
        };

        struct GameDebugInfoEvent
        {
            float fps;
            ::Math::vec3 player_position;
        };

        struct WorldGenerationInfoEvent
        {
            float fps;
            unsigned int chunks_generated;
        };

        struct RenderDebugInfoEvent
        {
            float fps;
            unsigned int chunks_rendered;
        };

        class DebugInfoOverlayWindow : public Window, public ::Events::EventHandler<GameDebugInfoEvent, WorldGenerationInfoEvent, RenderDebugInfoEvent>
        {
        public:
            DebugInfoOverlayWindow();
            virtual ~DebugInfoOverlayWindow() = default;

            virtual void Draw() override final;

            virtual void Handle(const GameDebugInfoEvent& event) override final;
            virtual void Handle(const WorldGenerationInfoEvent& event) override final;
            virtual void Handle(const RenderDebugInfoEvent& event) override final;

        private:
            float game_fps;
            ::Math::vec3 player_position;
            
            float world_generation_fps;
            unsigned int chunks_generated;

            float render_fps;
            unsigned int chunks_rendered;
        };
    };
};

