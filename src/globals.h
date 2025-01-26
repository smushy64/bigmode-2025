#if !defined(GLOBALS_H)
#define GLOBALS_H
/**
 * @file   globals.h
 * @brief  Globals.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 25, 2025
*/
#include "raylib.h"

Vector2 OptionCameraSensitivity();
void OptionCameraSensitivity( Vector2 new_sensitivity );

bool OptionMute();
void OptionMute( bool is_muted );

float OptionVolume();
void  OptionVolume( float scale );

float OptionVolumeMusic();
void  OptionVolumeMusic( float scale );

float OptionVolumeSFX();
void  OptionVolumeSFX( float scale );

bool OptionFXAA();
void OptionFXAA( bool is_on );

bool OptionInverseX();
void OptionInverseX( bool is_on );

bool OptionInverseY();
void OptionInverseY( bool is_on );

Font GameFont();
void GameFont( Font font );

#endif /* header guard */
