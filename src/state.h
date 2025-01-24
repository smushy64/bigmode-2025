#if !defined(STATE_H)
#define STATE_H
/**
 * @file   state.h
 * @brief  Game state.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "raylib.h"
#include "modes.h"
#include "gui.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720
#define WINDOW_NAME   "BIGMODE Game Jam 2025"

struct GlobalState {
    float timer;

    Mode mode;
    union {
        struct {
            Texture bigmode_logo;
        } intro;
        struct {
            bool is_options_open;
            bool is_credits_open;
            GuiOptionsMenu options_state;
        } main_menu;
        struct {

        } game;
    };

    void load_mode( Mode mode );
    void unload_mode( Mode mode );

    inline
    void set_mode( Mode mode ) {
        unload_mode( this->mode );
        load_mode( mode );
    }
};

#endif /* header guard */
