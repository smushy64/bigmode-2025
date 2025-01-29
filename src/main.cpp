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
#else
    #if defined(_WIN32)
        #define _dllexport_ __declspec(dllexport)
    #else
        #define _dllexport_
    #endif
    
    extern "C" {
        _dllexport_
        unsigned long NvOptimusEnablement = 1;
        _dllexport_
        int AmdPowerXpressRequestHighPerformance = 1;
    }
#endif

bool should_exit = false;

void game_exit() {
#if !defined(PLATFORM_WEB)
    TraceLog( LOG_INFO, "Exiting game . . ." );
    should_exit = true;
#endif
}

int main() {
#if !defined(PLATFORM_WEB)
    SetConfigFlags( FLAG_VSYNC_HINT );
#endif
    InitWindow( WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME );
    InitAudioDevice();

    if( !initialize() ) {
        return 1;
    }

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop( update, 0, 1 );
#else
    SetExitKey( KEY_ZERO );
    while( !WindowShouldClose() ) {
        update();
        if( should_exit ) {
            break;
        }
    }
#endif

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

#include "blit.cpp"
#include "gui.cpp"
#include "intro.cpp"
#include "main_menu.cpp"
#include "game.cpp"
#include "audio.cpp"
#include "globals.cpp"
#include "shaders.cpp"

// Thank you GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wenum-compare"
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#pragma GCC diagnostic pop


