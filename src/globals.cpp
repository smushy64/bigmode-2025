/**
 * @file   globals.cpp
 * @brief  Globals.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 25, 2025
*/
#include "globals.h"

struct Globals {
    Vector2 camera_sensitivity = { 1.0, 1.0 };
    bool    fxaa_on            = true;

    bool inverse_x = false;
    bool inverse_y = false;

    bool  mute         = false;
    float volume       = 0.5;
    float volume_music = 1.0;
    float volume_sfx   = 1.0;

    Font game_font;
} globals;

Vector2 OptionCameraSensitivity() {
    return globals.camera_sensitivity;
}
void OptionCameraSensitivity( Vector2 new_sensitivity ) {
    globals.camera_sensitivity = new_sensitivity;
}

bool OptionMute() {
    return globals.mute;
}
void OptionMute( bool is_muted ) {
    globals.mute = is_muted;
}

bool OptionInverseX() {
    return globals.inverse_x;
}
void OptionInverseX( bool is_on ) {
    globals.inverse_x = is_on;
}

bool OptionInverseY() {
    return globals.inverse_y;
}
void OptionInverseY( bool is_on ) {
    globals.inverse_y = is_on;
}


float OptionVolume() {
    return globals.volume;
}
void  OptionVolume( float scale ) {
    globals.volume = scale;
}

float OptionVolumeMusic() {
    return globals.volume_music;
}
void  OptionVolumeMusic( float scale ) {
    globals.volume_music = scale;
}

float OptionVolumeSFX() {
    return globals.volume_sfx;
}
void  OptionVolumeSFX( float scale ) {
    globals.volume_sfx = scale;
}

bool OptionFXAA() {
    return globals.fxaa_on;
}
void OptionFXAA( bool is_on ) {
    globals.fxaa_on = is_on;
}

Font GameFont() {
    return globals.game_font;
}
void GameFont( Font font ) {
    globals.game_font = font;
}



