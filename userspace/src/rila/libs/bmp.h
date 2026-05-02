#ifndef RILA_BMP_H
#define RILA_BMP_H

#include <graphics.h>
#include <stdint.h>

typedef enum
{
    RILA_BMP_OK = 0,
    RILA_BMP_ERR_MAGIC,
    RILA_BMP_ERR_HEADER,
    RILA_BMP_ERR_COMPRESSION,
    RILA_BMP_ERR_BPP
} rila_bmp_result_t;

rila_bmp_result_t rila_bmp_draw(const uint8_t *bmp, uint32_t size, uint32_t x, uint32_t y, graphics_context_t *ctx);

#define MASK_RED 0x00ff0000
#define MASK_GREEN 0x0000ff00
#define MASK_BLUE 0x000000ff
#define MASK_ALPHA 0
#endif
