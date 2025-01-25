/**
 * @file   main.cpp
 * @brief  Main function.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 23, 2025
*/
#include "raylib.h"
#include "entry.cpp"
#include "state.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

bool should_exit = false;

void game_exit() {
#if !defined(PLATFORM_WEB)
    TraceLog( LOG_INFO, "Exiting game . . ." );
    should_exit = true;
#endif
}

int main() {
    InitWindow( WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME );

    if( !initialize() ) {
        return 1;
    }

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop( update, 0, 1 );
#else
    SetExitKey( KEY_ZERO );
    SetTargetFPS( 60 );
    while( !WindowShouldClose() ) {
        update();
        if( should_exit ) {
            break;
        }
    }
#endif

    CloseWindow();
    return 0;
}

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "blit.cpp"
#include "gui.cpp"
#include "intro.cpp"
#include "main_menu.cpp"
#include "game.cpp"

