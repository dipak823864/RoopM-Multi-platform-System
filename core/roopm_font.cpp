#define STB_TRUETYPE_IMPLEMENTATION
#include "roopm_font.h"
#include "stb_truetype.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern float g_clipX, g_clipY, g_clipW, g_clipH;

extern "C" {

static stbtt_fontinfo g_fontRegular;
static stbtt_fontinfo g_fontBold;
static unsigned char *g_fontDataReg = NULL;
static unsigned char *g_fontDataBold = NULL;
static bool g_fontLoaded = false;
static bool g_hasBoldFont = false;

static unsigned char* load_font_file(const char **paths) {
    for (int i = 0; paths[i] != NULL; i++) {
        FILE *f = fopen(paths[i], "rb");
        if (f) {
            fseek(f, 0, SEEK_END); long size = ftell(f); fseek(f, 0, SEEK_SET);
            unsigned char *buf = (unsigned char*)malloc(size);
            if (buf) fread(buf, 1, size, f);
            fclose(f);
            return buf;
        }
    }
    return NULL;
}

void roopm_font_init(void) {
    if (g_fontLoaded) return;

    const char *regPaths[] = {
        "C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf",
        "/system/fonts/Roboto-Regular.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", NULL
    };
    const char *boldPaths[] = {
        "C:/Windows/Fonts/segoeuib.ttf", "C:/Windows/Fonts/arialbd.ttf",
        "/system/fonts/Roboto-Bold.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", NULL
    };

    g_fontDataReg = load_font_file(regPaths);
    if (g_fontDataReg && stbtt_InitFont(&g_fontRegular, g_fontDataReg, stbtt_GetFontOffsetForIndex(g_fontDataReg, 0))) {
        g_fontLoaded = true;
    }

    g_fontDataBold = load_font_file(boldPaths);
    if (g_fontDataBold && stbtt_InitFont(&g_fontBold, g_fontDataBold, stbtt_GetFontOffsetForIndex(g_fontDataBold, 0))) {
        g_hasBoldFont = true;
    }
}

void roopm_font_shutdown(void) {
    if (g_fontDataReg) { free(g_fontDataReg); g_fontDataReg = NULL; }
    if (g_fontDataBold) { free(g_fontDataBold); g_fontDataBold = NULL; }
    g_fontLoaded = false;
    g_hasBoldFont = false;
}

void roopm_font_draw_text_ex(uint32_t *pixels, int fbWidth, int fbHeight, float x, float y, const char *text, float size, uint32_t color, bool bold) {
    if (!g_fontLoaded || !text || !pixels) return;

    stbtt_fontinfo *font = (bold && g_hasBoldFont) ? &g_fontBold : &g_fontRegular;
    float scale = stbtt_ScaleForPixelHeight(font, size);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);
    
    // 🌟 True W3C Cap-Height Optical Center Baseline Equation (Sub-pixel Optical Center)
    float capHeight = (float)ascent * 0.72f * scale;
    float baseline = y + (size + capHeight) * 0.5f;

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g_c = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    float penX = x;

    while (*text) {
        unsigned char c = (unsigned char)*text;
        if (c >= 32 && c < 128) {
            int advanceWidth, leftSideBearing;
            stbtt_GetCodepointHMetrics(font, c, &advanceWidth, &leftSideBearing);

            int x0, y0, x1, y1;
            stbtt_GetCodepointBitmapBox(font, c, scale, scale, &x0, &y0, &x1, &y1);

            int glyphW = x1 - x0, glyphH = y1 - y0;
            if (glyphW > 0 && glyphH > 0) {
                unsigned char *glyphBitmap = (unsigned char *)malloc(glyphW * glyphH);
                if (glyphBitmap) {
                    stbtt_MakeCodepointBitmap(font, glyphBitmap, glyphW, glyphH, glyphW, scale, scale, c);
                    int destX = (int)(penX + (float)x0);
                    int destY = (int)(baseline + y0 + 0.5f);

                    for (int gy = 0; gy < glyphH; gy++) {
                        int screenY = destY + gy;
                        if (screenY < (int)g_clipY || screenY >= (int)(g_clipY + g_clipH)) continue;
                        if (screenY < 0 || screenY >= fbHeight) continue;

                        for (int gx = 0; gx < glyphW; gx++) {
                            int screenX = destX + gx;
                            if (screenX < (int)g_clipX || screenX >= (int)(g_clipX + g_clipW)) continue;
                            if (screenX < 0 || screenX >= fbWidth) continue;

                            uint8_t alpha = glyphBitmap[gy * glyphW + gx];
                            if (alpha == 0) continue;

                            int idx = screenY * fbWidth + screenX;
                            uint32_t dst = pixels[idx];
                            uint8_t dstR = (dst >> 16) & 0xFF, dstG = (dst >> 8) & 0xFF, dstB = dst & 0xFF;

                            uint8_t outR = (r * alpha + dstR * (255 - alpha)) / 255;
                            uint8_t outG = (g_c * alpha + dstG * (255 - alpha)) / 255;
                            uint8_t outB = (b * alpha + dstB * (255 - alpha)) / 255;
                            pixels[idx] = 0xFF000000 | (outR << 16) | (outG << 8) | outB;
                        }
                    }
                    free(glyphBitmap);
                }
            }
            penX += advanceWidth * scale;
            // 🌟 TrueType Pair-Kerning Engine (Zero awkward gaps between letters!)
            if (text[1]) {
                int kern = stbtt_GetCodepointKernAdvance(font, c, (unsigned char)text[1]);
                penX += kern * scale;
            }
        }
        text++;
    }
}

void roopm_font_draw_text(uint32_t *pixels, int fbWidth, int fbHeight, float x, float y, const char *text, float size, uint32_t color) {
    roopm_font_draw_text_ex(pixels, fbWidth, fbHeight, x, y, text, size, color, false);
}

float roopm_font_measure_text_ex(const char *text, float size, bool bold) {
    if (!g_fontLoaded || !text) return 0;
    stbtt_fontinfo *font = (bold && g_hasBoldFont) ? &g_fontBold : &g_fontRegular;
    float scale = stbtt_ScaleForPixelHeight(font, size);
    float width = 0;
    while (*text) {
        unsigned char c = (unsigned char)*text;
        if (c >= 32 && c < 128) {
            int advanceWidth, leftSideBearing;
            stbtt_GetCodepointHMetrics(font, c, &advanceWidth, &leftSideBearing);
            width += advanceWidth * scale;
        }
        text++;
    }
    return width;
}

float roopm_font_measure_text(const char *text, float size) {
    return roopm_font_measure_text_ex(text, size, false);
}

float roopm_font_line_height(float size) {
    return size * 1.35f;
}

} // extern "C"
