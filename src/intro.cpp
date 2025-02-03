/**
 * @file   intro.cpp
 * @brief  Intro mode.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "modes.h"
#include "state.h"
#include "raylib.h"
#include "blit.h"

void mode_intro_load( GlobalState* state ) {
    if( !IsTextureValid( state->persistent.tex_main_menu ) ) {
        state->persistent.tex_main_menu = LoadTexture( "resources/textures/logo.png" );
        SetTextureFilter(
            state->persistent.tex_main_menu, TEXTURE_FILTER_BILINEAR );
    }
}
void mode_intro_update( GlobalState* state, float dt ) {
    #define INTRO_LENGTH 1.25

    if( state->timer >= INTRO_LENGTH ) {
        mode_set( state, Mode::MAIN_MENU );
        return;
    }

    BeginDrawing();
    ClearBackground( { 20, 199, 195, 255 } );

    Vector2 screen = get_screen();

    Rectangle src, dst;

    src = {};
    rect_set_size( src, texture_size( state->persistent.tex_main_menu ) );
    src.y      = 1.0;
    src.height = 190.0;

    dst = centered_fit_to_screen( screen, rect_size(src) );

    float t = state->timer / INTRO_LENGTH;

    t = fmin( sin( t * M_PI ) * 2.0, 1.0 );

    DrawTexturePro( state->persistent.tex_main_menu, src, dst, {}, 0.0, WHITE );

    Color tint = ColorLerp( BLACK, {0, 0, 0, 0}, t );
    DrawRectangle( 0, 0, screen.x, screen.y, tint );
    EndDrawing();
}
void mode_intro_unload( GlobalState* state ) {
}

#undef INTRO_LENGTH

