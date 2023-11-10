#pragma once

#include "window.h"
#include "input.h"
#include "render.h"
#include "world.h"
#include "player.h"
#include "profiler.h"
#include "events.h"

#include "utils/memory.h"

DECLARE_LOG_CATEGORY(Application);

class ApplicationHandler : public ::Events::EventHandler<::Input::KeyEvent>
{
public:
    virtual void Handle(const ::Input::KeyEvent& key_event) override;
};

class Application
{
public:
    Application();
    ~Application();

    virtual void Init() {}
    virtual void Update() {}
    virtual void Destroy() {}

    void Run();

    inline ::Render::RenderThread& GetRenderThread() noexcept { return *render_thread; }
    inline const ::Render::RenderThread& GetRenderThread() const noexcept { return *render_thread; }

    inline ::Input::InputHandler& GetInputHandler() noexcept { return *input_handler; }
    inline const ::Input::InputHandler& GetInputHandler() const noexcept { return *input_handler; }

    template<typename EventType>
    void DispatchEvent(const EventType& event)
    {
        event_dispatcher->Dispatch(event);
    }

    template<typename EventType, typename... Args>
    void DispatchEvent(Args&&... args)
    {
        event_dispatcher->Dispatch<EventType>(::std::forward<Args>(args)...);
    }

    template<typename Handler>
    void RegisterHandler(const Handler& handler)
    {
        event_dispatcher->RegisterHandler(handler);
    }

    inline static Application* Get() noexcept { return instance; }

private:
    ::Utils::SharedPointer<::Profiler::ProfilerThread> profiler_thread;
    ::Utils::SharedPointer<::Render::RenderThread> render_thread;
    ::Utils::SharedPointer<::Game::WorldLoadThread> world_load_thread;

    ::Utils::SharedPointer<::Window::Window> window;
    ::Utils::SharedPointer<ApplicationHandler> application_handler;
    ::Utils::SharedPointer<::Input::InputHandler> input_handler;
    ::Utils::SharedPointer<::Events::EventDispatcherMT> event_dispatcher;

    ::Utils::SharedPointer<::Game::Player> player;
    ::Utils::SharedPointer<::Game::World> world;

    static Application* instance;
};

