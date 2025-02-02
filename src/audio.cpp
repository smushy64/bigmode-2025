/**
 * @file   audio.cpp
 * @brief  Audio implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 28, 2025
*/
#include "audio.h"
#include "globals.h"
#include "raymath.h"

#define SFX_ATTENUATION_DISTANCE_START (6.0)
#define SFX_ATTENUATION_DISTANCE_END   (12.0)

#define SFX_ATTENUATION_DISTANCE_START_SQR \
    (SFX_ATTENUATION_DISTANCE_START * SFX_ATTENUATION_DISTANCE_START)
#define SFX_ATTENUATION_DISTANCE_END_SQR \
    (SFX_ATTENUATION_DISTANCE_END * SFX_ATTENUATION_DISTANCE_END)

float InverseLerp( float a, float b, float v ) {
    return (v - a) / (b - a);
}

void play_sfx( Vector2 src, Vector2 listener, Sound sound, float v ) {
    float atten; {
        float dist_sqr = Vector2DistanceSqr( src, listener );
        if( dist_sqr < SFX_ATTENUATION_DISTANCE_START_SQR ) {
            atten = 1.0;
        } else if( dist_sqr > SFX_ATTENUATION_DISTANCE_END_SQR ) {
            atten = 0.0;
        } else {
            float t = InverseLerp(
                SFX_ATTENUATION_DISTANCE_START_SQR,
                SFX_ATTENUATION_DISTANCE_END_SQR, dist_sqr );

            atten = 1.0 - t;
        }
    }

    float volume = OptionVolume() * OptionVolumeSFX() * atten * v;

    float pitch_offset = ((float)GetRandomValue( -100, 100 ) / 100.0f) * 0.2;

    SetSoundPitch( sound, 1.0 + pitch_offset );
    SetSoundVolume( sound, volume );
    PlaySound( sound );
}
int play_sfx_random( Vector2 src, Vector2 listener, Sound* buf, int len, float volume ) {
    if( !buf || !len ) {
        return 0;
    }
    int idx = GetRandomValue( 0, 100000 ) % len;

    play_sfx( src, listener, buf[idx], volume );
    return idx;
}
float sound_length( const Sound& s ) {
    float length_seconds =
        (s.frameCount / (float)s.stream.channels) / (float)s.stream.sampleRate;
    return length_seconds;
}


