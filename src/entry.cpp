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

#include <string.h>

#define START_MODE Mode::INTRO

GlobalState* global_state;

bool initialize() {
    global_state = (GlobalState*)malloc( sizeof(*global_state) );
    if( !global_state ) {
        return false;
    }
    auto* state = global_state;

    state->load_mode( START_MODE );

    GuiLoadStyle( "resources/ui/styles/dark/style_dark.rgs" );
    return true;
}

extern bool should_exit;
void update() {
    auto* state = global_state;
    BeginDrawing();

    float dt = GetFrameTime();

    switch( state->mode ) {
        case Mode::INTRO: {
            if( !mode_intro( state, dt ) ) {
                should_exit = true;
            }
        } break;
        case Mode::MAIN_MENU: {
            if( !mode_main_menu( state, dt ) ) {
                should_exit = true;
            }
        } break;
        case Mode::GAME: {
            if( !mode_game( state, dt ) ) {
                should_exit = true;
            }
        } break;
    }

    state->timer += dt;

    EndDrawing();
}

void GlobalState::load_mode( Mode mode ) {
    switch( mode ) {
        case Mode::INTRO: {
            intro.bigmode_logo = LoadTexture( "resources/textures/logo.png" );
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

    char* clear_from = (char*)(&this->mode + 1);
    memset( clear_from, 0, sizeof(GlobalState) - (clear_from - (char*)this) );

    TraceLog( LOG_INFO, "Unloaded mode %s.", to_string(mode) );
}

bool mode_intro( GlobalState* state, float dt ) {
    #define INTRO_LENGTH 1.25

    if( state->timer >= INTRO_LENGTH ) {
        state->set_mode( Mode::MAIN_MENU );
        return true;
    }

    ClearBackground( { 20, 199, 195, 255 } );

    Vector2 screen = get_screen();

    Rectangle src, dst;

    src = {};
    rect_set_size( src, texture_size( state->intro.bigmode_logo ) );
    src.y      = 1.0;
    src.height = 94.0;

    dst = centered_fit_to_screen( screen, rect_size(src) );

    float t = state->timer / INTRO_LENGTH;

    t = fmin( sin( t * M_PI ) * 2.0, 1.0 );

    DrawTexturePro( state->intro.bigmode_logo, src, dst, {}, 0.0, WHITE );

    Color tint = ColorLerp( BLACK, {0, 0, 0, 0}, t );
    DrawRectangle( 0, 0, screen.x, screen.y, tint );

    return true;
}
bool mode_main_menu( GlobalState* state, float dt ) {
    ClearBackground( BLACK );

    Vector2 screen = get_screen();

    if( !(state->main_menu.is_credits_open || state->main_menu.is_options_open) ) {

        Vector2 text_measure = MeasureTextEx(
            GetFontDefault(), "BIGMODE Game Jam 2025", 24.0, 1.0 );
        DrawTextPro(
            GetFontDefault(), "BIGMODE Game Jam 2025",
            {screen.x / 2.0f, (screen.y / 2.0f) - 24.0f},
            text_measure / 2.0, 0.0, 24.0, 1.0, WHITE );

        Rectangle button_rect = { screen.x / 2.0f, screen.y / 2.0f, 95.0, 35.0 };
        button_rect.x -= button_rect.width / 2.0;

        float spacing = button_rect.height + 6.0;

        if( GuiButton( button_rect, "Start Game" ) ) {
            state->set_mode( Mode::GAME );
        }

        button_rect.y += spacing;

        if( GuiButton( button_rect, "Options" ) ) {
            state->main_menu.is_options_open = !state->main_menu.is_options_open;
        }

        button_rect.y += spacing;

        if( GuiButton( button_rect, "Credits" ) ) {
            state->main_menu.is_credits_open = !state->main_menu.is_credits_open;
        }

#if !defined(PLATFORM_WEB)
        button_rect.y += spacing;

        if( GuiButton( button_rect, "Quit Game" ) ) {
            return false;
        }
#endif

    } else {
        if( state->main_menu.is_options_open ) {
            if( draw_options_menu( state->main_menu.options_state ) ) {
                state->main_menu.is_options_open = false;
            }
        }

        if( state->main_menu.is_credits_open ) {
            if( draw_credits_menu() ) {
                state->main_menu.is_credits_open = false;
            }
        }
    }

    return true;
}
bool mode_game( GlobalState* state, float dt ) {
    ClearBackground( BLACK );
    return true;
}


