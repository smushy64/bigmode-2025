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
#define WINDOW_NAME   "Bolt Bot"

#define LEVEL_NAME_TIME       2.0
#define LEVEL_NAME_BEGIN_FADE 1.0

#define LEVEL_EXIT_TIME      (1.8)
#define LEVEL_EXIT_FADE_TIME (LEVEL_EXIT_TIME - 0.6)

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

    Shader sh_wall;
    int    sh_wall_loc_camera_position;
    int    sh_wall_loc_dist;
    int    sh_wall_loc_apply_dist;
    int    sh_wall_loc_clipping_planes;

    struct {
        Font    font;
        Texture tex_main_menu;
    } persistent;

    union {
        struct {
        } intro;
        struct {
            Music music;
            bool is_options_open;
            bool is_credits_open;
        } main_menu;
        struct {
            bool  is_paused;
            bool  is_exiting_stage;
            float exit_stage_timer;
            float level_timer;

            int enemy_counter;
            int total_enemy_count;

            int battery_counter;
            int total_battery_count;

            GuiPauseMenu pause_menu_state;
            Camera3D     camera;
            Player       player;
            LevelCondition condition;

            struct {
                Model bot;
                Model wall;
                Model battery;
                Model level_exit;
                Model floor_ceiling;
            } models;
            struct {
                Texture test;
                Texture white;
                Texture battery;
                Texture floor;
                Texture wall;
                Texture wall2;
                Texture wall3;
                Texture wall4;
                Texture ceiling;
            } textures;
            struct {
                Material bot;
                Material enemy;
                Material wall;
                Material floor;
                Material ceiling;
                Material battery;
                Material level_exit;
            } materials;
            struct {
                MaterialMap bot;
                MaterialMap enemy;
                MaterialMap wall;
                MaterialMap floor;
                MaterialMap ceiling;
                MaterialMap battery;
                MaterialMap level_exit;
            } material_maps;

            struct {
                ModelAnimation* buf;
                int             len;
            } animations;

            struct {
                SoundBuffer step;
                SoundBuffer dash;
                SoundBuffer whiff;
                SoundBuffer punch;
                SoundBuffer death;
                SoundBuffer powerup;
                SoundBuffer fallapart;
                SoundBuffer takedamage;
                SoundBuffer nextlevel;
            } sounds;

            Music music;

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
    mode_unload( state, state->mode );
    mode_load( state, mode );
}

#endif /* header guard */
