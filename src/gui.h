#if !defined(GUI_H)
#define GUI_H
/**
 * @file   gui.h
 * @brief  GUI helper functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "raylib.h"
#include "raygui.h"


struct GuiPauseMenu {
    bool is_options_open;
    bool quit_to_menu;
};

bool draw_pause_menu( GuiPauseMenu& state );
bool draw_options_menu();
bool draw_credits_menu();


#endif /* header guard */
