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

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720
#define WINDOW_NAME   "BIGMODE Game Jam 2025"

struct GlobalState {
    Mode          mode;
    float         timer;
    RenderTexture rt;

    Shader sh_post_process;
    int    sh_post_process_resolution_location;

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
            GuiOptionsMenu options_state;
        } main_menu;
        struct {
            Camera3D camera;
            Player   player;
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
