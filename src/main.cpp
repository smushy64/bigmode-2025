/**
 * @file   main.cpp
 * @brief  Main function.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 23, 2025
*/
#include "raylib.h"
#include "entry.cpp"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

int main() {
    InitWindow( 800, 600, "Hello, World!" );

    if( !initialize() ) {
        return 1;
    }

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

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "blit.cpp"

