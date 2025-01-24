#if !defined(BLIT_H)
#define BLIT_H
/**
 * @file   blit.h
 * @brief  Functions for blitting images.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "raylib.h"
#include "raymath.h"

Vector2 get_screen();

Vector2 rect_pos( Rectangle& rect );
Vector2 rect_size( Rectangle& rect );

void rect_set_pos( Rectangle& rect, Vector2 pos );
void rect_set_size( Rectangle& rect, Vector2 size );
Vector2 texture_size( Texture& texture );
Vector2 fit_to_screen( Vector2 dst, Vector2 src );

Rectangle centered_fit_to_screen( Vector2 dst_size, Vector2 src_size );

#endif /* header guard */
