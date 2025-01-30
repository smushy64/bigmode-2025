#if !defined(PLAYER_H)
#define PLAYER_H
/**
 * @file   player.h
 * @brief  Player.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "raylib.h"

#define readonly() static const constexpr

readonly() float MAX_WALK_VELOCITY = 15.0;
readonly() float MAX_RUN_VELOCITY  = 25.0;

readonly() float STOP_DRAG         = 10.0;
readonly() float TURN_SPEED        = 16.0;
readonly() float CAMERA_DISTANCE   = 5.8;
readonly() float CAMERA_HEIGHT     = 2.0;
readonly() Vector3 CAMERA_TARGET_OFFSET = { 0.0, 1.25, 0.0 };

readonly() float START_MAX_POWER         = 200.0;
readonly() float POWER_LOSS_RATE         = 1.2;
readonly() float POWER_LOSS_IDLE_MULT    = 0.2;
readonly() float POWER_LOSS_RUNNING_MULT = 2.0;

readonly() float POWER_LOSS_PUNCH = 12.0;
readonly() float POWER_LOSS_KICK  = 18.0;
readonly() float POWER_LOSS_DODGE = 30.0;

readonly() float DODGE_TIME  = 0.24;
readonly() float DODGE_SPEED = 30.0;

readonly() float DODGE_COOLDOWN_TIME = DODGE_TIME + 0.2;

readonly() float DAMAGE_IMPULSE_FORCE = 20.0;

struct Input {
    Vector2 camera;
    Vector2 movement;
    bool    is_trying_to_move;
    bool    is_run_down;

    bool is_punch_press;
    bool is_kick_press;
    bool is_dodge_press;
};

enum class PlayerState {
    DEFAULT,
    
    IS_MOVING,

    ATTACK_PUNCH,
    ATTACK_KICK,
    DODGE,

    TAKING_DAMAGE,

    IS_DEAD,
};

struct Player {
    Input       input;
    PlayerState state;
    Vector2     camera_rotation;

    Vector3 velocity;
    float max_velocity;

    Vector3 position;
    Vector3 movement_direction;

    float power;
    float power_target;
    float max_power;

    float inv_time;

    float dodge_cooldown_timer;
    float sfx_walk_timer;
    float sfx_walk_time;
};

#undef readonly

#endif /* header guard */
