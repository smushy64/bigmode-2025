#if !defined(ENEMY_H)
#define ENEMY_H
/**
 * @file   enemy.h
 * @brief  Enemy constants.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 28, 2025
*/
#include "raylib.h"
#include "raymath.h"
#include <stdint.h>

#define readonly() static const constexpr

readonly() float E_IDLE_TIME          = 1.0;
readonly() float E_SCAN_TIME          = 0.8;
readonly() float E_WANDER_TIME        = 1.5;
readonly() float E_ALERT_TIME         = 0.5;
readonly() float E_ATTACK_TIME        = 0.61;
readonly() float E_TAKING_DAMAGE_TIME = 0.3;
readonly() float E_DYING_TIME         = 1.316;

readonly() float E_SFX_RUN_TIME  = 0.2;
readonly() float E_SFX_WALK_TIME = 0.5;

readonly() float E_SIGHT_RANGE = 10.0;

readonly() float E_WANDER_MAX_VELOCITY = 5.0;
readonly() float E_CHASE_MAX_VELOCITY  = 10.0;

readonly() float E_RETURN_HOME_DISTANCE = 0.1;

readonly() float E_DEFAULT_RADIUS = 15.0;

readonly() float E_ACCELERATION       = 25.0f;
readonly() float E_CHASE_ACCELERATION = 100.0f;

readonly() float E_ATTACK_POWER = 20.0;
readonly() float E_ATTACK_PUSH  = 20.0;


enum class EnemyState {
    IDLE,
    SCAN,
    WANDER,
    ALERT,
    CHASING,
    ATTACKING,
    RETURN_HOME,
    TAKING_DAMAGE,
    DYING,
};
const char* to_string( EnemyState state );

extern uintptr_t obj_enemy_offset;

struct Enemy {
    EnemyState state;
    Vector3    velocity;
    Vector3    home;
    Vector3    facing_direction;
    float      radius;
    float      power;
    float      timer;
    bool       first_frame_state;

    int   animation_frame;
    float animation_timer;

    float sfx_timer;

    union {
        struct {
            Vector3 direction;
        } wander;
        struct {
            Vector3 target;
        } chase;
    };

    inline
    Vector3 direction_to_home_sqr() const {
        // TODO(alicia): assuming that the relative offset of position won't change . . .
        Vector3 position = *(Vector3*)((char*)this - obj_enemy_offset);
        return home - position;
    }
    inline
    Vector3 direction_to_home() const {
        return Vector3Normalize( direction_to_home_sqr() );
    }
    inline
    Vector3 random_direction() const {
        int   chance   = GetRandomValue( -1000, 1000 );
        float rotation = (float)chance / 1000.0f;
        Vector3 direction = {1.0, 0.0, 0.0};

        direction = Vector3RotateByAxisAngle( direction, {0.0, 1.0, 0.0}, rotation );
        return Vector3Normalize( direction );
    }
};


#undef readonly

inline
const char* to_string( EnemyState state ) {
    switch( state ) {
        case EnemyState::IDLE:          return "Idle";
        case EnemyState::SCAN:          return "Scan";
        case EnemyState::WANDER:        return "Wander";
        case EnemyState::ALERT:         return "Alert";
        case EnemyState::CHASING:       return "Chase";
        case EnemyState::ATTACKING:     return "Attacking";
        case EnemyState::TAKING_DAMAGE: return "Taking Damage";
        case EnemyState::DYING:         return "Dying";
        case EnemyState::RETURN_HOME:   return "Returning Home";
    }
    return "";
}

#endif /* header guard */
