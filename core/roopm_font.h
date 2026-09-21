#ifndef ROOPM_FONT_H
#define ROOPM_FONT_H

#include "roopm_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

void roopm_font_init(void);
void roopm_font_shutdown(void);
void roopm_font_draw_text(uint32_t *pixels, int fbWidth, int fbHeight, float x, float y, const char *text, float size, uint32_t color);
void roopm_font_draw_text_ex(uint32_t *pixels, int fbWidth, int fbHeight, float x, float y, const char *text, float size, uint32_t color, bool bold);
float roopm_font_measure_text(const char *text, float size);
float roopm_font_measure_text_ex(const char *text, float size, bool bold);
float roopm_font_line_height(float size);

#ifdef __cplusplus
}
#endif

#endif // ROOPM_FONT_H
