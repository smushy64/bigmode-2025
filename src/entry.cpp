/**
 * @file   entry.cpp
 * @brief  Entry Points.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 23, 2025
*/
#include <stdlib.h>
#include "raylib.h"
#include "raygui.h"

#include "blit.h"
#include "modes.h"
#include "state.h"
#include "gui.h"
#include "shaders.h"
#include "globals.h"

#include <string.h>

#define DEBUG_START_MODE Mode::GAME

GlobalState* global_state;

void on_resize( GlobalState* state ) {
    Vector2 resolution = {(float)GetRenderWidth(), (float)GetRenderHeight()};
    state->rt = LoadRenderTexture( resolution.x, resolution.y );
    SetTextureFilter( state->rt.texture, TEXTURE_FILTER_BILINEAR );

    SetShaderValue(
        state->sh_post_process,
        state->sh_post_process_loc_resolution,
        &resolution, SHADER_UNIFORM_VEC2 );
}
bool initialize() {
    global_state = (GlobalState*)malloc( sizeof(*global_state) );
    if( !global_state ) {
        return false;
    }
    memset( global_state, 0, sizeof(*global_state) );
    auto* state = global_state;

    state->sh_basic_shading = LoadShaderFromMemory( basic_shading_vert, basic_shading_frag );
    state->sh_basic_shading_loc_camera_position =
        GetShaderLocation( state->sh_basic_shading, "camera_position" );

    state->sh_post_process = LoadShaderFromMemory( 0, post_process_frag );
    state->sh_post_process_loc_resolution =
        GetShaderLocation( state->sh_post_process, "resolution" );
    on_resize( state );

    state->persistent.font =
        LoadFont( "resources/ui/fonts/RobotoCondensed/RobotoCondensed-Medium.ttf" );

    GameFont( state->persistent.font );

    SetTextureFilter(
        state->persistent.font.texture, TEXTURE_FILTER_BILINEAR );

#if defined(RELEASE)
    mode_load( state, Mode::INTRO );
#else
    mode_load( state, DEBUG_START_MODE );
#endif

    GuiLoadStyle( "resources/ui/styles/dark/style_dark.rgs" );
    GuiSetFont( state->persistent.font );
    GuiSetStyle( DEFAULT, TEXT_SIZE, 18 );
    return true;
}

extern bool should_exit;
void update() {
    auto* state = global_state;

    float dt = GetFrameTime();

    if( IsWindowResized() ) {
        UnloadRenderTexture( state->rt );
        on_resize( state );
    }

    switch( state->mode ) {
        case Mode::INTRO: {
            mode_intro_update( state, dt );
        } break;
        case Mode::MAIN_MENU: {
            mode_main_menu_update( state, dt );
        } break;
        case Mode::GAME: {
            mode_game_update( state, dt );
        } break;
    }

    state->timer += dt;
}

void mode_load( GlobalState* state, Mode mode ) {
    switch( mode ) {
        case Mode::INTRO: {
            mode_intro_load( state );
        } break;
        case Mode::MAIN_MENU: {
            mode_main_menu_load( state );
        } break;
        case Mode::GAME: {
            mode_game_load( state );
        } break;
    }

    TraceLog( LOG_INFO, "Loaded mode %s.", to_string(mode) );
    state->mode  = mode;
    state->timer = 0.0;
}
void mode_unload( GlobalState* state, Mode mode ) {
    switch( mode ) {
        case Mode::INTRO: {
            mode_intro_unload( state );
        } break;
        case Mode::MAIN_MENU: {
            mode_main_menu_unload( state );
        } break;
        case Mode::GAME: {
            mode_game_unload( state );
        } break;
    }

    memset( &state->transient, 0, sizeof(state->transient) );

    TraceLog( LOG_INFO, "Unloaded mode %s.", to_string(mode) );
}


