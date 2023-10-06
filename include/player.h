#pragma once

#include "logger.h"

#include "transform.h"
#include "events.h"
#include "render.h"

DECLARE_LOG_CATEGORY(Player);

namespace Game
{
    class Player : public Events::EventHandler<Render::FramebufferEvent>
    {
    public:
        Player(float fov, float aspect_ratio);
        ~Player();

        virtual void Handle(const Render::FramebufferEvent& event) override;

        inline Transform& GetTransform() noexcept { return transform; }
        inline const Transform& GetTransform() const noexcept { return transform; }

        inline const glm::mat4& GetProjection() const noexcept { return projection; }

        void SetFOV(float fov);
        void SetAspectRatio(float aspect_ratio);

        inline float GetRenderDistance() const noexcept { return render_distance; }
        inline void SetRenderDistance(float render_distance) noexcept { this->render_distance = render_distance; }

    private:
        Transform transform;
        glm::mat4 projection;
        float fov, aspect_ratio, near, far;
        float render_distance;
    };
};

