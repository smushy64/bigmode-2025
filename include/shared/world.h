#if !defined(SHARED_WORLD_H)
#define SHARED_WORLD_H
/**
 * @file   world.h
 * @brief  World objects and map identifiers.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 27, 2025
*/
#include <stdint.h>
#include "raylib.h"
#include "shared/object.h"
#include "shared/level.h"

#define MAP_IDENTIFIER "BM25"
#define MAP_EXT        ".map"

struct MapFileHeader {
    uint8_t  identifier[4];
    uint32_t total_size;

    uint16_t object_count;
    uint16_t vertex_count;
    uint16_t segment_count;
};
static_assert(sizeof(MapFileHeader) == 16, "What?" );

struct MapFileObject {
    Vector2    position;
    ObjectType type;
    uint16_t   rotation;
    union {
        struct {
            LevelCondition condition;
        } level_exit;
    };
};
static_assert(sizeof(MapFileObject) == 16, "What?" );

struct MapFileSegment {
    uint16_t start, end;
};
static_assert(sizeof(MapFileSegment) == 4, "What?" );

#endif /* header guard */
