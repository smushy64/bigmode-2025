/**
 * @file   main.cpp
 * @brief  Editor main.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 26, 2025
*/
#include "raylib.h"
#include "raygui.h"
#include "raymath.h"
#include "rlgl.h"

#include "editor/map.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

inline
void _0_( int v, ... );
#define unused(...) _0_( 0, ##__VA_ARGS__ )

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

struct State {
    MapStorage  storage;
    Selection   selection;
    HoverResult hover;
    Mode        mode;

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
    State& state = *_ptr_state; {
        EdVertex vertexes[] = {
            { Vector2{ 0, 0 } },
            { Vector2{ 0, 5 } },
            { Vector2{ 5, 5 } },
            { Vector2{ 5, 0 } },
        };
        buf_append( &state.storage.vertexes, vertexes[0] );
        buf_append( &state.storage.vertexes, vertexes[1] );
        buf_append( &state.storage.vertexes, vertexes[2] );
        buf_append( &state.storage.vertexes, vertexes[3] );

        EdSegment segments[] = {
            { 0, 1, WHITE, 0, { .is_flipped_normal = false, .is_solid = true } },
            { 1, 2, WHITE, 0, { .is_flipped_normal = false, .is_solid = true } },
            { 2, 3, WHITE, 0, { .is_flipped_normal = false, .is_solid = true } },
            { 3, 0, WHITE, 0, { .is_flipped_normal = false, .is_solid = true } },
        };
        buf_append( &state.storage.segments, segments[0] );
        buf_append( &state.storage.segments, segments[1] );
        buf_append( &state.storage.segments, segments[2] );
        buf_append( &state.storage.segments, segments[3] );

        EdObject obj = {
            .position = {2.5, 2.5},
            .type     = ObjectType::PLAYER_SPAWN
        };
        buf_append( &state.storage.objects, obj );
    }

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
            hover_items( state, world_mouse );
        }

        switch( state.mode ) {
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

                if( state.hover.object_exclusive() && lmb ) {
                    for( int idx = 0; idx < state.hover.object_indexes.len; ++idx ) {
                        auto* o =
                            state.storage.objects.buf +
                            state.hover.object_indexes.buf[idx];
                        o->position += world_delta;
                    }
                }
                if( state.hover.vertex_exclusive() && lmb ) {
                    for( int idx = 0; idx < state.hover.vertex_indexes.len; ++idx ) {
                        auto* v =
                            state.storage.vertexes.buf + 
                            state.hover.vertex_indexes.buf[idx];
                        v->position += world_delta;
                    }
                }
                if( state.hover.segment_exclusive() && lmb ) {
                    for( int idx = 0; idx < state.hover.segment_indexes.len; ++idx ) {
                        auto* s =
                            state.storage.segments.buf + 
                            state.hover.segment_indexes.buf[idx];
                        Vector2& start = state.storage.vertexes.buf[s->start].position;
                        Vector2& end   = state.storage.vertexes.buf[s->end].position;

                        start += world_delta;
                        end   += world_delta;
                    }
                }
            } break;

            case Mode::COUNT: break;
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

                                Vector2 _min = { 20000, 20000 }, _max = {};
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
                                rect.width  = (_max.x - _min.x) + 0.2;
                                rect.height = (_max.y - _min.y) + 0.2;
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
                            state.storage.vertexes.buf[state.selection.segment->start].position;
                        Vector2 end =
                            state.storage.vertexes.buf[state.selection.segment->end].position;
                        DrawLineV( start, end, GREEN );
                    } break;
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
            Vector2 mode_description_size = MeasureTextEx(
                font, mode_description( state.mode ), FONT_SIZE, 1.0 );
            Vector2 mode_description_position = {
                map_area.x + GUI_GUTTER,
                map_area.y + (map_area.height - mode_description_size.y) };
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
            GuiPanel( side_bar, "Inspector" ); {
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
    state.selection = {};
    state.mode      = new_mode;

    switch( state.mode ) {
        case Mode::SELECT: break;

        case Mode::COUNT:  break;
    }
}
void hover_items(
    State& state, Vector2 mouse_world_position
) {
    state.hover.clear();

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

                Vector2 _min = { 20000, 20000 }, _max = {};
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
                r.width  = (_max.x - _min.x);
                r.height = (_max.y - _min.y);

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
            "Shift + RMB Press - [Vertex] Split vertex.\n"
            "Shift + RMB Press - [Segment] Split line segment.\n"
            ;
        case Mode::COUNT: break;
    }
    return "";
}
int mode_key( Mode mode ) {
    switch( mode ) {
        case Mode::SELECT:  return KEY_Q;
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

