#if !defined(EDITOR_MAP_H)
#define EDITOR_MAP_H
/**
 * @file   map.h
 * @brief  Map representation in editor.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 27, 2025
*/
// IWYU pragma: begin_keep
#include "shared/world.h"
#include "shared/buffer.h"
#include "raymath.h"
#include <stddef.h>
// IWYU pragma: end_keep

struct EdObject;
struct EdSegment;

typedef Vector2 EdVertex;

struct MapStorage {
    struct {
        EdSegment* buf;
        int        len;
        int        cap;
    } segments;
    struct {
        EdVertex* buf;
        int       len;
        int       cap;
    } vertexes;
    struct {
        EdObject* buf;
        int       len;
        int       cap;
    } objects;
};

struct EdObject {
    Vector2    position;
    float      rotation;
    ObjectType type;
};

struct EdSegment {
    int start;
    int end;

    Color tint;

    inline
    Vector2& start_vertex( MapStorage* storage ) {
        return storage->vertexes.buf[this->start];
    }
    inline
    Vector2& end_vertex( MapStorage* storage ) {
        return storage->vertexes.buf[this->end];
    }
};

enum class ObjectShape {
    CIRCLE,
    RECTANGLE,
};
struct ObjectShapeDescription {
    Color       tint;
    ObjectShape shape;
    union {
        struct {
            float radius;
        } circle;
        struct {
            Vector2 size;
        } rectangle;
    };
};
inline
ObjectShapeDescription object_shape( ObjectType type ) {
    switch( type ) {
        case ObjectType::COUNT: 
        case ObjectType::NONE: return {
            .tint   = Color{ 100, 100, 100, 255 },
            .shape  = ObjectShape::RECTANGLE,
            .rectangle = {
                .size = Vector2{ 0.5, 0.5 } 
            }
        };
        case ObjectType::PLAYER_SPAWN: return {
            .tint   = BLUE,
            .shape  = ObjectShape::CIRCLE,
            .circle = {
                .radius = 0.5
            }
        };
        case ObjectType::ENEMY: return {
            .tint  = RED,
            .shape = ObjectShape::CIRCLE,
            .circle = {
                .radius = 0.5
            }
        };
        case ObjectType::BATTERY: return {
            .tint  = SKYBLUE,
            .shape = ObjectShape::CIRCLE,
            .circle = {
                .radius = 0.5
            }
        };
        case ObjectType::LEVEL_EXIT: return {
            .tint  = GOLD,
            .shape = ObjectShape::CIRCLE,
            .circle = {
                .radius = 0.5
            }
        };
    }
    return {};
}

struct NonAxisAlignedRect {
    Vector2 points[4];
};
inline NonAxisAlignedRect
gen_non_axis_aligned_rect( Vector2 center, Vector2 size, float rotation ) {
    NonAxisAlignedRect result;

    Vector2 offset = size / 2.0;
    Vector2 left_up_offset    = Vector2{ -offset.x, -offset.y };
    Vector2 left_down_offset  = Vector2{ -offset.x,  offset.y };
    Vector2 right_up_offset   = Vector2{  offset.x, -offset.y };
    Vector2 right_down_offset = Vector2{  offset.x,  offset.y };

    result.points[0] = center + Vector2Rotate( left_down_offset, rotation );
    result.points[1] = center + Vector2Rotate( right_up_offset, rotation );
    result.points[2] = center + Vector2Rotate( left_up_offset, rotation );
    result.points[3] = center + Vector2Rotate( right_down_offset, rotation );

    return result;
}

#endif /* header guard */
