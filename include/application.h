#pragma once

#include <memory>

#include "window.h"
#include "input.h"
#include "render.h"
#include "world.h"
#include "player.h"
#include "profiler.h"
#include "events.h"

DECLARE_LOG_CATEGORY(Application);

class Application
{
public:
    Application();
    ~Application();

    void Run();

    inline Render::RenderThread& GetRenderThread() noexcept { return *render_thread; }
    inline const Render::RenderThread& GetRenderThread() const noexcept { return *render_thread; }

    template<typename EventType>
    void DispatchEvent(const EventType& event)
    {
        event_dispatcher->Dispatch(event);
    }

    template<typename EventType, typename... Args>
    void DispatchEvent(Args&&... args)
    {
        event_dispatcher->Dispatch<EventType>(std::forward<Args>(args)...);
    }

    inline static Application* Get() noexcept { return instance; }

private:
    std::shared_ptr<Profiler::ProfilerThread> profiler_thread;
    std::shared_ptr<Render::RenderThread> render_thread;
    std::shared_ptr<Game::WorldLoadThread> world_load_thread;

    std::shared_ptr<Window::Window> window;
    std::shared_ptr<Input::InputHandler> input_handler;
    std::shared_ptr<Events::EventDispatcher> event_dispatcher;

    std::shared_ptr<Game::Player> player;
    std::shared_ptr<Game::World> world;

    static Application* instance;
};
