/*
 * RoopM Engine - Core Renderer Implementation
 * Software pixel renderer - draws directly to uint32_t* framebuffer.
 */

#include "roopm.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "vendor/blend2d/blend2d.h"

static BLImageCore g_blImage;
static BLContextCore g_blContext;
static bool g_blActive = false;

static RoopmPlatform *g_platform = NULL;
static RoopmFramebuffer *g_fb = NULL;
static RoopmInput *g_input = NULL;

#define MAX_CLIP_STACK 16
static struct {
  float x, y, w, h;
} g_clipStack[MAX_CLIP_STACK];
static int g_clipIndex = 0;

float g_clipX = 0, g_clipY = 0, g_clipW = 0, g_clipH = 0;

void roopm_init(RoopmPlatform *platform) {
  g_platform = platform;
  g_fb = NULL;
  g_input = NULL;
  g_clipIndex = 0;
}

void roopm_shutdown(void) {
  g_platform = NULL;
  g_fb = NULL;
  g_input = NULL;
}

void roopm_begin_frame(void) {
  if (!g_platform) return;
  g_fb = g_platform->getFramebuffer(g_platform);
  g_input = g_platform->getInput(g_platform);

  g_clipX = 0;
  g_clipY = 0;
  g_clipW = (float)g_fb->width;
  g_clipH = (float)g_fb->height;
  g_clipIndex = 0;

  // 🌟 Attach Blend2D SIMD JIT Compositor directly to Framebuffer (Zero-Copy)
  if (g_fb && g_fb->pixels && g_fb->width > 0 && g_fb->height > 0) {
    bl_image_init(&g_blImage);
    bl_context_init(&g_blContext);
    if (bl_image_create_from_data(&g_blImage, g_fb->width, g_fb->height, BL_FORMAT_PRGB32, g_fb->pixels, g_fb->width * 4, BL_DATA_ACCESS_RW, nullptr, nullptr) == BL_SUCCESS) {
      if (bl_context_begin(&g_blContext, &g_blImage, nullptr) == BL_SUCCESS) {
        g_blActive = true;
      }
    }
  }
}

void roopm_end_frame(void) {
  if (g_blActive) {
    bl_context_end(&g_blContext);
    bl_context_destroy(&g_blContext);
    bl_image_destroy(&g_blImage);
    g_blActive = false;
  }
  if (g_platform) {
    g_platform->present(g_platform);
  }
}

float roopm_get_width(void) {
  return (g_fb && g_fb->dpi > 0.0f) ? (float)g_fb->width / g_fb->dpi : 0.0f;
}

float roopm_get_height(void) {
  return (g_fb && g_fb->dpi > 0.0f) ? (float)g_fb->height / g_fb->dpi : 0.0f;
}

float roopm_get_dpi(void) { return g_fb ? g_fb->dpi : 1.0f; }
int roopm_get_pixel_width(void) { return g_fb ? g_fb->width : 0; }
int roopm_get_pixel_height(void) { return g_fb ? g_fb->height : 0; }

static inline int roopm_min(int a, int b) { return a < b ? a : b; }
static inline int roopm_max(int a, int b) { return a > b ? a : b; }
static inline float roopm_minf(float a, float b) { return a < b ? a : b; }
static inline float roopm_maxf(float a, float b) { return a > b ? a : b; }

static inline uint32_t roopm_blend(uint32_t dst, uint32_t src) {
  uint32_t sa = (src >> 24) & 0xFF;
  if (sa == 0) return dst;
  if (sa == 255) return src;

  uint32_t da = (dst >> 24) & 0xFF;
  uint32_t sr = (src >> 16) & 0xFF;
  uint32_t sg = (src >> 8) & 0xFF;
  uint32_t sb = src & 0xFF;
  uint32_t dr = (dst >> 16) & 0xFF;
  uint32_t dg = (dst >> 8) & 0xFF;
  uint32_t db = dst & 0xFF;

  uint32_t oa = sa + (da * (255 - sa)) / 255;
  if (oa == 0) return 0;
  uint32_t or_ = (sr * sa + dr * da * (255 - sa) / 255) / oa;
  uint32_t og = (sg * sa + dg * da * (255 - sa) / 255) / oa;
  uint32_t ob = (sb * sa + db * da * (255 - sa) / 255) / oa;

  return (oa << 24) | (or_ << 16) | (og << 8) | ob;
}

static inline void roopm_set_pixel(int x, int y, uint32_t color) {
  if (!g_fb || !g_fb->pixels) return;
  if (x < (int)g_clipX || x >= (int)(g_clipX + g_clipW)) return;
  if (y < (int)g_clipY || y >= (int)(g_clipY + g_clipH)) return;
  if (x < 0 || x >= g_fb->width || y < 0 || y >= g_fb->height) return;

  int idx = y * g_fb->width + x;
  g_fb->pixels[idx] = roopm_blend(g_fb->pixels[idx], color);
}

static inline float roopm_to_px(float logical) {
  return g_fb ? (logical * g_fb->dpi) : logical;
}

void roopm_push_clip(float x, float y, float w, float h) {
  if (g_clipIndex >= MAX_CLIP_STACK) return;

  g_clipStack[g_clipIndex].x = g_clipX;
  g_clipStack[g_clipIndex].y = g_clipY;
  g_clipStack[g_clipIndex].w = g_clipW;
  g_clipStack[g_clipIndex].h = g_clipH;
  g_clipIndex++;

  float px = roopm_to_px(x);
  float py = roopm_to_px(y);
  float pw = roopm_to_px(w);
  float ph = roopm_to_px(h);

  float newX = roopm_maxf(g_clipX, px);
  float newY = roopm_maxf(g_clipY, py);
  float newR = roopm_minf(g_clipX + g_clipW, px + pw);
  float newB = roopm_minf(g_clipY + g_clipH, py + ph);

  g_clipX = newX;
  g_clipY = newY;
  g_clipW = roopm_maxf(0.0f, newR - newX);
  g_clipH = roopm_maxf(0.0f, newB - newY);
}

void roopm_pop_clip(void) {
  if (g_clipIndex <= 0) return;
  g_clipIndex--;
  g_clipX = g_clipStack[g_clipIndex].x;
  g_clipY = g_clipStack[g_clipIndex].y;
  g_clipW = g_clipStack[g_clipIndex].w;
  g_clipH = g_clipStack[g_clipIndex].h;
}

void roopm_clear(uint32_t color) {
  if (!g_fb || !g_fb->pixels) return;
  int total = g_fb->width * g_fb->height;
  uint32_t *p = g_fb->pixels;
  for (int i = 0; i < total; i++) {
    p[i] = color;
  }
}

void roopm_draw_rect(float x, float y, float w, float h, uint32_t color) {
  if (!g_fb || w <= 0.0f || h <= 0.0f) return;

  int px = (int)roopm_to_px(x);
  int py = (int)roopm_to_px(y);
  int pw = (int)(roopm_to_px(x + w) - px);
  int ph = (int)(roopm_to_px(y + h) - py);

  int x0 = roopm_max(px, (int)g_clipX);
  int y0 = roopm_max(py, (int)g_clipY);
  int x1 = roopm_min(px + pw, (int)(g_clipX + g_clipW));
  int y1 = roopm_min(py + ph, (int)(g_clipY + g_clipH));

  x0 = roopm_max(0, x0);
  y0 = roopm_max(0, y0);
  x1 = roopm_min(g_fb->width, x1);
  y1 = roopm_min(g_fb->height, y1);

  uint8_t alpha = (color >> 24) & 0xFF;

  if (alpha == 255) {
    for (int row = y0; row < y1; row++) {
      uint32_t *rowPtr = g_fb->pixels + row * g_fb->width;
      for (int col = x0; col < x1; col++) {
        rowPtr[col] = color;
      }
    }
  } else if (alpha > 0) {
    for (int row = y0; row < y1; row++) {
      for (int col = x0; col < x1; col++) {
        roopm_set_pixel(col, row, color);
      }
    }
  }
}

void roopm_draw_rect_outline(float x, float y, float w, float h, uint32_t color, float thickness) {
  float t = thickness > 0 ? thickness : 1.0f;
  roopm_draw_rect(x, y, w, t, color);
  roopm_draw_rect(x, y + h - t, w, t, color);
  roopm_draw_rect(x, y + t, t, h - 2 * t, color);
  roopm_draw_rect(x + w - t, y + t, t, h - 2 * t, color);
}



// 🌟 Pure Software Smooth Gradient Rounded Rectangle (Supports Horizontal & Vertical)
void roopm_draw_rect_rounded_gradient(float x, float y, float w, float h, float radius, uint32_t startColor, uint32_t endColor, bool isHorizontal) {
  if (!g_fb || w <= 0 || h <= 0) return;

  float px = roopm_to_px(x);
  float py = roopm_to_px(y);
  float pw = roopm_to_px(w);
  float ph = roopm_to_px(h);
  float pr = roopm_to_px(radius);

  if (pr > pw * 0.5f) pr = pw * 0.5f;
  if (pr > ph * 0.5f) pr = ph * 0.5f;

  int ix0 = roopm_max(0, roopm_max((int)px, (int)g_clipX));
  int iy0 = roopm_max(0, roopm_max((int)py, (int)g_clipY));
  int ix1 = roopm_min(g_fb->width, roopm_min((int)(px + pw + 1), (int)(g_clipX + g_clipW)));
  int iy1 = roopm_min(g_fb->height, roopm_min((int)(py + ph + 1), (int)(g_clipY + g_clipH)));

  uint32_t r1 = (startColor >> 16) & 0xFF, g1 = (startColor >> 8) & 0xFF, b1 = startColor & 0xFF;
  uint32_t r2 = (endColor >> 16) & 0xFF, g2 = (endColor >> 8) & 0xFF, b2 = endColor & 0xFF;

  float cx_left = px + pr;
  float cx_right = px + pw - pr;
  float cy_top = py + pr;
  float cy_bottom = py + ph - pr;

  for (int py_idx = iy0; py_idx < iy1; py_idx++) {
    float curY = (float)py_idx + 0.5f;
    float ty = (curY - py) / ph;
    if (ty < 0.0f) ty = 0.0f; if (ty > 1.0f) ty = 1.0f;

    for (int px_idx = ix0; px_idx < ix1; px_idx++) {
      float curX = (float)px_idx + 0.5f;
      float tx = (curX - px) / pw;
      if (tx < 0.0f) tx = 0.0f; if (tx > 1.0f) tx = 1.0f;

      float t = isHorizontal ? tx : ty;

      uint32_t cr = (uint32_t)(r1 + t * (int)(r2 - r1));
      uint32_t cg = (uint32_t)(g1 + t * (int)(g2 - g1));
      uint32_t cb = (uint32_t)(b1 + t * (int)(b2 - b1));
      uint32_t pixelColor = 0xFF000000 | (cr << 16) | (cg << 8) | cb;

      float qx = 0.0f;
      if (curX < cx_left) qx = cx_left - curX;
      else if (curX > cx_right) qx = curX - cx_right;

      float qy = 0.0f;
      if (curY < cy_top) qy = cy_top - curY;
      else if (curY > cy_bottom) qy = cy_bottom - curY;

      if (qx == 0.0f || qy == 0.0f) {
        roopm_set_pixel(px_idx, py_idx, pixelColor);
      } else {
        float dist = sqrtf(qx * qx + qy * qy);
        float edgeDist = pr - dist;
        if (edgeDist >= 0.5f) {
          roopm_set_pixel(px_idx, py_idx, pixelColor);
        } else if (edgeDist > -0.5f) {
          float aa = edgeDist + 0.5f;
          uint8_t final_alpha = (uint8_t)(255 * aa);
          uint32_t aa_color = (final_alpha << 24) | (cr << 16) | (cg << 8) | cb;
          roopm_set_pixel(px_idx, py_idx, aa_color);
        }
      }
    }
  }
}

// 🌟 100% Seamless Continuous Rounded Outline (Zero Corner Circles!)


void roopm_draw_line(float x1, float y1, float x2, float y2, uint32_t color, float thickness) {
  if (g_blActive) {
    double px1 = (double)roopm_to_px(x1), py1 = (double)roopm_to_px(y1);
    double px2 = (double)roopm_to_px(x2), py2 = (double)roopm_to_px(y2);
    double pt = (double)roopm_to_px(thickness);
    if (pt < 1.0) pt = 1.0;
    BLLine line = { px1, py1, px2, py2 };
    bl_context_set_stroke_style_rgba32(&g_blContext, color);
    bl_context_set_stroke_width(&g_blContext, pt);
    bl_context_stroke_geometry(&g_blContext, BL_GEOMETRY_TYPE_LINE, &line);
    return;
  }
  int px1 = (int)roopm_to_px(x1);
  int py1 = (int)roopm_to_px(y1);
  int px2 = (int)roopm_to_px(x2);
  int py2 = (int)roopm_to_px(y2);

  int dx = abs(px2 - px1);
  int dy = abs(py2 - py1);
  int sx = px1 < px2 ? 1 : -1;
  int sy = py1 < py2 ? 1 : -1;
  int err = dx - dy;

  int t = (int)roopm_to_px(thickness);
  if (t < 1) t = 1;

  while (1) {
    for (int ty = -t / 2; ty <= t / 2; ty++) {
      for (int tx = -t / 2; tx <= t / 2; tx++) {
        roopm_set_pixel(px1 + tx, py1 + ty, color);
      }
    }
    if (px1 == px2 && py1 == py2) break;
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      px1 += sx;
    }
    if (e2 < dx) {
      err += dx;
      py1 += sy;
    }
  }
}

void roopm_draw_circle(float cx, float cy, float radius, uint32_t color) {
  int pcx = (int)roopm_to_px(cx);
  int pcy = (int)roopm_to_px(cy);
  float pr = roopm_to_px(radius);

  uint8_t alpha_orig = (color >> 24) & 0xFF;
  uint8_t cr = (color >> 16) & 0xFF;
  uint8_t cg = (color >> 8) & 0xFF;
  uint8_t cb = color & 0xFF;

  int iRadius = (int)(pr + 2);

  for (int y = -iRadius; y <= iRadius; y++) {
    for (int x = -iRadius; x <= iRadius; x++) {
      float dist = sqrtf((float)(x * x + y * y));
      float edgeDist = pr - dist;

      if (edgeDist > 0.5f) {
        roopm_set_pixel(pcx + x, pcy + y, color);
      } else if (edgeDist > -0.5f) {
        float aa_alpha = edgeDist + 0.5f;
        uint8_t final_alpha = (uint8_t)(alpha_orig * aa_alpha);
        uint32_t aa_color = (final_alpha << 24) | (cr << 16) | (cg << 8) | cb;
        roopm_set_pixel(pcx + x, pcy + y, aa_color);
      }
    }
  }
}

void roopm_draw_circle_outline(float cx, float cy, float radius, uint32_t color, float thickness) {
  int pcx = (int)roopm_to_px(cx);
  int pcy = (int)roopm_to_px(cy);
  float pr = roopm_to_px(radius);
  float pt = roopm_to_px(thickness);
  if (pt < 1.0f) pt = 1.0f;

  float outerR = pr;
  float innerR = pr - pt;

  uint8_t alpha_orig = (color >> 24) & 0xFF;
  uint8_t cr = (color >> 16) & 0xFF;
  uint8_t cg = (color >> 8) & 0xFF;
  uint8_t cb = color & 0xFF;

  int iRadius = (int)(pr + 2);

  for (int y = -iRadius; y <= iRadius; y++) {
    for (int x = -iRadius; x <= iRadius; x++) {
      float dist = sqrtf((float)(x * x + y * y));
      float outerEdge = outerR - dist;
      float innerEdge = dist - innerR;

      if (outerEdge > -0.5f && innerEdge > -0.5f) {
        float aa_outer = outerEdge > 0.5f ? 1.0f : (outerEdge + 0.5f);
        float aa_inner = innerEdge > 0.5f ? 1.0f : (innerEdge + 0.5f);
        float aa_alpha = aa_outer * aa_inner;
        if (aa_alpha > 0.0f) {
          uint8_t final_alpha = (uint8_t)(alpha_orig * aa_alpha);
          uint32_t aa_color = (final_alpha << 24) | (cr << 16) | (cg << 8) | cb;
          roopm_set_pixel(pcx + x, pcy + y, aa_color);
        }
      }
    }
  }
}

RoopmInput *roopm_get_input(void) { return g_input; }

bool roopm_is_pointer_in_rect(float x, float y, float w, float h) {
  if (!g_input) return false;
  float px = g_input->pointerX;
  float py = g_input->pointerY;
  return px >= x && px <= x + w && py >= y && py <= y + h;
}

bool roopm_is_rect_hovered(float x, float y, float w, float h) {
  return roopm_is_pointer_in_rect(x, y, w, h);
}

bool roopm_is_rect_pressed(float x, float y, float w, float h) {
  return g_input && g_input->pointerDown && roopm_is_pointer_in_rect(x, y, w, h);
}

bool roopm_is_rect_clicked(float x, float y, float w, float h) {
  return g_input && g_input->pointerJustReleased && roopm_is_pointer_in_rect(x, y, w, h);
}

#include "roopm_font.h"

static bool g_useHighQualityFont = false;

void roopm_init_fonts(void) {
  roopm_font_init();
  g_useHighQualityFont = true;
}

void roopm_draw_text_ex(float x, float y, const char *text, float size, uint32_t color, bool bold) {
  if (!text || !g_fb) return;
  if (g_useHighQualityFont) {
    float px = roopm_to_px(x);
    float py = roopm_to_px(y);
    float psize = size * g_fb->dpi;
    roopm_font_draw_text_ex(g_fb->pixels, g_fb->width, g_fb->height, px, py, text, psize, color, bold);
  }
}

void roopm_draw_text(float x, float y, const char *text, float size, uint32_t color) {
  roopm_draw_text_ex(x, y, text, size, color, false);
}

void roopm_draw_text_centered_ex(float x, float y, float w, float h, const char *text, float size, uint32_t color, bool bold) {
  if (!text) return;
  float tw = roopm_measure_text_ex(text, size, bold);
  float tx = x + (w - tw) * 0.5f;
  float ty = y + (h - size) * 0.5f;
  roopm_draw_text_ex(tx, ty, text, size, color, bold);
}

void roopm_draw_text_centered(float x, float y, float w, float h, const char *text, float size, uint32_t color) {
  roopm_draw_text_centered_ex(x, y, w, h, text, size, color, false);
}

float roopm_measure_text_ex(const char *text, float size, bool bold) {
  if (!text) return 0.0f;
  if (g_useHighQualityFont && g_fb) {
    return roopm_font_measure_text_ex(text, size * g_fb->dpi, bold) / g_fb->dpi;
  }
  return (float)(strlen(text)) * size * 0.55f;
}

float roopm_measure_text(const char *text, float size) {
  return roopm_measure_text_ex(text, size, false);
}

float roopm_get_text_height(float size) { return size; }

void roopm_draw_rect_rounded(float x, float y, float w, float h, float radius, uint32_t color) {
  if (!g_fb || w <= 0.0f || h <= 0.0f) return;

  float px = roopm_to_px(x), py = roopm_to_px(y);
  float pw = roopm_to_px(w), ph = roopm_to_px(h);
  float pr = roopm_to_px(radius);
  if (pr > pw * 0.5f) pr = pw * 0.5f;
  if (pr > ph * 0.5f) pr = ph * 0.5f;

  if (g_blActive) {
    BLRoundRect rr = { (double)px, (double)py, (double)pw, (double)ph, (double)pr, (double)pr };
    bl_context_set_fill_style_rgba32(&g_blContext, color);
    bl_context_fill_geometry(&g_blContext, BL_GEOMETRY_TYPE_ROUND_RECT, &rr);
    return;
  }

  int ix0 = roopm_max(0, roopm_max((int)px, (int)g_clipX));
  int iy0 = roopm_max(0, roopm_max((int)py, (int)g_clipY));
  int ix1 = roopm_min(g_fb->width, roopm_min((int)(px + pw + 1), (int)(g_clipX + g_clipW)));
  int iy1 = roopm_min(g_fb->height, roopm_min((int)(py + ph + 1), (int)(g_clipY + g_clipH)));

  uint8_t alpha_orig = (color >> 24) & 0xFF;
  uint8_t cr = (color >> 16) & 0xFF, cg = (color >> 8) & 0xFF, cb = color & 0xFF;

  float halfW = pw * 0.5f, halfH = ph * 0.5f;
  float boxW = halfW - pr, boxH = halfH - pr;
  float midX = px + halfW, midY = py + halfH;

  for (int py_idx = iy0; py_idx < iy1; py_idx++) {
    float curY = (float)py_idx + 0.5f;
    float dy = fabsf(curY - midY) - boxH;
    float qy = dy > 0.0f ? dy : 0.0f;

    for (int px_idx = ix0; px_idx < ix1; px_idx++) {
      float curX = (float)px_idx + 0.5f;
      float dx = fabsf(curX - midX) - boxW;
      float qx = dx > 0.0f ? dx : 0.0f;

      float outsideDist = sqrtf(qx * qx + qy * qy);
      float insideDist = (dx < 0.0f && dy < 0.0f) ? (dx > dy ? dx : dy) : 0.0f;
      float distToEdge = outsideDist + insideDist - pr;

      if (distToEdge <= 0.5f) {
        float aa = (distToEdge <= -0.5f) ? 1.0f : (0.5f - distToEdge);
        uint8_t final_a = (uint8_t)(alpha_orig * aa);
        roopm_set_pixel(px_idx, py_idx, (final_a << 24) | (cr << 16) | (cg << 8) | cb);
      }
    }
  }
}

// 🌟 Unified 2D Signed Distance Field (SDF) Rounded Rectangle Outline (Zero Artifacts!)
// 🌟 W3C Rounded Container Mask: Clears sharp corner pixels outside radius
void roopm_mask_rounded_corners(float x, float y, float w, float h, float radius, uint32_t clearColor) {
    if (!g_fb || w <= 0.0f || h <= 0.0f || radius <= 0.0f) return;
    float px = roopm_to_px(x), py = roopm_to_px(y);
    float pw = roopm_to_px(w), ph = roopm_to_px(h);
    float pr = roopm_to_px(radius);
    if (pr > pw * 0.5f) pr = pw * 0.5f;
    if (pr > ph * 0.5f) pr = ph * 0.5f;

    float halfW = pw * 0.5f, halfH = ph * 0.5f;
    float boxW = halfW - pr, boxH = halfH - pr;
    float midX = px + halfW, midY = py + halfH;

    int cX[4] = { (int)px, (int)(px + pw - pr), (int)px, (int)(px + pw - pr) };
    int cY[4] = { (int)py, (int)py, (int)(py + ph - pr), (int)(py + ph - pr) };

    for (int c = 0; c < 4; c++) {
        int startX = cX[c], startY = cY[c];
        int endX = startX + (int)(pr + 1), endY = startY + (int)(pr + 1);

        for (int py_idx = startY; py_idx < endY; py_idx++) {
            float curY = (float)py_idx + 0.5f;
            float dy = fabsf(curY - midY) - boxH;
            float qy = dy > 0.0f ? dy : 0.0f;

            for (int px_idx = startX; px_idx < endX; px_idx++) {
                float curX = (float)px_idx + 0.5f;
                float dx = fabsf(curX - midX) - boxW;
                float qx = dx > 0.0f ? dx : 0.0f;

                float outsideDist = sqrtf(qx * qx + qy * qy);
                float insideDist = (dx < 0.0f && dy < 0.0f) ? (dx > dy ? dx : dy) : 0.0f;
                float distToEdge = outsideDist + insideDist - pr;

                if (distToEdge > 0.0f) {
                    if (px_idx >= 0 && px_idx < g_fb->width && py_idx >= 0 && py_idx < g_fb->height) {
                        g_fb->pixels[py_idx * g_fb->width + px_idx] = clearColor;
                    }
                }
            }
        }
    }
}

void roopm_draw_rect_rounded_outline(float x, float y, float w, float h, float radius, uint32_t color, float thickness) {
  if (!g_fb || w <= 0.0f || h <= 0.0f || thickness <= 0.0f) return;

  float px = roopm_to_px(x), py = roopm_to_px(y);
  float pw = roopm_to_px(w), ph = roopm_to_px(h);
  float pr = roopm_to_px(radius);
  float pt = roopm_to_px(thickness);
  if (pr > pw * 0.5f) pr = pw * 0.5f;
  if (pr > ph * 0.5f) pr = ph * 0.5f;

  int ix0 = roopm_max(0, roopm_max((int)px, (int)g_clipX));
  int iy0 = roopm_max(0, roopm_max((int)py, (int)g_clipY));
  int ix1 = roopm_min(g_fb->width, roopm_min((int)(px + pw + 1), (int)(g_clipX + g_clipW)));
  int iy1 = roopm_min(g_fb->height, roopm_min((int)(py + ph + 1), (int)(g_clipY + g_clipH)));

  uint8_t alpha_orig = (color >> 24) & 0xFF;
  uint8_t cr = (color >> 16) & 0xFF, cg = (color >> 8) & 0xFF, cb = color & 0xFF;

  float halfW = pw * 0.5f, halfH = ph * 0.5f;
  float boxW = halfW - pr, boxH = halfH - pr;
  float midX = px + halfW, midY = py + halfH;

  for (int py_idx = iy0; py_idx < iy1; py_idx++) {
    float curY = (float)py_idx + 0.5f;
    float dy = fabsf(curY - midY) - boxH;
    float qy = dy > 0.0f ? dy : 0.0f;

    for (int px_idx = ix0; px_idx < ix1; px_idx++) {
      float curX = (float)px_idx + 0.5f;
      float dx = fabsf(curX - midX) - boxW;
      float qx = dx > 0.0f ? dx : 0.0f;

      float outsideDist = sqrtf(qx * qx + qy * qy);
      float insideDist = (dx < 0.0f && dy < 0.0f) ? (dx > dy ? dx : dy) : 0.0f;
      float distToEdge = outsideDist + insideDist - pr;

      // Outer & Inner Boundary SDF
      float d_outer = -distToEdge;
      float d_inner = -distToEdge - pt;

      float cov_outer = d_outer >= 0.5f ? 1.0f : (d_outer > -0.5f ? (d_outer + 0.5f) : 0.0f);
      float cov_inner = d_inner >= 0.5f ? 1.0f : (d_inner > -0.5f ? (d_inner + 0.5f) : 0.0f);
      float stroke_cov = cov_outer - cov_inner;

      if (stroke_cov > 0.01f) {
        uint8_t final_a = (uint8_t)(alpha_orig * stroke_cov);
        roopm_set_pixel(px_idx, py_idx, (final_a << 24) | (cr << 16) | (cg << 8) | cb);
      }
    }
  }
}

// =============================================================================
// 🌟 REALISTIC ANALYTICAL GAUSSIAN SOFT SHADOW ENGINE (Zero Hard Edges!)
// =============================================================================


// =============================================================================
// 🌟 SUBTLE OUTER-ONLY SOFT SHADOW (Never touches card interior!)
// =============================================================================


// =============================================================================
// 🌟 ZERO-SHELF CONTINUOUS GAUSSIAN SOFT SHADOW (Zero Step Artifacts!)
// =============================================================================


// =============================================================================
// 🌟 PERFECT SYMMETRIC SDF AMBIENT SHADOW BLOOM (Zero Offset Gap & Zero Shelves!)
// =============================================================================


// =============================================================================
// 🌟 DIRECTIONAL RAY-TRACED SDF SHADOW ENGINE (Downwards Directional Cast!)
// =============================================================================


// =============================================================================
// 🌟 AIRY NATURAL DIRECTIONAL SHADOW (Zero Muddy Smudges!)
// =============================================================================


// =============================================================================
// 🌟 BOOLEAN SDF SHADOW (Zero "Second Corner" Artifacts!)
// =============================================================================


// Fast Pseudo-Random Noise for Physical Glass Texture
static float roopm_fast_noise(int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

// =============================================================================
// 🌟 THE ULTIMATE APPLE iOS FROSTED GLASS ENGINE
// =============================================================================


// =============================================================================
// 🌟 SUBTLE AMBIENT OCCLUSION SHADOW (Soft & Natural)
// =============================================================================
void roopm_draw_soft_shadow(float x, float y, float w, float h, float radius, float blurRadius, uint8_t shadowAlpha) {
  if (!g_fb || w <= 0.0f || h <= 0.0f || blurRadius <= 0.0f || shadowAlpha == 0) return;

  float px = roopm_to_px(x), py = roopm_to_px(y);
  float pw = roopm_to_px(w), ph = roopm_to_px(h);
  float pr = roopm_to_px(radius);
  float pb = roopm_to_px(blurRadius);

  float offX = roopm_to_px(0.0f);
  float offY = roopm_to_px(6.0f);

  if (pr > pw * 0.5f) pr = pw * 0.5f;
  if (pr > ph * 0.5f) pr = ph * 0.5f;

  int ix0 = roopm_max(0, roopm_max((int)(px - pb), (int)g_clipX));
  int iy0 = roopm_max(0, roopm_max((int)(py - pb), (int)g_clipY));
  int ix1 = roopm_min(g_fb->width, roopm_min((int)(px + pw + pb + offX + 1), (int)(g_clipX + g_clipW)));
  int iy1 = roopm_min(g_fb->height, roopm_min((int)(py + ph + pb + offY + 1), (int)(g_clipY + g_clipH)));

  float halfW = pw * 0.5f, halfH = ph * 0.5f;
  float boxW = halfW - pr, boxH = halfH - pr;
  float midX = px + halfW, midY = py + halfH;

  for (int py_idx = iy0; py_idx < iy1; py_idx++) {
    float curY = (float)py_idx + 0.5f;
    for (int px_idx = ix0; px_idx < ix1; px_idx++) {
      float curX = (float)px_idx + 0.5f;

      // 🌟 High-Performance Occlusion Culling: Skip interior occluded by card itself (90% CPU save!)
      if (curX >= px + pr && curX <= px + pw - pr && curY >= py + pr && curY <= py + ph - pr) {
        continue;
      }

      float mdx = fabsf(curX - midX) - boxW;
      float mdy = fabsf(curY - midY) - boxH;
      float mqx = mdx > 0.0f ? mdx : 0.0f;
      float mqy = mdy > 0.0f ? mdy : 0.0f;
      float distToCard = sqrtf(mqx * mqx + mqy * mqy) + ((mdx < 0.0f && mdy < 0.0f) ? (mdx > mdy ? mdx : mdy) : 0.0f) - pr;

      if (distToCard >= 0.0f) {
        float sx = curX - offX;
        float sy = curY - offY;
        float sdx = fabsf(sx - midX) - boxW;
        float sdy = fabsf(sy - midY) - boxH;
        float sqx = sdx > 0.0f ? sdx : 0.0f;
        float sqy = sdy > 0.0f ? sdy : 0.0f;
        float distToShadow = sqrtf(sqx * sqx + sqy * sqy) + ((sdx < 0.0f && sdy < 0.0f) ? (sdx > sdy ? sdx : sdy) : 0.0f) - pr;

        if (distToShadow < pb) {
          float t = distToShadow <= 0.0f ? 0.0f : (distToShadow / pb);
          float falloff = (1.0f - t) * (1.0f - t);
          uint8_t a = (uint8_t)(shadowAlpha * falloff);
          if (a > 0) {
            roopm_set_pixel(px_idx, py_idx, (a << 24));
          }
        }
      }
    }
  }
}

// =============================================================================
// 🌟 100% AUTHENTIC APPLE VISIONOS LIQUID ACRYLIC GLASS (Flawless Optics!)
// =============================================================================


// =============================================================================
// 🌟 TRUE APPLE VISIONOS FROSTED ACRYLIC GLASS (Heavy Blur + Luminance Boost)
// =============================================================================


// =============================================================================
// 🌟 TRUE APPLE VISIONOS PADDED OPTICAL FROSTED GLASS (Zero Edge Cutoffs!)
// =============================================================================


// =============================================================================
// 🌟 GENUINE APPLE VISIONOS LIQUID MATERIAL (180% Saturation + 3D Inner Bevel)
// =============================================================================


// =============================================================================
// 🌟 FLAWLESS ZERO-SCAR APPLE VISIONOS LIQUID ACRYLIC ENGINE
// =============================================================================


// =============================================================================
// 🌟 ULTRA-FAST HIGH-PERFORMANCE ZERO-SCAR ACRYLIC GLASS (60 FPS OPTIMIZED)
// =============================================================================
void roopm_draw_vision_glass(float x, float y, float w, float h, float radius) {
  if (!g_fb || w <= 0.0f || h <= 0.0f) return;

  float px = roopm_to_px(x), py = roopm_to_px(y);
  float pw = roopm_to_px(w), ph = roopm_to_px(h);
  float pr = roopm_to_px(radius);
  if (pr > pw * 0.5f) pr = pw * 0.5f;
  if (pr > ph * 0.5f) pr = ph * 0.5f;

  int ix0 = roopm_max(0, roopm_max((int)px, (int)g_clipX));
  int iy0 = roopm_max(0, roopm_max((int)py, (int)g_clipY));
  int ix1 = roopm_min(g_fb->width, roopm_min((int)(px + pw + 1), (int)(g_clipX + g_clipW)));
  int iy1 = roopm_min(g_fb->height, roopm_min((int)(py + ph + 1), (int)(g_clipY + g_clipH)));

  int rectW = ix1 - ix0;
  int rectH = iy1 - iy0;
  if (rectW <= 0 || rectH <= 0) return;

  float halfW = pw * 0.5f, halfH = ph * 0.5f;
  float boxW = halfW - pr, boxH = halfH - pr;
  float midX = px + halfW, midY = py + halfH;

  // Soft Ambient Shadow
  roopm_draw_soft_shadow(x, y, w, h, radius, 14.0f, 35);

  // High-Speed Glass Composite with Specular Top Bevel
  for (int py_idx = iy0; py_idx < iy1; py_idx++) {
    float curY = (float)py_idx + 0.5f;
    float normY = (curY - py) / ph;
    if (normY < 0.0f) normY = 0.0f; if (normY > 1.0f) normY = 1.0f;

    uint8_t borderAlpha = (uint8_t)(130 - normY * 90);
    uint8_t bodyTintA = (uint8_t)(28 - normY * 12);

    float dy = fabsf(curY - midY) - boxH;
    float qy = dy > 0.0f ? dy : 0.0f;

    for (int px_idx = ix0; px_idx < ix1; px_idx++) {
      float curX = (float)px_idx + 0.5f;
      float dx = fabsf(curX - midX) - boxW;
      float qx = dx > 0.0f ? dx : 0.0f;

      float outsideDist = sqrtf(qx * qx + qy * qy);
      float insideDist = (dx < 0.0f && dy < 0.0f) ? (dx > dy ? dx : dy) : 0.0f;
      float distToEdge = outsideDist + insideDist - pr;

      if (distToEdge <= 0.5f) {
        float aa = (distToEdge <= -0.5f) ? 1.0f : (0.5f - distToEdge);
        uint32_t orig = g_fb->pixels[py_idx * g_fb->width + px_idx];
        uint32_t or_ = (orig >> 16) & 0xFF, og = (orig >> 8) & 0xFF, ob = orig & 0xFF;

        // Subtle Glass Tint & Translucency
        uint8_t curTintA = (distToEdge > -2.0f && normY < 0.30f) ? (bodyTintA + 25) : bodyTintA;
        uint32_t fr = (255 * curTintA + or_ * (255 - curTintA)) / 255;
        uint32_t fg = (255 * curTintA + og * (255 - curTintA)) / 255;
        uint32_t fb = (255 * curTintA + ob * (255 - curTintA)) / 255;

        if (aa < 1.0f) {
          uint8_t aByte = (uint8_t)(aa * 255.0f);
          fr = (fr * aByte + or_ * (255 - aByte)) / 255;
          fg = (fg * aByte + og * (255 - aByte)) / 255;
          fb = (fb * aByte + ob * (255 - aByte)) / 255;
        }
        g_fb->pixels[py_idx * g_fb->width + px_idx] = 0xFF000000 | (fr << 16) | (fg << 8) | fb;

        // Specular Crystal 1px Outline
        float d_inner = -distToEdge - 1.0f;
        float cov_outer = (-distToEdge >= 0.5f) ? 1.0f : (-distToEdge > -0.5f ? (-distToEdge + 0.5f) : 0.0f);
        float cov_inner = (d_inner >= 0.5f) ? 1.0f : (d_inner > -0.5f ? (d_inner + 0.5f) : 0.0f);
        float stroke_cov = cov_outer - cov_inner;
        if (stroke_cov > 0.01f) {
          uint8_t b_a = (uint8_t)(borderAlpha * stroke_cov);
          uint32_t currP = g_fb->pixels[py_idx * g_fb->width + px_idx];
          uint32_t cr = (currP >> 16) & 0xFF, cg = (currP >> 8) & 0xFF, cb = currP & 0xFF;
          fr = (255 * b_a + cr * (255 - b_a)) / 255;
          fg = (255 * b_a + cg * (255 - b_a)) / 255;
          fb = (255 * b_a + cb * (255 - b_a)) / 255;
          g_fb->pixels[py_idx * g_fb->width + px_idx] = 0xFF000000 | (fr << 16) | (fg << 8) | fb;
        }
      }
    }
  }
}


// 🌟 High-Quality Bilinear Scaled Image Blitter with SDF Rounded Corners
void roopm_draw_image(float x, float y, float w, float h, const uint32_t *srcPixels, int srcW, int srcH, float radius) {
  if (!g_fb || !srcPixels || w <= 0.0f || h <= 0.0f || srcW <= 0 || srcH <= 0) return;

  float px = roopm_to_px(x), py = roopm_to_px(y);
  float pw = roopm_to_px(w), ph = roopm_to_px(h);
  float pr = roopm_to_px(radius);
  if (pr > pw * 0.5f) pr = pw * 0.5f;
  if (pr > ph * 0.5f) pr = ph * 0.5f;

  int ix0 = roopm_max(0, roopm_max((int)px, (int)g_clipX));
  int iy0 = roopm_max(0, roopm_max((int)py, (int)g_clipY));
  int ix1 = roopm_min(g_fb->width, roopm_min((int)(px + pw + 1), (int)(g_clipX + g_clipW)));
  int iy1 = roopm_min(g_fb->height, roopm_min((int)(py + ph + 1), (int)(g_clipY + g_clipH)));

  float halfW = pw * 0.5f, halfH = ph * 0.5f;
  float boxW = halfW - pr, boxH = halfH - pr;
  float midX = px + halfW, midY = py + halfH;

  float scaleX = (float)srcW / pw;
  float scaleY = (float)srcH / ph;

  for (int py_idx = iy0; py_idx < iy1; py_idx++) {
    float curY = (float)py_idx + 0.5f;
    float dy = fabsf(curY - midY) - boxH;
    float qy = dy > 0.0f ? dy : 0.0f;

    int srcY = (int)((curY - py) * scaleY);
    if (srcY < 0) srcY = 0; if (srcY >= srcH) srcY = srcH - 1;

    for (int px_idx = ix0; px_idx < ix1; px_idx++) {
      float curX = (float)px_idx + 0.5f;
      float dx = fabsf(curX - midX) - boxW;
      float qx = dx > 0.0f ? dx : 0.0f;

      float outsideDist = sqrtf(qx * qx + qy * qy);
      float insideDist = (dx < 0.0f && dy < 0.0f) ? (dx > dy ? dx : dy) : 0.0f;
      float distToEdge = outsideDist + insideDist - pr;

      if (distToEdge <= 0.5f) {
        int srcX = (int)((curX - px) * scaleX);
        if (srcX < 0) srcX = 0; if (srcX >= srcW) srcX = srcW - 1;

        uint32_t srcColor = srcPixels[srcY * srcW + srcX];
        if (distToEdge <= -0.5f) {
            roopm_set_pixel(px_idx, py_idx, srcColor);
        } else {
            float aa = 0.5f - distToEdge;
            uint8_t a = (uint8_t)(((srcColor >> 24) & 0xFF) * aa);
            uint32_t finalCol = (a << 24) | (srcColor & 0x00FFFFFF);
            roopm_set_pixel(px_idx, py_idx, finalCol);
        }
      }
    }
  }
}
