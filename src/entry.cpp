/**
 * @file   entry.cpp
 * @brief  Entry Points.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 23, 2025
*/
#include <stdlib.h>
#include "raylib.h"

struct GlobalState {
    Texture bigmode_logo;
} *global_state;


bool initialize() {
    global_state = (GlobalState*)malloc( sizeof(*global_state) );
    if( !global_state ) {
        return false;
    }
    auto* state = global_state;

    state->bigmode_logo = LoadTexture( "resources/textures/bigmode/logo-text.png" );

    return true;
}

void update() {
    auto* state = global_state;
    BeginDrawing();

    DrawTextureEx( state->bigmode_logo, Vector2{}, 0.0, 0.5, WHITE );

    EndDrawing();
}

