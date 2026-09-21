#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vendor/yoga/yoga/Yoga.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "core/stb_truetype.h"

int main() {
    printf("=== DIAGNOSTIC FOR TEST 09 & 13 ===\n");
    
    // 1. Yoga Aspect Ratio Check
    YGNodeRef root = YGNodeNew();
    YGNodeStyleSetWidth(root, 300.0f);
    YGNodeStyleSetHeight(root, 300.0f);
    YGNodeRef child = YGNodeNew();
    YGNodeStyleSetWidth(child, 160.0f);
    YGNodeStyleSetAspectRatio(child, 16.0f / 9.0f);
    YGNodeInsertChild(root, child, 0);
    YGNodeCalculateLayout(root, 300.0f, 300.0f, YGDirectionLTR);
    float ch_h = YGNodeLayoutGetHeight(child);
    printf("Yoga Aspect Ratio Child Height: %f (Expected: 90.0)\n", ch_h);
    YGNodeFreeRecursive(root);

    // 2. Font Auto-shrink Check
    const char *fontPaths[] = { "C:\\Windows\\Fonts\\segoeui.ttf", "C:\\Windows\\Fonts\\arial.ttf" };
    FILE *f = NULL;
    for (const char *path : fontPaths) { f = fopen(path, "rb"); if (f) break; }
    if (f) {
        fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
        unsigned char *buf = (unsigned char*)malloc(sz); fread(buf, 1, sz, f); fclose(f);
        stbtt_fontinfo font;
        stbtt_InitFont(&font, buf, stbtt_GetFontOffsetForIndex(buf, 0));

        float targetWidth = 100.0f, bestSize = 0.0f;
        for (float s = 32.0f; s >= 10.0f; s -= 2.0f) {
            float scale = stbtt_ScaleForPixelHeight(&font, s);
            float w = 0;
            for (const char *p = "RoopM Title"; *p; p++) {
                int adv, lsb; stbtt_GetCodepointHMetrics(&font, *p, &adv, &lsb);
                w += adv * scale;
            }
            printf("  Font Size %.1f -> Width: %f\n", s, w);
            if (w <= targetWidth && bestSize == 0.0f) { bestSize = s; }
        }
        printf("Fitted Best Size: %f\n", bestSize);
        free(buf);
    }
    return 0;
}
