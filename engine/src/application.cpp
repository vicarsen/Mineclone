#include "application.h"

#include "window.h"
#include "input.h"

#include "events.h"

#include "gui/render.h"

DEFINE_LOG_CATEGORY(Application, CONSOLE_LOGGER(trace));

Application* Application::instance = nullptr;

static void PlaceLayer(std::shared_ptr<Game::Chunk>& chunk, int layer, Game::BlockID block)
{
    for(int x = 0; x < CHUNK_SIZE; x++)
        for(int z = 0; z < CHUNK_SIZE; z++)
            chunk->PlaceBlock({ x, layer, z }, block);
}

Application::Application()
{
    instance = this;

    event_dispatcher = std::make_shared<Events::EventDispatcherMT>();

    window = std::make_shared<Window::Window>(800, 600, "Mineclone");

    application_handler = std::make_shared<ApplicationHandler>();
    event_dispatcher->RegisterHandler(application_handler);

    input_handler = std::make_shared<Input::InputHandler>(window->GetInternalWindow());
    event_dispatcher->RegisterHandler(input_handler);

    profiler_thread = std::make_shared<Profiler::ProfilerThread>();

    render_thread = std::make_shared<Render::RenderThread>(window->GetInternalWindow());
    while(!render_thread->IsInitialized());
    render_thread->SetChunkDrawMode(Render::ChunkRenderer::DrawMode::NORMAL);
    event_dispatcher->RegisterHandler(render_thread);

    player = std::make_shared<Game::Player>(90.0f, window->GetAspectRatio());
    event_dispatcher->RegisterHandler(player);
    render_thread->SetPlayer(player);

    world_load_thread = std::make_shared<Game::WorldLoadThread>();
    world_load_thread->AddPlayer(player);

    std::unique_ptr<Game::ChunkGenerator> generator = std::make_unique<Game::Perlin2DChunkGenerator>(glm::uvec2{ 256, 256 }, 8, 32, 128);
    world = std::make_shared<Game::World>(generator);
    world_load_thread->SetWorld(world);
}

Application::~Application()
{
    world.reset();
    world_load_thread.reset();

    player.reset();

    render_thread.reset();

    event_dispatcher.reset();

    profiler_thread.reset();

    input_handler.reset();
    window.reset();

    instance = nullptr;
}

void ApplicationHandler::Handle(const Input::KeyEvent& key_event)
{
    if(key_event.key == GLFW_KEY_ESCAPE && key_event.action == Input::InputAction::RELEASED)
    {
        Application::Get()->GetInputHandler().ToggleCursor();
        Application::Get()->GetInputHandler().ToggleCaptureCursorMovement();
    }
}

void Application::Run()
{
    Init();

    const float player_speed = 5.0f, player_warp_speed = 200.0f, player_sensitivity = 0.1f;
    float player_pitch = 0.0f;
    float player_yaw = 0.0f;
    
    float last_time = glfwGetTime(), delta_time = 0.0f;
    float last_print = glfwGetTime();
    int count = 0;

    while(!window->ShouldClose())
    {
        PROFILE_THREAD(MainThread);

        float update_start_time = glfwGetTime();

        input_handler->Reset();
        glfwPollEvents();
        input_handler->Update();

        event_dispatcher->ProcessEvents();

        float update_end_time = glfwGetTime();
        float update_time = update_end_time - update_start_time;

        float sleep_time = (update_time < 1.0f / 144.0f ? 1.0f / 144.0f - update_time : 0.0f);

        if(sleep_time != 0.0f)
            std::this_thread::sleep_for(std::chrono::microseconds((long long) (sleep_time * 1'000'000.0f)));

        float time = glfwGetTime();
        delta_time = time - last_time;
        last_time = time;

        glm::vec2 movement = input_handler->GetCursorMovement() * player_sensitivity;
        player_pitch = std::clamp(player_pitch - movement.y, -89.0f, 89.0f);
        player_yaw = glm::mod(player_yaw - movement.x, 360.0f);

        player->GetTransform().Rotation() = glm::quat({ glm::radians(player_pitch), glm::radians(player_yaw), 0.0f });

        glm::vec3 forward = player->GetTransform().Forward();
        forward.y = 0.0f; forward = glm::normalize(forward);

        glm::vec3 right = player->GetTransform().Right();
        right = glm::normalize(right);

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        float speed = (input_handler->IsKeyPressed(GLFW_KEY_LEFT_CONTROL) ? player_warp_speed : player_speed);

        if(input_handler->IsKeyPressed(GLFW_KEY_A)) {
            player->GetTransform().Position() -= delta_time * speed * right;
        }
        if(input_handler->IsKeyPressed(GLFW_KEY_D)) {
            player->GetTransform().Position() += delta_time * speed * right;
        }
        if(input_handler->IsKeyPressed(GLFW_KEY_W)) {
            player->GetTransform().Position() += delta_time * speed * forward;
        }
        if(input_handler->IsKeyPressed(GLFW_KEY_S)) {
            player->GetTransform().Position() -= delta_time * speed * forward;
        }
        if(input_handler->IsKeyPressed(GLFW_KEY_SPACE)) {
            player->GetTransform().Position() += delta_time * speed * up;
        }
        if(input_handler->IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
            player->GetTransform().Position() -= delta_time * speed * up;
        }

        count++;

        time = glfwGetTime();
        //if(time - last_print >= 2.0f)
        {
            ::GUI::Render::GameDebugInfoEvent event;
            event.fps = count / (time - last_print);
            event.player_position = player->GetTransform().Position();

            DispatchEvent(event);

            last_print = time;
            count = 0;
        }
    }

    Destroy();
}

