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

struct __attribute__((packed))
MapFileHeader {
    uint8_t  identifier[4];
    uint32_t total_size;

    uint16_t object_count;  // -> objects  = MapFileObject[object_count]   @ this + 1
    uint16_t sector_count;  // -> sectors  = MapFileSector[sector_count]   @ objects + object_count
    uint16_t segment_count; // -> segments = MapFileSegment[segment_count] @ sectors + sector_count
    uint16_t vertex_count;  // -> vertexes = Vector2[vertex_count]         @ segments + segment_count
    uint16_t texture_count; // -> textures = MapFileTexture[texture_count] @ vertexes + segment_count
};

struct __attribute__((packed))
MapFileObject {
    Vector2    position;
    ObjectType type;
};

struct __attribute__((packed))
MapFileSector {
    uint16_t segment_start;
    uint16_t segment_count;
    uint16_t floor_texture;
    uint16_t ceiling_texture;
};

struct __attribute__((packed))
MapFileSegment {
    Vector2  start;
    Vector2  end;
    Color    tint;
    uint16_t texture;
    bool     is_flipped_normal;
    bool     is_solid;
};

struct __attribute__((packed))
MapFileTexture {
    char          name[128]; // resources/textures + name
    TextureFilter filter : 8;
    TextureWrap   wrap   : 8;
};

#endif /* header guard */
