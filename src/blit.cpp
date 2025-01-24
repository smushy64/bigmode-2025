/**
 * @file   blit.cpp
 * @brief  Functions for blitting images.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "blit.h"

void rect_set_pos( Rectangle& rect, Vector2 pos ) {
    *(Vector2*)&rect = pos;
}
void rect_set_size( Rectangle& rect, Vector2 size ) {
    *(Vector2*)&rect.width = size;
}

Vector2 texture_size( Texture& texture ) {
    return Vector2{ (float)texture.width, (float)texture.height };
}

Rectangle centered_fit_to_screen( Vector2 dst_size, Vector2 src_size ) {
    Rectangle result = {};
    rect_set_size( result, fit_to_screen( dst_size, src_size ) );
    if( FloatEquals( result.width, dst_size.x ) ) {
        result.y = (dst_size.y - result.height) / 2.0;
    } else {
        result.x = (dst_size.x - result.width) / 2.0;
    }
    return result;
}

Vector2 fit_to_screen( Vector2 dst, Vector2 src ) {
    float src_aspect = src.x / src.y;
    float dst_aspect = dst.x / dst.y;

    Vector2 result;
    if( dst_aspect > src_aspect ) {
        result.x = src.x * (dst.y / src.y);
        result.y = dst.y;
    } else {
        result.x = dst.x;
        result.y = src.y * (dst.x / src.x);
    }
    return result;
}

