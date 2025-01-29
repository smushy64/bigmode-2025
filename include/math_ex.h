#if !defined(MATH_EX_H)
#define MATH_EX_H
/**
 * @file   math_ex.h
 * @brief  Extra math functions and types.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/

inline
int Min( int v, int min ) {
    if( v < min ) {
        return v;
    } else {
        return min;
    }
}
inline
int Max( int v, int max ) {
    if( v > max ) {
        return v;
    } else {
        return max;
    }
}
inline
int Clamp( int v, int min, int max ) {
    if( v < min ) {
        return min;
    } else if( v > max ) {
        return max;
    }
    return v;
}

#endif /* header guard */
