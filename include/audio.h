#if !defined(AUDIO_H)
#define AUDIO_H
/**
 * @file   audio.h
 * @brief  Audio
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 28, 2025
*/
#include "raylib.h"

void play_sfx( Vector2 src, Vector2 listener, Sound sound );
int  play_sfx_random( Vector2 src, Vector2 listener, Sound* buf, int len );

float sound_length( const Sound& sound );

#endif /* header guard */
