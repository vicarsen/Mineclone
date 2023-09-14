#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <unordered_set>
#include <future>

#include "logger.h"
#include "window.h"
#include "input.h"
#include "render.h"
#include "player.h"
#include "world.h"

DECLARE_LOG_CATEGORY(Main);
DEFINE_LOG_CATEGORY(Main, spdlog::level::trace);

int main()
{
    Window::Init(800, 600, "Mineclone");
    Input::Init();
    Render::Renderer::Init();

    {
        Game::Player player(90.0f, Window::GetAspectRatio());
        std::shared_ptr<Game::World> world = std::make_shared<Game::World>();

        std::unique_ptr<Game::ChunkGenerator> generator = std::make_unique<Game::SuperflatChunkGenerator>();
        Game::WorldLoader loader(world, generator);

        bool runWorldLoader = true;
        std::thread worldLoaderThread([&loader, &runWorldLoader, &player]() {
            while(runWorldLoader)
                loader.LoadChunks(Game::World::GetChunkCoordinates(player.GetTransform().Position()), 8);
        });

        float player_speed = 5.0f, player_sensitivity = 0.1f;
        float player_pitch = 0.0f;
        float player_yaw = 0.0f;

        float last_time = glfwGetTime(), delta_time = 0.0f;
        float last_print = glfwGetTime();
        int count = 0;
        while(!Window::ShouldClose())
        {
            Render::Renderer::Render(*world.get(), player);

            Input::PreUpdate();
            glfwPollEvents();
            Input::Update();

            float time = glfwGetTime();
            delta_time = time - last_time;
            last_time = time;

            glm::vec2 movement = Input::GetCursorMovement() * player_sensitivity;
            player_pitch = std::clamp(player_pitch - movement.y, -89.0f, 89.0f);
            player_yaw = player_yaw - movement.x;

            player.GetTransform().Rotation() = glm::quat({ glm::radians(player_pitch), glm::radians(player_yaw), 0.0f });

            glm::vec3 forward = player.GetTransform().Forward();
            forward.y = 0.0f; forward = glm::normalize(forward);

            glm::vec3 right = player.GetTransform().Right();

            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

            if(Input::IsKeyPressed(GLFW_KEY_A)) {
                player.GetTransform().Position() -= delta_time * player_speed * right;
            }
            if(Input::IsKeyPressed(GLFW_KEY_D)) {
                player.GetTransform().Position() += delta_time * player_speed * right;
            }
            if(Input::IsKeyPressed(GLFW_KEY_W)) {
                player.GetTransform().Position() += delta_time * player_speed * forward;
            }
            if(Input::IsKeyPressed(GLFW_KEY_S)) {
                player.GetTransform().Position() -= delta_time * player_speed * forward;
            }
            if(Input::IsKeyPressed(GLFW_KEY_SPACE)) {
                player.GetTransform().Position() += delta_time * player_speed * up;
            }
            if(Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
                player.GetTransform().Position() -= delta_time * player_speed * up;
            }

            count++;

            if(time - last_print >= 5.0f)
            {
                INFO(LogTemp, "Average FPS over 5 seconds: {}", count * 1.0f / (time - last_print));
                last_print = time;
                count = 0;
            }
        }

        runWorldLoader = false;
        worldLoaderThread.join();
    }

    Render::Renderer::Destroy();
    Input::Destroy();
    Window::Destroy();
    return 0;
}

