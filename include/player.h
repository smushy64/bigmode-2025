#if !defined(PLAYER_H)
#define PLAYER_H
/**
 * @file   player.h
 * @brief  Player.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "raylib.h"
#include <math.h>

#define readonly() static const constexpr

readonly() float ANIMATION_TIME = 1.0 / 60.0;

readonly() float   CAMERA_MIN_ROTATION  = -15 * ( M_PI / 180.0 );
readonly() float   CAMERA_MAX_ROTATION  = 60 * ( M_PI / 180.0 );
readonly() float   CAMERA_DISTANCE      = 6.0;
readonly() float   CAMERA_HEIGHT        = 2.0;
readonly() Vector3 CAMERA_TARGET_OFFSET = { 0.0, 1.35, 0.0 };

readonly() float PLAYER_COLLISION_RADIUS   = 0.6;
readonly() float PLAYER_COLLISION_RADIUS_2 = PLAYER_COLLISION_RADIUS * PLAYER_COLLISION_RADIUS;

readonly() float MAX_WALK_VELOCITY = 12.0;
readonly() float MAX_RUN_VELOCITY  = 25.0;

readonly() float STOP_DRAG  = 10.0;
readonly() float TURN_SPEED = 40.0;

readonly() float START_MAX_POWER         = 100.0;
readonly() float POWER_LOSS_RATE         = 1.2;
readonly() float POWER_LOSS_IDLE_MULT    = 0.2;
readonly() float POWER_LOSS_RUNNING_MULT = 2.0;

readonly() float POWER_LOSS_ATTACK = 8.0;
readonly() float POWER_LOSS_DODGE  = 4.0;

readonly() float DODGE_TIME  = 0.45;
readonly() float DODGE_SPEED = 20.0;

readonly() float DODGE_COOLDOWN_TIME = 0.0;

readonly() float TAKING_DAMAGE_TIME = 0.3;

readonly() float ATTACK_TIME     = 0.29;
readonly() float ATTACK_DAMAGE   = 25.0;
readonly() float ATTACK_RADIUS   = 0.6;
readonly() float ATTACK_RADIUS_2 = ATTACK_RADIUS * 2.0f;

readonly() float DEATH_TIME = 3.0;

struct Input {
    Vector2 camera;
    Vector2 movement;
    bool    is_trying_to_move;
    bool    is_run_down;

    bool is_punch_press;
    bool is_kick_press;
    bool is_dodge_press;

    bool is_using_gamepad;
    bool is_pause_press;
};

enum class PlayerState {
    DEFAULT,
    
    IS_MOVING,

    ATTACK,
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
    float attack_timer;
    bool  which_attack;

    float dodge_cooldown_timer;
    float sfx_walk_timer;
    float sfx_walk_time;

    float animation_timer;
    int   animation_frame;

    bool attack_landed;
};

#undef readonly

#endif /* header guard */
