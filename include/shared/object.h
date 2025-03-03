#if !defined(SHARED_OBJECT_H)
#define SHARED_OBJECT_H
/**
 * @file   object.h
 * @brief  Objects.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 28, 2025
*/
#include "raylib.h"
#include "enemy.h"
#include <stdint.h>
#include <stddef.h>
#include "shared/level.h"

enum class ObjectType {
    NONE,
    PLAYER_SPAWN,
    ENEMY,
    BATTERY,
    LEVEL_EXIT,

    COUNT
};
const char* to_string( ObjectType type );

struct Player;
struct GlobalState;

struct Object {
    Vector3    position;
    ObjectType type;
    bool       is_active;

    union {
        Enemy enemy;

        struct {
            float power;
            float timer;
        } battery;

        struct {
            LevelCondition condition;
        } level_exit;
    };

    static inline
    Object create_enemy( Vector3 position, float rotation, float radius = 5.0f, float power = 50.0f ) {
        Object result = {};
        result.position  = position;
        result.type      = ObjectType::ENEMY;
        result.is_active = true;

        result.enemy.state  = EnemyState::IDLE;
        result.enemy.home   = position;
        result.enemy.radius = radius;
        result.enemy.power  = power;
        result.enemy.facing_direction =
            Vector3RotateByAxisAngle( Vector3UnitX, Vector3UnitY, rotation );
        return result;
    }
    static inline
    Object create_battery( Vector2 position ) {
        Object result = {};
        result.type      = ObjectType::BATTERY;
        result.is_active = true;
        result.position  = { position.x, 1.0, position.y };
        result.battery.power = 20.0;
        return result;
    }
    static inline
    Object create_level_exit( Vector2 position, LevelCondition condition ) {
        Object result = {};
        result.type      = ObjectType::LEVEL_EXIT;
        result.is_active = true;
        result.position  = { position.x, 0.0, position.y };
        result.level_exit.condition = condition;
        return result;
    }
};
inline
uintptr_t obj_enemy_offset = offsetof( Object, enemy );

inline
const char* to_string( ObjectType type ) {
    switch( type ) {
        case ObjectType::NONE:         return "None";
        case ObjectType::PLAYER_SPAWN: return "Player Spawn";
        case ObjectType::ENEMY:        return "Enemy";
        case ObjectType::BATTERY:      return "Battery";
        case ObjectType::LEVEL_EXIT:   return "Level Exit";
        
        case ObjectType::COUNT: break;
    }
    return "";
}

#endif /* header guard */
