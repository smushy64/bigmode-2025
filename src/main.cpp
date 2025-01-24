/**
 * @file   main.cpp
 * @brief  Entry point of program.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 23, 2025
*/
#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void update();

int main() {
    InitWindow( 800, 600, "Hello, World!" );

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop( update, 0, 1 );
#else
    SetTargetFPS(60);
    while( !WindowShouldClose() ) {
        update();
    }
#endif

    CloseWindow();
    return 0;
}

void update() {
    static bool first_frame = true;

    Texture bigmode_logo;

    if( first_frame ) {
        first_frame = false;
        bigmode_logo = LoadTexture( "resources/textures/bigmode/logo-text.png" );
    }
    BeginDrawing();
    DrawTextureEx( bigmode_logo, Vector2{}, 0.0, 0.5, WHITE );
    EndDrawing();
}

