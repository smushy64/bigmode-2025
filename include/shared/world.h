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

#define MAP_IDENTIFIER "BM25"
#define MAP_EXT        ".map"

struct __attribute__((packed))
MapFileHeader {
    uint8_t  identifier[4];
    uint32_t total_size;

    uint16_t object_count;
    uint16_t vertex_count;
    uint16_t segment_count;
};

struct __attribute__((packed))
MapFileObject {
    Vector2    position;
    ObjectType type    : 16;
    uint16_t   rotation;
};

struct __attribute__((packed))
MapFileSegment {
    uint16_t start, end;
};

#endif /* header guard */
