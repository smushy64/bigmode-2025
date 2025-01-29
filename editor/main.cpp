/**
 * @file   main.cpp
 * @brief  Editor main.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 26, 2025
*/
// IWYU pragma: begin_keep
#include "raylib.h"
#include "raygui.h"
#include "raymath.h"
#include "rlgl.h"

#include "editor/map.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// IWYU pragma: end_keep

inline
void _0_( int v, ... );
#define unused(...) _0_( 0, ##__VA_ARGS__ )

Vector3 v3( float x, float y, float z ) { return Vector3{ x, y, z }; }
Vector3 v3( float x ) { return v3( x, x, x ); }

#define DEFAULT_WINDOW_WIDTH  (1280)
#define DEFAULT_WINDOW_HEIGHT ( 720)

#define FONT_SIZE           ( 18)
#define TOP_PANEL_HEIGHT    ( 12 + FONT_SIZE)
#define SIDE_PANEL_WIDTH    (300)

#define OBJECT_RADIUS (0.1f)
#define VERTEX_RADIUS (0.05f)

void DrawRectangleLinesRec( Rectangle rect, Color color );

enum class Mode {
    SELECT,
    PUT,
    BUILD,

    COUNT
};
const char* to_string( Mode mode );
const char* mode_description( Mode mode );
int mode_key( Mode mode );

const char* collate_modes();
const char* collate_objects();

enum SelectionType {
    NONE,
    OBJECT,
    VERTEX,
    SEGMENT,
};

struct Selection {
    SelectionType type;
    union {
        EdObject*  object;
        EdVertex*  vertex;
        EdSegment* segment;
    };
};

struct HoverIndexes {
    int* buf;
    int  len;
    int  cap;
};

struct HoverResult {
    HoverIndexes object_indexes;
    HoverIndexes vertex_indexes;
    HoverIndexes segment_indexes;

    bool object_hovered;
    bool vertex_hovered;
    bool segment_hovered;

    bool object_exclusive() {
        return object_hovered;
    }
    bool vertex_exclusive() {
        return !object_hovered && vertex_hovered;
    }
    bool segment_exclusive() {
        return !(object_hovered || vertex_hovered) && segment_hovered;
    }

    void clear() {
        object_hovered  = false;
        vertex_hovered  = false;
        segment_hovered = false;

        object_indexes.len  = 0;
        vertex_indexes.len  = 0;
        segment_indexes.len = 0;
    }
};

struct VertexBuffer {
    Vector3* buf;
    int      len;
    int      cap;
};
struct IndexBuffer {
    unsigned short* buf;
    int len;
    int cap;
};

struct State {
    MapStorage  storage;
    HoverResult hover;
    Mode        mode;

    VertexBuffer triangle_positions;
    IndexBuffer  triangle_indexes;
    bool triangles_dirty;

    union {
        Selection selection;

        struct {
            ObjectType type;
        } put;

        struct {
            int previous_index;
        } build;
    };
    struct {
        // tag: selection.type
        union {
            struct {
                struct {
                    char x_buf[32];
                    char y_buf[32];

                    bool x_edit;
                    bool y_edit;
                } position;

                bool type_edit;
            } object;
            struct {
                struct {
                    char x_buf[32];
                    char y_buf[32];

                    bool x_edit;
                    bool y_edit;
                } position;
            } vertex;
            struct {
                struct {
                    char x_buf[32];
                    char y_buf[32];

                    bool x_edit;
                    bool y_edit;
                } start;

                struct {
                    char x_buf[32];
                    char y_buf[32];

                    bool x_edit;
                    bool y_edit;
                } end;

            } segment;
        };

        bool is_dirty;
    } inspector;
};
bool triangulate( State& state );
void hover_items( State& state, Vector2 mouse_world_position );
void set_mode( State& state, Mode mode );
int main() {
    InitWindow( DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "BIGMODE Game Jam 2025 - Editor" );

    Font font = LoadFont( "resources/ui/fonts/RobotoCondensed/RobotoCondensed-Medium.ttf");
    SetTextureFilter( font.texture, TEXTURE_FILTER_BILINEAR );

    GuiLoadStyle( "resources/ui/styles/dark/style_dark.rgs" );
    GuiSetFont( font );
    GuiSetStyle( DEFAULT, TEXT_SIZE, FONT_SIZE );

    int scr_width  = DEFAULT_WINDOW_WIDTH;
    int scr_height = DEFAULT_WINDOW_HEIGHT;

    Vector2 draw_area = {};
    draw_area.x = scr_width  - SIDE_PANEL_WIDTH;
    draw_area.y = scr_height - TOP_PANEL_HEIGHT;

    RenderTexture rt = LoadRenderTexture( draw_area.x, draw_area.y );

    State* _ptr_state = (State*)calloc( 1, sizeof(State) );
    State& state = *_ptr_state; 
    state.triangles_dirty = true;

    const char* modes = collate_modes();
    const char* object_sc_list = collate_objects();

    set_mode( state, Mode::SELECT );

    Camera2D camera = {};
    camera.zoom     = 100.0;
    camera.target   = {};

    #define SCROLL_SPEED   10.0f
    #define MAX_ZOOM      500.0f
    #define GUI_GUTTER      8.0f

    Vector2 last_world_mouse = {};
    while( !WindowShouldClose() ) {
        float dt = GetFrameTime();

        bool    lmb         = IsMouseButtonDown( MOUSE_LEFT_BUTTON );
        bool    rmb         = IsMouseButtonDown( MOUSE_RIGHT_BUTTON );
        bool    mmb         = IsMouseButtonDown( MOUSE_MIDDLE_BUTTON );
        bool    lmb_press   = IsMouseButtonPressed( MOUSE_LEFT_BUTTON );
        bool    rmb_press   = IsMouseButtonPressed( MOUSE_RIGHT_BUTTON );
        bool    mmb_press   = IsMouseButtonPressed( MOUSE_MIDDLE_BUTTON );
        bool    lmb_release = IsMouseButtonReleased( MOUSE_LEFT_BUTTON );
        bool    rmb_release = IsMouseButtonReleased( MOUSE_RIGHT_BUTTON );
        bool    mmb_release = IsMouseButtonReleased( MOUSE_MIDDLE_BUTTON );
        Vector2 mouse       = GetMousePosition();
        Vector2 mouse_delta = GetMouseDelta();
        float   scroll      = GetMouseWheelMove() * SCROLL_SPEED;

        int anykey = GetKeyPressed();
        if( anykey != KEY_NULL ) {
            for( int i = 0; i < (int)Mode::COUNT; ++i ) {
                int k = mode_key( (Mode)i );
                if( k == anykey ) {
                    set_mode( state, (Mode)i );
                }
            }
        }

        unused(lmb,rmb,mmb,lmb_press,rmb_press,mmb_press,lmb_release,rmb_release,mmb_release,mouse,mouse_delta,scroll);
        unused(dt,modes);

        if( mmb ) {
            camera.target = GetScreenToWorld2D( camera.offset - mouse_delta, camera );
        }

        scr_width  = GetScreenWidth();
        scr_height = GetScreenHeight();

        if( IsWindowResized() ) {
            draw_area.x = scr_width  - SIDE_PANEL_WIDTH;
            draw_area.y = scr_height - TOP_PANEL_HEIGHT;

            UnloadRenderTexture( rt );
            rt = LoadRenderTexture( draw_area.x, draw_area.y );
        }

        camera.offset = { (float)draw_area.x / 2.0f, (float)draw_area.y / 2.0f };
        camera.zoom = Clamp( camera.zoom + scroll, 0.0, MAX_ZOOM );

        Vector2 world_mouse = GetScreenToWorld2D(
            mouse - Vector2{ 0, TOP_PANEL_HEIGHT }, camera );
        Vector2 world_delta = world_mouse - last_world_mouse;
        last_world_mouse = world_mouse;

        if( !lmb ) {
            state.hover.clear();
            hover_items( state, world_mouse );
        }

        if( 
            (mouse.x > 0)                              &&
            (mouse.y > 0)                              &&
            (mouse.x < (scr_width - SIDE_PANEL_WIDTH)) &&
            (mouse.y > TOP_PANEL_HEIGHT)
        ) {
            switch( state.mode ) {
                case Mode::BUILD: {
                    if( lmb_press ) {
                        if( state.hover.vertex_indexes.len ) {
                            int index = state.hover.vertex_indexes.buf[0];

                            if( state.build.previous_index >= 0 ) {
                                EdSegment segment = {};
                                segment.start = state.build.previous_index;
                                segment.end   = index;
                                segment.tint  = WHITE;

                                buf_append( &state.storage.segments, segment );
                                state.build.previous_index = -1;
                            } else {
                                state.build.previous_index = index;
                            }
                        } else {
                            EdVertex vertex = {};
                            vertex.position = world_mouse;
                            int this_index = state.storage.vertexes.len;
                            buf_append( &state.storage.vertexes, vertex );

                            if( state.build.previous_index >= 0 ) {
                                EdSegment segment = {};
                                segment.start = state.build.previous_index;
                                segment.end   = this_index;
                                segment.tint  = WHITE;

                                buf_append( &state.storage.segments, segment );
                            }

                            state.build.previous_index = this_index;
                        }
                    }
                    if( rmb_press ) {
                        state.build.previous_index = -1;
                    }
                } break;
                case Mode::SELECT: {
                    if( lmb_press ) {
                        if(
                            state.hover.object_exclusive() &&
                            state.hover.object_indexes.len
                        ) {
                            state.selection.type   = SelectionType::OBJECT;
                            state.selection.object =
                                state.storage.objects.buf + state.hover.object_indexes.buf[0];
                            state.inspector.is_dirty = true;
                        } else if(
                            state.hover.vertex_exclusive() &&
                            state.hover.vertex_indexes.len
                        ) {
                            state.selection.type   = SelectionType::VERTEX;
                            state.selection.vertex =
                                state.storage.vertexes.buf + state.hover.vertex_indexes.buf[0];
                            state.inspector.is_dirty = true;
                        } else if(
                            state.hover.segment_exclusive() &&
                            state.hover.segment_indexes.len
                        ) {
                            state.selection.type    = SelectionType::SEGMENT;
                            state.selection.segment =
                                state.storage.segments.buf + state.hover.segment_indexes.buf[0];
                            state.inspector.is_dirty = true;
                        } else if( CheckCollisionPointRec( mouse, Rectangle{ 0, (float)TOP_PANEL_HEIGHT, draw_area.x, draw_area.y } ) ) {
                            state.selection.type     = SelectionType::NONE;
                            state.selection.object   = nullptr;
                            state.inspector.is_dirty = true;
                        }
                    }
                    if( lmb_release ) {
                        state.triangles_dirty = true;
                    }

                    if( state.hover.object_exclusive() ) {
                        if( lmb ) {
                            for( int idx = 0; idx < state.hover.object_indexes.len; ++idx ) {
                                auto* o =
                                    state.storage.objects.buf +
                                    state.hover.object_indexes.buf[idx];
                                o->position += world_delta;
                            }
                        } else if( rmb_press ) {
                            auto* indexes = &state.hover.object_indexes;
                            auto* buf     = &state.storage.objects;

                            if( indexes->len ) {
                                int idx     = indexes->buf[indexes->len - 1];
                                int obj_idx = indexes->buf[idx];

                                auto* obj = buf->buf + obj_idx;
                                TraceLog(
                                    LOG_INFO, "Removing %s @ { %f, %f }",
                                    to_string( obj->type ), obj->position.x, obj->position.y );

                                buf->buf[obj_idx] = buf->buf[--state.storage.objects.len];
                            }
                        }
                    }

                    if( state.hover.vertex_exclusive() ) {
                        if( lmb ) {
                            for( int idx = 0; idx < state.hover.vertex_indexes.len; ++idx ) {
                                auto* v =
                                    state.storage.vertexes.buf + 
                                    state.hover.vertex_indexes.buf[idx];
                                v->position += world_delta;
                            }
                        } else if( rmb_press ) {
                            int vertex_to_remove = state.hover.vertex_indexes.buf[0];

                            auto* vertexes = &state.storage.vertexes;
                            auto* segments = &state.storage.segments;

                            if( segments->len > 1 ) {
                                EdSegment* left = nullptr, *right = nullptr;
                                for( int i = 0; i < segments->len; ++i ) {
                                    auto* s = segments->buf + i;
                                    if( s->end == vertex_to_remove ) {
                                        left = s;
                                        continue;
                                    }
                                    if( s->start == vertex_to_remove ) {
                                        right = s;
                                        continue;
                                    }
                                }

                                if( left && right ) {
                                    left->end = right->end;

                                    int right_idx = (int)(right - segments->buf);
                                    buf_swap_remove( segments, right_idx );
                                }

                                for( int i = 0; i < segments->len; ++i ) {
                                    auto* s = segments->buf + i; 
                                    if( s->start > vertex_to_remove ) {
                                        s->start--;
                                    }
                                    if( s->end > vertex_to_remove ) {
                                        s->end--;
                                    }
                                }

                                buf_remove( vertexes, vertex_to_remove );
                            } else {
                                buf_swap_remove( segments, 0 );
                                buf_swap_remove( vertexes, vertex_to_remove );
                            }

                        }
                    }
                    if( state.hover.segment_exclusive() ) {
                        if( lmb ) {
                            for( int idx = 0; idx < state.hover.segment_indexes.len; ++idx ) {
                                auto* s =
                                    state.storage.segments.buf + 
                                    state.hover.segment_indexes.buf[idx];
                                Vector2& start = state.storage.vertexes.buf[s->start].position;
                                Vector2& end   = state.storage.vertexes.buf[s->end].position;

                                start += world_delta;
                                end   += world_delta;
                            }
                        } else if( rmb_press ) {
                            auto* idx      = &state.hover.segment_indexes;
                            auto* segments = &state.storage.segments;

                            for( int i = 0; i < idx->len; ++i ) {
                                buf_swap_remove( segments, idx->buf[i] );
                            }
                        }
                    }

                } break;
                case Mode::PUT: {
                    if( lmb_press ) {
                        EdObject obj = {};
                        obj.type     = state.put.type;
                        obj.position = world_mouse;
                        obj.rotation = 0.0;
                        buf_append( &state.storage.objects, obj );

                        TraceLog(
                            LOG_INFO, "Put %s @ { %f, %f }",
                            to_string( obj.type ), world_mouse.x, world_mouse.y );
                    }
                    if( rmb_press ) {
                        auto* indexes = &state.hover.object_indexes;
                        auto* buf     = &state.storage.objects;

                        if( indexes->len ) {
                            int idx     = indexes->buf[indexes->len - 1];
                            int obj_idx = indexes->buf[idx];

                            auto* obj = buf->buf + obj_idx;
                            TraceLog(
                                LOG_INFO, "Removing %s @ { %f, %f }",
                                to_string( obj->type ), obj->position.x, obj->position.y );

                            buf->buf[obj_idx] = buf->buf[--state.storage.objects.len];
                        }
                    }

                    int k = (anykey - KEY_ZERO);
                    if( k >= 0 && k < (int)ObjectType::COUNT ) {
                        ObjectType t = (ObjectType)k;
                        state.put.type = t;
                    }
                } break;

                case Mode::COUNT: break;
            }
        }

        if( triangulate( state ) ) {
            TraceLog( LOG_INFO, "Retriangulated level." );
            TraceLog( LOG_INFO, "Vertex count: %i", state.triangle_positions.len );
            TraceLog( LOG_INFO, "Index count:  %i", state.triangle_indexes.len );
        }

        BeginDrawing();
        ClearBackground( BLACK );

        /* Draw Map */
        BeginTextureMode( rt ); {
            ClearBackground( BLACK );

            BeginMode2D( camera ); {

                for(
                    int vertex_idx = 0;
                    vertex_idx < state.storage.vertexes.len;
                    ++vertex_idx
                ) {
                    auto* v = state.storage.vertexes.buf + vertex_idx;
                    Color color = WHITE;
                    DrawCircleLinesV( v->position, 0.05, color );
                }
                if( state.hover.vertex_exclusive() ) {
                    for(
                        int idx = 0;
                        idx < state.hover.vertex_indexes.len;
                        ++idx
                    ) {
                        auto* v =
                            state.storage.vertexes.buf +
                            state.hover.vertex_indexes.buf[idx];
                        Color color = WHITE;
                        DrawCircleV( v->position, 0.05, color );
                    }
                }
                for(
                    int segment_idx = 0;
                    segment_idx < state.storage.segments.len;
                    ++segment_idx
                ) {
                    auto* s = state.storage.segments.buf + segment_idx;

                    Color color = s->tint;
                    color.a = 255 / 2;

                    Vector2 start, end;
                    start = state.storage.vertexes.buf[s->start].position;
                    end   = state.storage.vertexes.buf[s->end].position;

                    DrawLineV( start, end, color );
                }
                if( state.hover.segment_exclusive() ) {
                    for(
                        int idx = 0;
                        idx < state.hover.segment_indexes.len;
                        ++idx
                    ) {
                        auto* s =
                            state.storage.segments.buf + 
                            state.hover.segment_indexes.buf[idx];

                        Color color = s->tint;

                        Vector2 start, end;
                        start = state.storage.vertexes.buf[s->start].position;
                        end   = state.storage.vertexes.buf[s->end].position;

                        DrawLineV( start, end, color );
                    }
                }

                for(
                    int obj_idx = 0;
                    obj_idx < state.storage.objects.len;
                    ++obj_idx
                ) {
                    auto* o = state.storage.objects.buf + obj_idx;

                    ObjectShapeDescription shape = object_shape( o->type );

                    float direction_line_size = 1.0;
                    switch( shape.shape ) {
                        case ObjectShape::CIRCLE: {
                            DrawCircleLinesV( o->position, shape.circle.radius, shape.tint );
                            direction_line_size = shape.circle.radius + 0.2;
                        } break;
                        case ObjectShape::RECTANGLE: {
                            NonAxisAlignedRect rect = gen_non_axis_aligned_rect(
                                o->position, shape.rectangle.size, o->rotation );
                            DrawLineV( rect.points[0], rect.points[2], shape.tint );
                            DrawLineV( rect.points[3], rect.points[1], shape.tint );
                            DrawLineV( rect.points[0], rect.points[3], shape.tint );
                            DrawLineV( rect.points[1], rect.points[2], shape.tint );
                        } break;
                    }
                    DrawLineV(
                        o->position,
                        o->position + Vector2Rotate(
                            Vector2{ direction_line_size, 0.0 }, o->rotation),
                        shape.tint );

                }
                if( state.hover.object_exclusive() ) {
                    for(
                        int idx = 0;
                        idx < state.hover.object_indexes.len;
                        ++idx
                    ) {
                        auto* o =
                            state.storage.objects.buf +
                            state.hover.object_indexes.buf[idx];
                        ObjectShapeDescription shape = object_shape( o->type );

                        switch( shape.shape ) {
                            case ObjectShape::CIRCLE: {
                                DrawCircleV( o->position, shape.circle.radius, shape.tint );
                            } break;
                            case ObjectShape::RECTANGLE: {
                                Rectangle rect = {
                                    o->position.x,
                                    o->position.y,
                                    shape.rectangle.size.x,
                                    shape.rectangle.size.y,
                                };
                                DrawRectanglePro(
                                    rect, (shape.rectangle.size / 2.0), o->rotation * (180.0 / M_PI), shape.tint );
                            } break;
                        }
                    }
                }

                // NOTE(alicia): draw selection outline
                switch( state.mode ) {
                    case Mode::BUILD: {
                        if( state.build.previous_index >= 0 ) {
                            Vector2 prev =
                                state.storage.vertexes.buf
                                    [state.build.previous_index].position;
                            DrawLineV( prev, world_mouse, WHITE );
                        }
                    } break;
                    case Mode::PUT:
                    case Mode::COUNT: break;
                    case Mode::SELECT: {
                        switch( state.selection.type ) {
                            case NONE:    break;
                            case OBJECT: {
                                Rectangle rect = {};
                                auto* obj = state.selection.object;
                                ObjectShapeDescription shape = object_shape( obj->type );
                                switch( shape.shape ) {
                                    case ObjectShape::CIRCLE: {
                                        rect.x      = obj->position.x - shape.circle.radius;
                                        rect.y      = obj->position.y - shape.circle.radius;
                                        rect.width  = shape.circle.radius * 2.0;
                                        rect.height = shape.circle.radius * 2.0;
                                    } break;
                                    case ObjectShape::RECTANGLE: {
                                        NonAxisAlignedRect r = gen_non_axis_aligned_rect(
                                            obj->position, shape.rectangle.size, obj->rotation );

                                        Vector2 _min = { 20000, 20000 },
                                                _max = { -200000, -200000 };
                                        for( int i = 0; i < 4; ++i ) {
                                            Vector2 p = r.points[i];
                                            if( p.x < _min.x ) {
                                                _min.x = p.x;
                                            }
                                            if( p.y < _min.y ) {
                                                _min.y = p.y;
                                            }
                                            if( p.x > _max.x ) {
                                                _max.x = p.x;
                                            }
                                            if( p.y > _max.y ) {
                                                _max.y = p.y;
                                            }
                                        }

                                        rect.x      = _min.x - 0.1;
                                        rect.y      = _min.y - 0.1;
                                        rect.width  = abs(_max.x - _min.x) + 0.2;
                                        rect.height = abs(_max.y - _min.y) + 0.2;
                                    } break;
                                }

                                DrawRectangleLinesRec( rect, GREEN );
                            } break;
                            case VERTEX: {
                                Rectangle rect = {
                                    state.selection.vertex->position.x,
                                    state.selection.vertex->position.y,
                                    VERTEX_RADIUS + 0.2,
                                    VERTEX_RADIUS + 0.2,
                                };
                                rect.x -= rect.width  / 2.0;
                                rect.y -= rect.height / 2.0;

                                DrawRectangleLinesRec( rect, GREEN );
                            } break;
                            case SEGMENT: {
                                Vector2 start =
                                    state.storage.vertexes.buf[state.selection.segment->start]
                                        .position;
                                Vector2 end =
                                    state.storage.vertexes.buf[state.selection.segment->end]
                                        .position;
                                DrawLineV( start, end, GREEN );
                            } break;
                        }
                    } break;
                }

                /* Draw level mesh */ {
                    auto* t = &state.triangle_indexes;
                    auto* v = &state.triangle_positions;
                    for( int i = 0; i < t->len; i += 3 ) {
                        int i0 = t->buf[i + 0];
                        int i1 = t->buf[i + 1];
                        int i2 = t->buf[i + 2];
                        Vector2 p0 = { v->buf[i0].x, v->buf[i0].z };
                        Vector2 p1 = { v->buf[i1].x, v->buf[i1].z };
                        Vector2 p2 = { v->buf[i2].x, v->buf[i2].z };

                        DrawTriangle( p0, p1, p2, ColorAlpha( WHITE, 0.2 ) );
                    }
                }

            } EndMode2D();

            for( int obj_idx = 0; obj_idx < state.storage.objects.len; ++obj_idx ) {
                auto*       obj       = state.storage.objects.buf + obj_idx;
                const char* name      = to_string( obj->type );
                Vector2     text_size = MeasureTextEx( font, name, FONT_SIZE, 1.0 );

                Vector2 text_offset = { text_size.x / 2.0f, text_size.y };

                Vector2 text_pos =
                    GetWorldToScreen2D( obj->position, camera ) - text_offset;
                DrawTextEx(
                    font, name, text_pos, FONT_SIZE, 1.0,
                    GetColor( GuiGetStyle( DEFAULT, TEXT_COLOR_NORMAL ) ) );
            }

        } EndTextureMode();

        Rectangle map_area = {
            0, TOP_PANEL_HEIGHT,
            draw_area.x, draw_area.y
        };

        /* Blit Texture */
        DrawTexturePro(
            rt.texture, { 0, 0, draw_area.x, -draw_area.y },
            map_area, {}, 0.0, WHITE );

        /* Draw GUI */ {

            Vector2 mode_name_position = *(Vector2*)&map_area;
            mode_name_position.x += GUI_GUTTER;
            mode_name_position.y += GUI_GUTTER;
            for( int i = 0; i < (int)Mode::COUNT; ++i ) {
                Color color;
                if( (Mode)i == state.mode ) {
                    color = GetColor( GuiGetStyle( DEFAULT, TEXT_COLOR_NORMAL ) );
                } else {
                    color = GetColor( GuiGetStyle( DEFAULT, TEXT_COLOR_DISABLED ) );
                }
                DrawTextEx(
                    font, TextFormat( "%s", to_string( (Mode)i ) ),
                    mode_name_position, FONT_SIZE, 1.0, color );
                mode_name_position.y += FONT_SIZE;
            }

            if( state.mode == Mode::PUT ) {
                mode_name_position.y += FONT_SIZE;
                for( int i = 0; i < (int)ObjectType::COUNT; ++i ) {
                    Color color;
                    if( (ObjectType)i == state.put.type ) {
                        color = GetColor( GuiGetStyle( DEFAULT, TEXT_COLOR_NORMAL ) );
                    } else {
                        color = GetColor( GuiGetStyle( DEFAULT, TEXT_COLOR_DISABLED ) );
                    }
                    DrawTextEx(
                        font, TextFormat( "%i - %s", i, to_string( (ObjectType)i ) ),
                        mode_name_position, FONT_SIZE, 1.0, color );
                    mode_name_position.y += FONT_SIZE;
                }
            }
            Vector2 mode_description_size = MeasureTextEx(
                font, mode_description( state.mode ), FONT_SIZE, 1.0 );
            Vector2 mode_description_position = {
                map_area.x + GUI_GUTTER,
                map_area.y + (map_area.height - (mode_description_size.y + GUI_GUTTER)) };
            DrawTextEx( font, mode_description( state.mode ), mode_description_position, FONT_SIZE, 1.0, GetColor( GuiGetStyle( DEFAULT, TEXT_COLOR_NORMAL )) );

            Rectangle top_bar = {
                0, 0,
                (float)scr_width,
                TOP_PANEL_HEIGHT
            };
            DrawRectangleRec( top_bar, GetColor( GuiGetStyle( DEFAULT, BACKGROUND_COLOR ) ) ); {
                Rectangle button = top_bar;
                button.width /= 10.0;
                button.height -= 10.0;
                button.x += GUI_GUTTER;
                button.y += 6.0;

                GuiButton( button, "Load Map" );

                button.x += button.width + GUI_GUTTER;

                GuiButton( button, "Save Map" );
            }

            Rectangle side_bar = {
                (float)scr_width - SIDE_PANEL_WIDTH,
                top_bar.height,
                SIDE_PANEL_WIDTH,
                scr_height - top_bar.height
            };
            GuiPanel( side_bar, "Inspector" );
            if( state.mode == Mode::SELECT ) {
                side_bar.x      += 10.0;
                side_bar.y      += 10.0 + TOP_PANEL_HEIGHT;
                side_bar.width  -= (10.0) * 2.0;
                side_bar.height -= (10.0) * 2.0 + TOP_PANEL_HEIGHT;

                Rectangle setting = side_bar;
                setting.height = 20.0;
                Vector2 text_width = {};

                switch( state.selection.type ) {
                    case SelectionType::NONE:    break;
                    case SelectionType::OBJECT: {

                        if( state.inspector.is_dirty ) {
                            state.inspector.is_dirty = false;
                            snprintf(
                                state.inspector.object.position.x_buf, 32,
                                "%f", state.selection.object->position.x );
                            snprintf(
                                state.inspector.object.position.y_buf, 32,
                                "%f", state.selection.object->position.y );
                            state.inspector.object.position.x_edit = false;
                            state.inspector.object.position.y_edit = false;
                            state.inspector.object.type_edit       = false;
                        }

                        GuiLabel( setting, "Object" );
                        setting.y += setting.height + GUI_GUTTER;

                        text_width = MeasureTextEx( font, "Position", FONT_SIZE, 1.0 );
                        GuiLabel( setting, "Position" ); {

                            Rectangle input_area = setting;
                            input_area.x     += text_width.x + GUI_GUTTER;
                            input_area.width -= text_width.x + GUI_GUTTER;

                            Rectangle x_input = input_area;
                            x_input.width = (x_input.width / 2.0) - (GUI_GUTTER / 2.0);

                            if( GuiValueBoxFloat(
                                x_input, "", state.inspector.object.position.x_buf,
                                &state.selection.object->position.x,
                                state.inspector.object.position.x_edit
                            ) ) {
                                state.inspector.object.position.x_edit =
                                    !state.inspector.object.position.x_edit;
                            }

                            Rectangle y_input = x_input;
                            y_input.x += y_input.width + GUI_GUTTER;

                            if( GuiValueBoxFloat(
                                y_input, "", state.inspector.object.position.y_buf,
                                &state.selection.object->position.y,
                                state.inspector.object.position.y_edit
                            ) ) {
                                state.inspector.object.position.y_edit =
                                    !state.inspector.object.position.y_edit;
                            }
                        }

                        setting.y += setting.height + GUI_GUTTER;

                        text_width = MeasureTextEx( font, "Rotation 000.000r", FONT_SIZE, 1.0 );
                        GuiLabel( setting, "Rotation" ); {

                            Rectangle input_area = setting;
                            input_area.x     += text_width.x + GUI_GUTTER;
                            input_area.width -= text_width.x + GUI_GUTTER;

                            GuiSlider(
                                input_area, TextFormat( "%.2f rad", state.selection.object->rotation ),
                                "", &state.selection.object->rotation,
                                -(M_PI * 2.0f), (M_PI * 2.0f) );
                        }

                        setting.y += setting.height + GUI_GUTTER;

                        text_width = MeasureTextEx( font, "Object Type", FONT_SIZE, 1.0 );
                        GuiLabel( setting, "Object Type" );

                        if( GuiDropdownBox( {
                            setting.x + text_width.x + GUI_GUTTER, setting.y,
                            setting.width - (text_width.x + GUI_GUTTER), setting.height },
                            object_sc_list, (int*)&state.selection.object->type,
                            state.inspector.object.type_edit
                        ) ) {
                            state.inspector.object.type_edit =
                                !state.inspector.object.type_edit;
                        }
                        setting.y += setting.height + GUI_GUTTER;

                        state.inspector.is_dirty = false;
                    } break;
                    case SelectionType::VERTEX: {

                        snprintf(
                            state.inspector.vertex.position.x_buf, 32,
                            "%f", state.selection.vertex->position.x );
                        snprintf(
                            state.inspector.vertex.position.y_buf, 32,
                            "%f", state.selection.vertex->position.y );

                        if( state.inspector.is_dirty ) {
                            state.inspector.is_dirty = false;
                            state.inspector.vertex.position.x_edit = false;
                            state.inspector.vertex.position.y_edit = false;
                        }

                        GuiLabel( setting, "Vertex" );
                        setting.y += setting.height + GUI_GUTTER;

                        text_width = MeasureTextEx( font, "Position", FONT_SIZE, 1.0 );
                        GuiLabel( setting, "Position" ); {

                            Rectangle input_area = setting;
                            input_area.x     += text_width.x + GUI_GUTTER;
                            input_area.width -= text_width.x + GUI_GUTTER;

                            Rectangle x_input = input_area;
                            x_input.width = (x_input.width / 2.0) - (GUI_GUTTER / 2.0);

                            if( GuiValueBoxFloat(
                                x_input, "", state.inspector.vertex.position.x_buf,
                                &state.selection.vertex->position.x,
                                state.inspector.vertex.position.x_edit
                            ) ) {
                                state.inspector.vertex.position.x_edit =
                                    !state.inspector.vertex.position.x_edit;
                            }

                            Rectangle y_input = x_input;
                            y_input.x += y_input.width + GUI_GUTTER;

                            if( GuiValueBoxFloat(
                                y_input, "", state.inspector.vertex.position.y_buf,
                                &state.selection.vertex->position.y,
                                state.inspector.vertex.position.y_edit
                            ) ) {
                                state.inspector.vertex.position.y_edit =
                                    !state.inspector.vertex.position.y_edit;
                            }
                        }

                    } break;
                    case SelectionType::SEGMENT: {

                        snprintf(
                            state.inspector.segment.start.x_buf, 32,
                            "%f", state.selection.segment->start_vertex( &state.storage ).x );
                        snprintf(
                            state.inspector.segment.start.y_buf, 32,
                            "%f", state.selection.segment->start_vertex( &state.storage ).y );

                        snprintf(
                            state.inspector.segment.end.x_buf, 32,
                            "%f", state.selection.segment->end_vertex( &state.storage ).x );
                        snprintf(
                            state.inspector.segment.end.y_buf, 32,
                            "%f", state.selection.segment->end_vertex( &state.storage ).y );

                        if( state.inspector.is_dirty ) {
                            state.inspector.is_dirty = false;

                            state.inspector.segment.start.x_edit = false;
                            state.inspector.segment.start.y_edit = false;
                            state.inspector.segment.end.x_edit = false;
                            state.inspector.segment.end.y_edit = false;
                        }

                        GuiLabel( setting, "Line Segment (Wall)" );
                        setting.y += setting.height + GUI_GUTTER;

                        text_width = MeasureTextEx( font, "Start", FONT_SIZE, 1.0 );
                        GuiLabel( setting, "Start" ); {

                            Rectangle input_area = setting;
                            input_area.x     += text_width.x + GUI_GUTTER;
                            input_area.width -= text_width.x + GUI_GUTTER;

                            Rectangle x_input = input_area;
                            x_input.width = (x_input.width / 2.0) - (GUI_GUTTER / 2.0);

                            if( GuiValueBoxFloat(
                                x_input, "", state.inspector.segment.start.x_buf,
                                &state.selection.segment->start_vertex( &state.storage ).x,
                                state.inspector.segment.start.x_edit
                            ) ) {
                                state.inspector.segment.start.x_edit =
                                    !state.inspector.segment.start.x_edit;
                            }

                            Rectangle y_input = x_input;
                            y_input.x += y_input.width + GUI_GUTTER;

                            if( GuiValueBoxFloat(
                                y_input, "", state.inspector.segment.start.y_buf,
                                &state.selection.segment->start_vertex( &state.storage ).y,
                                state.inspector.segment.start.y_edit
                            ) ) {
                                state.inspector.segment.start.y_edit =
                                    !state.inspector.segment.start.y_edit;
                            }
                        }

                        setting.y += setting.height + GUI_GUTTER;

                        text_width = MeasureTextEx( font, "Start", FONT_SIZE, 1.0 );
                        GuiLabel( setting, "End" ); {

                            Rectangle input_area = setting;
                            input_area.x     += text_width.x + GUI_GUTTER;
                            input_area.width -= text_width.x + GUI_GUTTER;

                            Rectangle x_input = input_area;
                            x_input.width = (x_input.width / 2.0) - (GUI_GUTTER / 2.0);

                            if( GuiValueBoxFloat(
                                x_input, "", state.inspector.segment.end.x_buf,
                                &state.selection.segment->end_vertex( &state.storage ).x,
                                state.inspector.segment.end.x_edit
                            ) ) {
                                state.inspector.segment.end.x_edit =
                                    !state.inspector.segment.end.x_edit;
                            }

                            Rectangle y_input = x_input;
                            y_input.x += y_input.width + GUI_GUTTER;

                            if( GuiValueBoxFloat(
                                y_input, "", state.inspector.segment.end.y_buf,
                                &state.selection.segment->end_vertex( &state.storage ).y,
                                state.inspector.segment.end.y_edit
                            ) ) {
                                state.inspector.segment.end.y_edit =
                                    !state.inspector.segment.end.y_edit;
                            }
                        }

                        setting.y += setting.height + GUI_GUTTER;

                        GuiLabel( setting, "Tint" );

                        Rectangle color_picker = setting;
                        color_picker.width  -= 200;
                        color_picker.height  = color_picker.width;
                        color_picker.x      += setting.width - (color_picker.width + 30);
                        GuiColorPicker( color_picker, "", &state.selection.segment->tint );

                        setting.y += color_picker.height + GUI_GUTTER;

                        Rectangle check_box = setting;

                        check_box.x     += check_box.width - (check_box.height + GUI_GUTTER);
                        check_box.width  = check_box.height;

                        GuiLabel( setting, "Flip Normal" );

                        bool active = state.selection.segment->is_flipped_normal;
                        if( GuiCheckBox( check_box, "", &active ) ) {
                            state.selection.segment->is_flipped_normal = active;
                        }

                        setting.y += setting.height + GUI_GUTTER;
                        check_box = setting;

                        check_box.x     += check_box.width - (check_box.height + GUI_GUTTER);
                        check_box.width  = check_box.height;

                        GuiLabel( setting, "Is Solid" );

                        active = state.selection.segment->is_solid;
                        if( GuiCheckBox( check_box, "", &active ) ) {
                            state.selection.segment->is_solid = active;
                        }
                    } break;
                }

            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
void set_mode( State& state, Mode new_mode ) {
    state.mode = new_mode;

    switch( state.mode ) {
        case Mode::SELECT: {
            state.selection = {};
        } break;
        case Mode::PUT: {
            state.put = {};
        } break;
        case Mode::BUILD: {
            state.build.previous_index = -1;
        } break;

        case Mode::COUNT:  break;
    }
}
struct PositionAndCenter {
    Vector3 p;
    Vector3 center;
};
float cross_product( Vector3 a, Vector3 b, Vector3 c ) {
    return (b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x);
}
bool point_in_triangle( Vector3 p, Vector3 a, Vector3 b, Vector3 c ) {
    Vector3 v0 = { c.x - a.x, 0, c.z - a.z };
    Vector3 v1 = { b.x - a.x, 0, b.z - a.z };
    Vector3 v2 = { p.x - a.x, 0, p.z - a.z };

    float dot00 = v0.x * v0.x + v0.y * v0.y;
    float dot01 = v0.x * v1.x + v0.y * v1.y;
    float dot02 = v0.x * v2.x + v0.y * v2.y;
    float dot11 = v1.x * v1.x + v1.y * v1.y;
    float dot12 = v1.x * v2.x + v1.y * v2.y;

    float denom = dot00 * dot11 - dot01 * dot01;
    if( !denom ) {
        return false;
    }

    float inv_denom = 1.0 / denom;
    float u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
    float v = (dot00 * dot12 - dot01 * dot02) * inv_denom;

    return (u >= 0) && (v >= 0) && (u + v <= 1);
}
bool is_ear(
    int previous, int current, int next,
    VertexBuffer* pos, IndexBuffer* idx, int count
) {
    Vector3 a = pos->buf[idx->buf[previous]];
    Vector3 b = pos->buf[idx->buf[current]];
    Vector3 c = pos->buf[idx->buf[next]];

    if( cross_product( a, b, c ) <= 0 ) {
        return false;
    }

    for( int i = 0; i < count; ++i ) {
        if( i == previous || i == current || i == next ) {
            continue;
        }
        Vector3 point = pos->buf[idx->buf[i]];

        if( point_in_triangle( point, a, b, c ) ) {
            return false;
        }
    }

    return true;
}
Vector3 center = {};
extern "C"
int v3_cmp_qsort( const void* vp1, const void* vp2 ) {
    Vector3 c  = center;
    Vector3 v1 = *(Vector3*)vp1;
    Vector3 v2 = *(Vector3*)vp2;

    float angle_v1 = atan2( v1.z - c.z, v1.x - c.x );
    float angle_v2 = atan2( v2.z - c.z, v2.x - c.x );

    int result = angle_v1 == angle_v2 ? 0 : angle_v1 > angle_v2 ? 1 : -1;

    TraceLog( LOG_INFO, "%f, %f -> %i", angle_v1, angle_v2, result );
    return result;
}
bool triangulate( State& state ) {
    if( !state.triangles_dirty || !state.storage.vertexes.len ) {
        return false;
    }
    state.triangles_dirty = false;

    auto* v   = &state.storage.vertexes;
    auto* pos = &state.triangle_positions;
    auto* idx = &state.triangle_indexes;

    pos->len = idx->len = 0;

    for( int i = 0; i < v->len; ++i ) {
        auto* vertex = v->buf + i;
        buf_append( pos, v3( vertex->position.x, 0.0, vertex->position.y ) );
    }
    // NOTE(alicia): sort into CCW
    center = {};
    for( int i = 0; i < pos->len; ++i ) {
        center += pos->buf[i];
    }
    center /= (float)pos->len;
    qsort( pos->buf, pos->len, sizeof(Vector3), v3_cmp_qsort );

    IndexBuffer idx2 = {};
    for( int i = 0; i < pos->len; ++i ) {
        buf_append( &idx2, i );
    }

    int rem = pos->len;
    while( rem > 3 ) {
        bool ear_found = false;

        for( int i = 0; i < rem; ++i ) {
            int previous = ( i - 1 + rem ) % rem;
            int current  = i;
            int next     = ( i + 1 ) % rem;

            if( is_ear( previous, current, next, pos, &idx2, rem ) ) {
                buf_append( idx, idx2.buf[current] );
                buf_append( idx, idx2.buf[previous] );
                buf_append( idx, idx2.buf[next] );

                for( int j = current; j < (rem - 1); ++j ) {
                    idx2.buf[j] = idx2.buf[j + 1];
                }
                rem--;

                ear_found = true;
                break;
            }
        }

        unused(ear_found);
    }

    buf_append( idx, idx2.buf[1] );
    buf_append( idx, idx2.buf[0] );
    buf_append( idx, idx2.buf[2] );

    free( idx2.buf );
    return true;
}
void hover_items(
    State& state, Vector2 mouse_world_position
) {
    for( int i = 0; i < state.storage.objects.len; ++i ) {
        auto* o = state.storage.objects.buf + i;
        bool is_hovering;

        ObjectShapeDescription shape_desc = object_shape( o->type );
        switch( shape_desc.shape ) {
            case ObjectShape::CIRCLE: {
                is_hovering = CheckCollisionPointCircle(
                    mouse_world_position, o->position, shape_desc.circle.radius );
            } break;
            case ObjectShape::RECTANGLE: {
                NonAxisAlignedRect rect = gen_non_axis_aligned_rect(
                    o->position, shape_desc.rectangle.size, o->rotation );

                Vector2 _min = { 20000, 20000 }, _max = { -200000, -200000 };
                for( int i = 0; i < 4; ++i ) {
                    Vector2 p = rect.points[i];
                    if( p.x < _min.x ) {
                        _min.x = p.x;
                    }
                    if( p.y < _min.y ) {
                        _min.y = p.y;
                    }
                    if( p.x > _max.x ) {
                        _max.x = p.x;
                    }
                    if( p.y > _max.y ) {
                        _max.y = p.y;
                    }
                }

                Rectangle r;
                r.x      = _min.x;
                r.y      = _min.y;
                r.width  = abs(_max.x - _min.x);
                r.height = abs(_max.y - _min.y);

                is_hovering = CheckCollisionPointRec( mouse_world_position, r );
            } break;
        }


        if( is_hovering ) {
            buf_append( &state.hover.object_indexes, i );
            state.hover.object_hovered = true;
        }
    }

    for( int i = 0; i < state.storage.vertexes.len; ++i ) {
        auto* v = state.storage.vertexes.buf + i;
        bool is_hovering = CheckCollisionPointCircle(
            mouse_world_position, v->position, VERTEX_RADIUS );

        if( is_hovering ) {
            buf_append( &state.hover.vertex_indexes, i );
            state.hover.vertex_hovered = true;
        }
    }

    for( int i = 0; i < state.storage.segments.len; ++i ) {
        auto* s = state.storage.segments.buf + i;
        Vector2 start = state.storage.vertexes.buf[s->start].position;
        Vector2 end   = state.storage.vertexes.buf[s->end].position;

        bool is_hovering = CheckCollisionCircleLine( mouse_world_position, 0.05, start, end );

        if( is_hovering ) {
            buf_append( &state.hover.segment_indexes, i );
            state.hover.segment_hovered = true;
        }
    }
}

const char* collate_modes() {
    struct {
        char* buf;
        int   len;
        int   cap;
    } result = {};

    int count = (int)Mode::COUNT;
    for( int i = 0; i < count; ++i ) {
        auto mode = (Mode)i;
        const char* name = to_string( mode );
        while( *name ) {
            buf_append( &result, *name++ );
        }

        if( i + 1 < count ) {
            buf_append( &result, ';' );
        }
    }
    buf_append( &result, 0 );
    return result.buf;
}
const char* collate_objects() {
    struct {
        char* buf;
        int   len;
        int   cap;
    } result = {};

    int count = (int)ObjectType::COUNT;
    for( int i = 0; i < count; ++i ) {
        auto type = (ObjectType)i;
        const char* name = to_string( type );
        while( *name ) {
            buf_append( &result, *name++ );
        }

        if( i + 1 < count ) {
            buf_append( &result, ';' );
        }
    }
    buf_append( &result, 0 );
    return result.buf;

}
const char* to_string( Mode mode ) {
    switch( mode ) {
        case Mode::SELECT: return TextFormat( "[%c] - Select Mode", mode_key( mode ) ); // "Select";
        case Mode::PUT:    return TextFormat( "[%c] - Put Mode", mode_key( mode ) );
        case Mode::BUILD:  return TextFormat( "[%c] - Build Mode", mode_key( mode ) );

        case Mode::COUNT: break;
    }
    return "";
}
const char* mode_description( Mode mode ) {
    switch( mode ) {
        case Mode::SELECT: return
            "LMB Press - Select item.\n"
            "LMB Drag - Move item.\n"
            "MMB Drag - Move camera.\n"
            "RMB Press - [Vertex] Remove vertex.\n"
            "RMB Press - [Segment] Collapse line segment.\n"
            "RMB Press - [Object] Remove object.\n"
            "Shift + RMB Press - [Vertex] Split vertex.\n"
            "Shift + RMB Press - [Segment] Split line segment."
            ;
        case Mode::PUT: return
            "LMB Press - Place object.\n"
            "MMB Drag  - Move camera.\n"
            "RMB Press - Remove object."
            ;
        case Mode::BUILD: return
            "LMB Press - Place vertex/start building lines.\n"
            "RMB Press - Stop building."
            ;
        case Mode::COUNT: break;
    }
    return "";
}
int mode_key( Mode mode ) {
    switch( mode ) {
        case Mode::SELECT:  return KEY_Q;
        case Mode::PUT:     return KEY_W;
        case Mode::BUILD:   return KEY_E;
        case Mode::COUNT:   break;
    }
    return KEY_NULL;
}

__attribute__((hot)) __attribute__((always_inline)) inline
void _0_( int v, ... ) {
    (void)v;
}
void DrawRectangleLinesRec( Rectangle rect, Color color ) {
    Vector2 pos  = { rect.x, rect.y };
    Vector2 size = { rect.width, rect.height };
    DrawLineV( pos, pos + Vector2{ size.x, 0.0 }, color );
    DrawLineV( pos + Vector2{ size.x, 0.0 }, pos + Vector2{ size.x, size.y }, color );
    DrawLineV( pos + Vector2{ size.x, size.y }, pos + Vector2{ 0, size.y }, color );
    DrawLineV( pos + Vector2{ 0, size.y }, pos, color );
}

// Thank you GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wenum-compare"
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

#pragma GCC diagnostic pop

