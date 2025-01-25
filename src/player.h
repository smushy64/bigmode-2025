#if !defined(PLAYER_H)
#define PLAYER_H
/**
 * @file   player.h
 * @brief  Player.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "raylib.h"

struct Input {
    Vector2 camera;
};

struct Player {
    Input   input;
    Vector2 camera_rotation;

    Vector3 position;
};

#endif /* header guard */
