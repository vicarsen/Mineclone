#include "player.h"

#include "files.h"

#include <glm/gtx/transform.hpp>

DEFINE_LOG_CATEGORY(Player, spdlog::level::trace, LOGFILE("Player.txt"));

namespace Game
{
    Player::Player(float _fov, float _aspect_ratio) :
        transform(), projection(glm::perspective(_fov, _aspect_ratio, 0.1f, 1000.0f)), fov(_fov), aspect_ratio(_aspect_ratio), near(0.1f), far(1000.0f)
    {
        TRACE(Player, "[{}:constructor] (#fov:{}) (#aspect_ratio:{}) <>", (unsigned long long) this, _fov, _aspect_ratio);
    }

    Player::~Player()
    {
        TRACE(Player, "[{}:destructor] <>", (unsigned long long) this);
    }

    void Player::SetFOV(float _fov)
    {
        fov = _fov;
        projection = glm::perspective(_fov, aspect_ratio, 0.1f, 1000.0f);
    }

    void Player::SetAspectRatio(float _aspect_ratio)
    {
        aspect_ratio = _aspect_ratio;
        projection = glm::perspective(fov, _aspect_ratio, 0.1f, 1000.0f);
    }
};

