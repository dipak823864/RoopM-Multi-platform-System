#ifndef ROOPM_H
#define ROOPM_H

#include "roopm_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// Lifecycle
void roopm_init(RoopmPlatform *platform);
void roopm_shutdown(void);
void roopm_begin_frame(void);
void roopm_end_frame(void);

// Drawing Primitives
void roopm_clear(uint32_t color);
void roopm_draw_rect(float x, float y, float w, float h, uint32_t color);
void roopm_draw_rect_outline(float x, float y, float w, float h, uint32_t color, float thickness);
void roopm_draw_rect_rounded(float x, float y, float w, float h, float radius, uint32_t color);
void roopm_mask_rounded_corners(float x, float y, float w, float h, float radius, uint32_t clearColor);
void roopm_draw_rect_rounded_outline(float x, float y, float w, float h, float radius, uint32_t color, float thickness);
void roopm_draw_rect_rounded_gradient(float x, float y, float w, float h, float radius, uint32_t startColor, uint32_t endColor, bool isHorizontal);
void roopm_draw_glass_panel(float x, float y, float w, float h, float radius, int blurRadius, uint32_t tintColor, uint32_t borderColor);
void roopm_draw_line(float x1, float y1, float x2, float y2, uint32_t color, float thickness);
void roopm_draw_circle(float cx, float cy, float radius, uint32_t color);
void roopm_draw_circle_outline(float cx, float cy, float radius, uint32_t color, float thickness);
void roopm_draw_soft_shadow(float x, float y, float w, float h, float radius, float blurRadius, uint8_t shadowAlpha);
void roopm_draw_vision_glass(float x, float y, float w, float h, float radius);
void roopm_draw_image(float x, float y, float w, float h, const uint32_t *srcPixels, int srcW, int srcH, float radius);

// Typography
void roopm_draw_text(float x, float y, const char *text, float size, uint32_t color);
void roopm_draw_text_ex(float x, float y, const char *text, float size, uint32_t color, bool bold);
void roopm_draw_text_centered(float x, float y, float w, float h, const char *text, float size, uint32_t color);
void roopm_draw_text_centered_ex(float x, float y, float w, float h, const char *text, float size, uint32_t color, bool bold);
float roopm_measure_text(const char *text, float size);
float roopm_measure_text_ex(const char *text, float size, bool bold);
float roopm_get_text_height(float size);
void roopm_init_fonts(void);

// Clipping
void roopm_push_clip(float x, float y, float w, float h);
void roopm_pop_clip(void);

// Input Helpers
RoopmInput *roopm_get_input(void);
bool roopm_is_pointer_in_rect(float x, float y, float w, float h);
bool roopm_is_rect_clicked(float x, float y, float w, float h);
bool roopm_is_rect_pressed(float x, float y, float w, float h);
bool roopm_is_rect_hovered(float x, float y, float w, float h);

// Screen Info
float roopm_get_width(void);
float roopm_get_height(void);
float roopm_get_dpi(void);
int roopm_get_pixel_width(void);
int roopm_get_pixel_height(void);

#ifdef __cplusplus
}
#endif

#endif // ROOPM_H
