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

struct GuiOptionsMenu {
};

bool draw_options_menu( GuiOptionsMenu& state );
bool draw_credits_menu();

bool draw_pause_menu();

#endif /* header guard */
