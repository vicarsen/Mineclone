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

        inline ::Math::Transform& GetTransform() noexcept { return transform; }
        inline const ::Math::Transform& GetTransform() const noexcept { return transform; }

        inline const glm::mat4& GetProjection() const noexcept { return projection; }

        void SetFOV(float fov);
        void SetAspectRatio(float aspect_ratio);

        inline float GetFOV() const noexcept { return fov; }
        inline float GetAspectRatio() const noexcept { return aspect_ratio; }
        inline float GetNear() const noexcept { return near; }
        inline float GetFar() const noexcept { return far; }

        inline float GetRenderDistance() const noexcept { return render_distance; }
        inline void SetRenderDistance(float render_distance) noexcept { this->render_distance = render_distance; }

    private:
        ::Math::Transform transform;
        glm::mat4 projection;
        float fov, aspect_ratio, near, far;
        float render_distance;
    };
};

