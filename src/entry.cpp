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

#define INTRO_MAX_TIME               2.5
#define INTRO_LOGO_TIME_LOGO_VISIBLE 1.2

GlobalState* global_state;

bool initialize() {
    global_state = (GlobalState*)malloc( sizeof(*global_state) );
    if( !global_state ) {
        return false;
    }
    auto* state = global_state;

    state->load_mode( Mode::INTRO );

    GuiLoadStyle( "resources/ui/styles/dark/style_dark.rgs" );
    return true;
}

void update() {
    auto* state = global_state;
    BeginDrawing();
    ClearBackground( BLACK );

    float dt = GetFrameTime();

    switch( state->mode ) {
        case Mode::INTRO: {
            mode_intro( state, dt );
        } break;
        case Mode::MAIN_MENU: {
            mode_main_menu( state, dt );
        } break;
        case Mode::GAME: {
            mode_game( state, dt );
        } break;
    }

    state->timer += dt;

    EndDrawing();
}

void GlobalState::load_mode( Mode mode ) {
    switch( mode ) {
        case Mode::INTRO: {
            intro.bigmode_logo = LoadTexture( "resources/textures/bigmode/logo-text.png" );
            SetTextureFilter( intro.bigmode_logo, TEXTURE_FILTER_BILINEAR );
        } break;
        case Mode::MAIN_MENU: {

        } break;
        case Mode::GAME: {

        } break;
    }

    TraceLog( LOG_INFO, "Loaded mode %s.", to_string(mode) );
    this->mode  = mode;
    this->timer = 0.0;
}
void GlobalState::unload_mode( Mode mode ) {
    switch( mode ) {
        case Mode::INTRO: {
            UnloadTexture( intro.bigmode_logo );
        } break;
        case Mode::MAIN_MENU: {

        } break;
        case Mode::GAME: {

        } break;
    }

    TraceLog( LOG_INFO, "Unloaded mode %s.", to_string(mode) );
}

void mode_intro( GlobalState* state, float dt ) {
    if( state->timer >= INTRO_MAX_TIME ) {
        state->set_mode( Mode::MAIN_MENU );
        return;
    }

    Vector2 screen;
    screen.x = GetScreenWidth();
    screen.y = GetScreenHeight();

    Rectangle src, dst;

    src = {};
    rect_set_size( src, texture_size( state->intro.bigmode_logo ) );

    dst = centered_fit_to_screen( screen, texture_size( state->intro.bigmode_logo ) );

    Color tint = ColorLerp( Color{}, WHITE, state->timer / INTRO_LOGO_TIME_LOGO_VISIBLE );

    DrawTexturePro( state->intro.bigmode_logo, src, dst, {}, 0.0, tint );
}
void mode_main_menu( GlobalState* state, float dt ) {

}
void mode_game( GlobalState* state, float dt ) {

}


