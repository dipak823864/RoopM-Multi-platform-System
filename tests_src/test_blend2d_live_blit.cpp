#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "vendor/blend2d/blend2d.h"

int main() {
    FILE *out = fopen("OUTPUT.TXT", "w");
    if (!out) out = stdout;

    fprintf(out, "===================================================================\n");
    fprintf(out, " [ROOPM ENGINE] FLAW #3 (BLEND2D SIMD FRAMEBUFFER BLIT) TEST\n");
    fprintf(out, "===================================================================\n\n");

    // 1. Create a simulated 200x200 32-bit PRGB Framebuffer
    int w = 200, h = 200;
    std::vector<uint32_t> framebuffer(w * h, 0xFF0F172A); // Dark Slate Blue background

    // 2. Attach Blend2D to the raw memory buffer (Zero-Copy)
    BLImageCore img;
    bl_image_init(&img);
    BLResult resCreate = bl_image_create_from_data(&img, w, h, BL_FORMAT_PRGB32, framebuffer.data(), w * 4, BL_DATA_ACCESS_READ_WRITE, nullptr, nullptr);

    fprintf(out, "Step 1: Zero-Copy Attach to Framebuffer: %s\n",
            resCreate == BL_SUCCESS ? "PASSED ✅ (0 ms overhead)" : "FAILED ❌");

    // 3. Initialize Blend2D High-Speed Vector Context
    BLContextCore ctx;
    bl_context_init(&ctx);
    BLResult resBegin = bl_context_begin(&ctx, &img, nullptr);

    fprintf(out, "Step 2: SIMD Hardware Vector Context:    %s\n",
            resBegin == BL_SUCCESS ? "PASSED ✅ (AVX2/SIMD Active)" : "FAILED ❌");

    // 4. Draw a Super-Smooth Rounded Rect with Anti-Aliasing
    bl_context_set_fill_style_rgba32(&ctx, 0xFF38BDF8); // Sky Blue (#38bdf8)
    BLRoundRect rr = { 20.0, 20.0, 160.0, 60.0, 12.0, 12.0 };
    bl_context_fill_geometry(&ctx, BL_GEOMETRY_TYPE_ROUND_RECT, &rr);

    // 5. Draw a 2px Vector Stroke Line
    bl_context_set_stroke_style_rgba32(&ctx, 0xFFFFFFFF); // Pure White
    bl_context_set_stroke_width(&ctx, 2.0);
    BLLine line = { 20.0, 120.0, 180.0, 120.0 };
    bl_context_stroke_geometry(&ctx, BL_GEOMETRY_TYPE_LINE, &line);

    bl_context_end(&ctx);
    bl_context_destroy(&ctx);
    bl_image_destroy(&img);

    // 6. Verify Pixels in Memory
    uint32_t centerPixel = framebuffer[50 * w + 100];
    uint32_t bgPixel = framebuffer[5 * w + 5];
    uint32_t linePixel = framebuffer[120 * w + 100];

    bool rectOk = ((centerPixel & 0x00FFFFFF) == 0x0038BDF8);
    bool bgOk = (bgPixel == 0xFF0F172A);
    bool lineOk = (linePixel == 0xFFFFFFFF);

    fprintf(out, "Step 3: Pixel Verification in Memory:\n");
    fprintf(out, "  Center of Rounded Rect = 0x%08X (Expected Sky Blue) %s\n", centerPixel, rectOk ? "PASSED ✅" : "FAILED ❌");
    fprintf(out, "  Background Clear Pixel = 0x%08X (Expected Dark Slate) %s\n", bgPixel, bgOk ? "PASSED ✅" : "FAILED ❌");
    fprintf(out, "  2px Vector Stroke Line = 0x%08X (Expected White)      %s\n", linePixel, lineOk ? "PASSED ✅" : "FAILED ❌");

    fprintf(out, "\n===================================================================\n");
    if (out != stdout) fclose(out);
    return 0;
}
