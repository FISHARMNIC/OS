#include "bmp.h"

#define BMP_FILE_HEADER_SIZE 14
#define BMP_INFO_HEADER_MIN_SIZE 40
#define BMP_COMPRESSION_RGB 0
#define BMP_COMPRESSION_BITFIELDS 3

static uint16_t read_le16(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t read_le32(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

static int32_t read_le32s(const uint8_t *data)
{
    return (int32_t)read_le32(data);
}

static uint32_t rgb_to_fb_color(uint8_t r, uint8_t g, uint8_t b, uint8_t bpp)
{
    if (bpp == 16)
    {
        return (uint32_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
    }

    return COLOR_FORMAT_RGB(r, g, b);
}

static uint8_t fb_color_r(uint32_t color, uint8_t bpp)
{
    if (bpp == 16)
    {
        return (uint8_t)((((color >> 11) & 0x1f) * 255) / 31);
    }

    return (uint8_t)((color >> 16) & 0xff);
}

static uint8_t fb_color_g(uint32_t color, uint8_t bpp)
{
    if (bpp == 16)
    {
        return (uint8_t)((((color >> 5) & 0x3f) * 255) / 63);
    }

    return (uint8_t)((color >> 8) & 0xff);
}

static uint8_t fb_color_b(uint32_t color, uint8_t bpp)
{
    if (bpp == 16)
    {
        return (uint8_t)(((color & 0x1f) * 255) / 31);
    }

    return (uint8_t)(color & 0xff);
}

static uint8_t blend_channel(uint8_t src, uint8_t dst, uint8_t alpha)
{
    return (uint8_t)(((uint32_t)src * alpha + (uint32_t)dst * (255 - alpha)) / 255);
}

static uint32_t mask_shift(uint32_t mask)
{
    uint32_t shift = 0;

    while (mask != 0 && (mask & 1) == 0)
    {
        mask >>= 1;
        shift++;
    }

    return shift;
}

static uint32_t mask_bits(uint32_t mask)
{
    uint32_t bits = 0;

    while (mask != 0)
    {
        if ((mask & 1) != 0)
        {
            bits++;
        }

        mask >>= 1;
    }

    return bits;
}

static uint8_t extract_masked_channel(uint32_t raw, uint32_t mask)
{
    if (mask == 0)
    {
        return 255;
    }

    uint32_t shift = mask_shift(mask);
    uint32_t bits = mask_bits(mask);
    uint32_t value = (raw & mask) >> shift;
    uint32_t max = (1U << bits) - 1;

    return (uint8_t)((value * 255) / max);
}

static uint32_t read_fb_pixel(uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    framebuffer_t *fb = (framebuffer_t *)ctx->fb;
    uint8_t *row = (uint8_t *)(fb->addr + (y * fb->pitch));

    if (fb->bpp == 32)
    {
        return ((uint32_t *)row)[x];
    }

    if (fb->bpp == 16)
    {
        return ((uint16_t *)row)[x];
    }

    return 0;
}

static void draw_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha, graphics_context_t *ctx)
{
    framebuffer_t *fb = (framebuffer_t *)ctx->fb;

    if (alpha == 0 || x >= fb->width || y >= fb->height)
    {
        return;
    }

    if (alpha < 255)
    {
        uint32_t dst = read_fb_pixel(x, y, ctx);

        r = blend_channel(r, fb_color_r(dst, fb->bpp), alpha);
        g = blend_channel(g, fb_color_g(dst, fb->bpp), alpha);
        b = blend_channel(b, fb_color_b(dst, fb->bpp), alpha);
    }

    uint32_t color = rgb_to_fb_color(r, g, b, fb->bpp);
    uint8_t *row = (uint8_t *)(fb->addr + (y * fb->pitch));

    if (fb->bpp == 32)
    {
        ((uint32_t *)row)[x] = color;
    }
    else if (fb->bpp == 16)
    {
        ((uint16_t *)row)[x] = (uint16_t)color;
    }
}

rila_bmp_result_t rila_bmp_draw(const uint8_t *bmp, uint32_t size, uint32_t x, uint32_t y, graphics_context_t *ctx)
{
    if (size < BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_MIN_SIZE)
    {
        return RILA_BMP_ERR_HEADER;
    }

    if (bmp[0] != 'B' || bmp[1] != 'M')
    {
        return RILA_BMP_ERR_MAGIC;
    }

    uint32_t pixel_offset = read_le32(bmp + 10);
    uint32_t info_size = read_le32(bmp + 14);
    int32_t bmp_width = read_le32s(bmp + 18);
    int32_t bmp_height = read_le32s(bmp + 22);
    uint16_t planes = read_le16(bmp + 26);
    uint16_t bmp_bpp = read_le16(bmp + 28);
    uint32_t compression = read_le32(bmp + 30);

    if (info_size < BMP_INFO_HEADER_MIN_SIZE || bmp_width <= 0 || bmp_height == 0 || planes != 1)
    {
        return RILA_BMP_ERR_HEADER;
    }

    if (compression != BMP_COMPRESSION_RGB && compression != BMP_COMPRESSION_BITFIELDS)
    {
        return RILA_BMP_ERR_COMPRESSION;
    }

    if (bmp_bpp != 24 && bmp_bpp != 32)
    {
        return RILA_BMP_ERR_BPP;
    }

    if (compression == BMP_COMPRESSION_BITFIELDS && bmp_bpp != 32)
    {
        return RILA_BMP_ERR_BPP;
    }

    uint32_t red_mask = MASK_RED;
    uint32_t green_mask = MASK_GREEN;
    uint32_t blue_mask = MASK_BLUE;
    uint32_t alpha_mask = MASK_ALPHA;
    
    if (compression == BMP_COMPRESSION_BITFIELDS && info_size >= 56 && size >= BMP_FILE_HEADER_SIZE + 56)
    {
        red_mask = read_le32(bmp + 54);
        green_mask = read_le32(bmp + 58);
        blue_mask = read_le32(bmp + 62);
        alpha_mask = read_le32(bmp + 66);
    }

    uint32_t width = (uint32_t)bmp_width;
    uint32_t height = bmp_height < 0 ? (uint32_t)-bmp_height : (uint32_t)bmp_height;
    uint32_t bytes_per_pixel = bmp_bpp / 8;
    uint32_t row_size = ((width * bmp_bpp + 31) / 32) * 4;

    if (pixel_offset >= size || row_size == 0 || row_size > size - pixel_offset)
    {
        return RILA_BMP_ERR_HEADER;
    }

    for (uint32_t row = 0; row < height; row++)
    {
        uint32_t source_row = bmp_height < 0 ? row : height - 1 - row;
        uint32_t row_offset = pixel_offset + (source_row * row_size);

        if (row_offset >= size || row_size > size - row_offset)
        {
            return RILA_BMP_ERR_HEADER;
        }

        for (uint32_t col = 0; col < width; col++)
        {
            uint32_t pixel = row_offset + (col * bytes_per_pixel);

            if (pixel + bytes_per_pixel > size)
            {
                return RILA_BMP_ERR_HEADER;
            }

            uint32_t raw = read_le32(bmp + pixel);
            uint8_t b = bmp[pixel];
            uint8_t g = bmp[pixel + 1];
            uint8_t r = bmp[pixel + 2];
            uint8_t alpha = 255;

            if (compression == BMP_COMPRESSION_BITFIELDS)
            {
                r = extract_masked_channel(raw, red_mask);
                g = extract_masked_channel(raw, green_mask);
                b = extract_masked_channel(raw, blue_mask);
                alpha = extract_masked_channel(raw, alpha_mask);
            }

            draw_pixel(x + col, y + row, r, g, b, alpha, ctx);
        }
    }

    return RILA_BMP_OK;
}
