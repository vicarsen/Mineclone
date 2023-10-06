#include "player.h"

#include "files.h"

#include <glm/gtx/transform.hpp>

DEFINE_LOG_CATEGORY(Player, FILE_LOGGER(trace, LOGFILE("Player.txt")));

namespace Game
{
    Player::Player(float _fov, float _aspect_ratio) :
        transform(), projection(glm::perspective(_fov, _aspect_ratio, 0.1f, 1000.0f)), fov(_fov), aspect_ratio(_aspect_ratio), near(0.1f), far(1000.0f), render_distance(32)
    {
        TRACE(Player, "[{}:constructor] (#fov:{}) (#aspect_ratio:{}) <>", (unsigned long long) this, _fov, _aspect_ratio);
    }

    Player::~Player()
    {
        TRACE(Player, "[{}:destructor] <>", (unsigned long long) this);
    }

    void Player::Handle(const Render::FramebufferEvent& event)
    {
        aspect_ratio = event.width * 1.0f / event.height;
        projection = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
    }

    void Player::SetFOV(float _fov)
    {
        fov = _fov;
        projection = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
    }

    void Player::SetAspectRatio(float _aspect_ratio)
    {
        aspect_ratio = _aspect_ratio;
        projection = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
    }
};

