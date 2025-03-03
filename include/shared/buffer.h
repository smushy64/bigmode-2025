#if !defined(SHARED_BUFFER_H)
#define SHARED_BUFFER_H
/**
 * @file   buffer.h
 * @brief  Simple push/pop buffer.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 28, 2025
*/

#define buf_append( _buf, item ) do { \
    if( !(_buf)->buf || !( (_buf)->cap - (_buf)->len ) ) { \
        int new_count; \
        if( !(_buf)->len ) { \
            new_count = 2; \
        } else { \
            new_count = (_buf)->len * 2; \
        } \
        (_buf)->buf = (__typeof__((_buf)->buf))realloc( (_buf)->buf, sizeof( (_buf)->buf[0] ) * new_count ); \
        (_buf)->cap = new_count; \
    } \
    (_buf)->buf[(_buf)->len++] = (item); \
} while(0)

#define buf_remove( _buf, index ) do { \
    int stride = sizeof((_buf)->buf[0]); \
    memmove( \
        (_buf)->buf + index, \
        (_buf)->buf + index + 1, \
        stride * ((_buf)->len - index) ); \
    (_buf)->len--; \
} while(0)

#define buf_swap_remove( _buf, index ) do { \
    (_buf)->len--; \
    (_buf)->buf[index] = (_buf)->buf[(_buf)->len]; \
} while(0)

#endif /* header guard */
