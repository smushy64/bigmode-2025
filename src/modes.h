#if !defined(MODES_H)
#define MODES_H
/**
 * @file   modes.h
 * @brief  Game modes.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/

struct GlobalState;
enum class Mode {
    INTRO,
    MAIN_MENU,
    GAME,
};

void mode_intro_load( GlobalState* state );
void mode_main_menu_load( GlobalState* state );
void mode_game_load( GlobalState* state );

void mode_intro_update( GlobalState* state, float dt );
void mode_main_menu_update( GlobalState* state, float dt );
void mode_game_update( GlobalState* state, float dt );

void mode_intro_unload( GlobalState* state );
void mode_main_menu_unload( GlobalState* state );
void mode_game_unload( GlobalState* state );

inline
const char* to_string( Mode mode ) {
    switch( mode ) {
        case Mode::INTRO:     return "Intro";
        case Mode::MAIN_MENU: return "Main Menu";
        case Mode::GAME:      return "Game";
    }
    return "Unknown";
}

#endif /* header guard */
