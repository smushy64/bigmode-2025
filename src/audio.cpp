/**
 * @file   audio.cpp
 * @brief  Audio implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 28, 2025
*/
#include "audio.h"
#include "globals.h"

void play_sfx( Sound sound ) {
    float volume = OptionVolume() * OptionVolumeSFX();
    SetSoundVolume( sound, volume );
    PlaySound( sound );
}
int play_sfx_random( Sound* buf, int len ) {
    if( !buf || !len ) {
        return 0;
    }
    int idx = GetRandomValue( 0, 100000 ) % len;

    play_sfx( buf[idx] );
    return idx;
}
float sound_length( const Sound& s ) {
    float length_seconds =
        (s.frameCount / (float)s.stream.channels) / (float)s.stream.sampleRate;
    return length_seconds;
}


