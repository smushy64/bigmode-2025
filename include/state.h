#if !defined(STATE_H)
#define STATE_H
/**
 * @file   state.h
 * @brief  Game state.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "raylib.h"
#include "modes.h"
#include "gui.h"
#include "player.h"
#include "shared/object.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720
#define WINDOW_NAME   "BIGMODE Game Jam 2025"

struct SoundBuffer {
    Sound* buf;
    int    len;
    int    cap;
};

struct Segment {
    int start, end;
};
struct GlobalState {
    Mode          mode;
    float         timer;
    RenderTexture rt;

    Shader sh_post_process;
    int    sh_post_process_loc_resolution;

    Shader sh_basic_shading;
    int    sh_basic_shading_loc_camera_position;

    struct {
        Font font;
    } persistent;

    union {
        struct {
            Texture bigmode_logo;
        } intro;
        struct {
            bool is_options_open;
            bool is_credits_open;
        } main_menu;
        struct {
            bool         is_paused;
            GuiPauseMenu pause_menu_state;
            Camera3D     camera;
            Player       player;

            struct {
                Model player;
                Model wall;
            } models;
            struct {
                Texture test;
                Texture white;
            } textures;

            struct {
                ModelAnimation* buf;
                int             len;
            } player_animation;

            struct {
                SoundBuffer step;
                SoundBuffer dash;
                SoundBuffer whiff;
                SoundBuffer punch;
            } sounds;

            struct {
                Object* buf;
                int     len;
                int     cap;
            } objects;
            struct {
                Vector2* buf;
                int      len;
                int      cap;
            } vertexes;
            struct {
                Segment* buf;
                int      len;
                int      cap;
            } segments;
        } game;
    } transient;
};

void game_exit();

void mode_load( GlobalState* state, Mode mode );
void mode_unload( GlobalState* state, Mode mode );

inline
void mode_set( GlobalState* state, Mode mode ) {
    mode_unload( state, mode );
    mode_load( state, mode );
}

#endif /* header guard */
