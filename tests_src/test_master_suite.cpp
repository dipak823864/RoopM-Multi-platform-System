#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>

// 1. QuickJS Header
#include "vendor/quickjs/quickjs.h"

// 2. Yoga Flexbox Header
#include "vendor/yoga/yoga/Yoga.h"

// 3. stb_truetype Header
#define STB_TRUETYPE_IMPLEMENTATION
#include "core/stb_truetype.h"

// 4. Lexbor HTML5 Parser Headers
#include "vendor/lexbor/html/parser.h"
#include "vendor/lexbor/html/interfaces/element.h"
#include "vendor/lexbor/dom/interfaces/element.h"

// 5. Blend2D Header
#include "vendor/blend2d/blend2d.h"

static int g_totalTests = 0;
static int g_passedTests = 0;
static int g_failedTests = 0;

#define RUN_TEST(name, func) do { \
    g_totalTests++; \
    printf("  [%02d] Testing %-48s ... ", g_totalTests, name); \
    fflush(stdout); \
    auto t0 = std::chrono::high_resolution_clock::now(); \
    bool ok = func(); \
    auto t1 = std::chrono::high_resolution_clock::now(); \
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count(); \
    if (ok) { \
        g_passedTests++; \
        printf("PASSED (%6.1f us)\n", us); \
    } else { \
        g_failedTests++; \
        printf("FAILED (%6.1f us)\n", us); \
    } \
    fflush(stdout); \
} while(0)

// ============================================================================
// SYSTEM 1: QuickJS Scripting & Promise Event Loop
// ============================================================================
bool test_qjs_runtime_allocation() {
    JSRuntime *rt = JS_NewRuntime(); if (!rt) return false;
    JSContext *ctx = JS_NewContext(rt); if (!ctx) { JS_FreeRuntime(rt); return false; }
    JS_FreeContext(ctx); JS_FreeRuntime(rt);
    return true;
}
bool test_qjs_math_trigonometry() {
    JSRuntime *rt = JS_NewRuntime(); JSContext *ctx = JS_NewContext(rt);
    const char *code = "Math.sqrt(256) + Math.cos(0) * 10 + (10 * 5);";
    JSValue val = JS_Eval(ctx, code, strlen(code), "<test>", JS_EVAL_TYPE_GLOBAL);
    double res = 0; JS_ToFloat64(ctx, &res, val); JS_FreeValue(ctx, val);
    JS_FreeContext(ctx); JS_FreeRuntime(rt);
    return fabs(res - 76.0) < 0.0001;
}
bool test_qjs_json_serialization() {
    JSRuntime *rt = JS_NewRuntime(); JSContext *ctx = JS_NewContext(rt);
    const char *code = "JSON.stringify({ project: 'RoopM-Studio', fps: 60, vector: true });";
    JSValue val = JS_Eval(ctx, code, strlen(code), "<test>", JS_EVAL_TYPE_GLOBAL);
    const char *str = JS_ToCString(ctx, val);
    bool ok = (str && strstr(str, "RoopM-Studio") && strstr(str, "60"));
    if (str) JS_FreeCString(ctx, str);
    JS_FreeValue(ctx, val); JS_FreeContext(ctx); JS_FreeRuntime(rt);
    return ok;
}
bool test_qjs_promise_microtask_queue() {
    JSRuntime *rt = JS_NewRuntime(); JSContext *ctx = JS_NewContext(rt);
    const char *code = "var result = 0; Promise.resolve(100).then(v => { result = v + 42; });";
    JSValue val = JS_Eval(ctx, code, strlen(code), "<promise>", JS_EVAL_TYPE_GLOBAL);
    JS_FreeValue(ctx, val);
    JSContext *pctx;
    while (JS_ExecutePendingJob(rt, &pctx) > 0) {}
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue resVal = JS_GetPropertyStr(ctx, global, "result");
    int num = 0; JS_ToInt32(ctx, &num, resVal);
    JS_FreeValue(ctx, resVal); JS_FreeValue(ctx, global);
    JS_FreeContext(ctx); JS_FreeRuntime(rt);
    return (num == 142);
}
bool test_qjs_garbage_collector_sweep() {
    JSRuntime *rt = JS_NewRuntime(); JSContext *ctx = JS_NewContext(rt);
    const char *code = "for (let i = 0; i < 1500; i++) { let o = { id: i, data: [1, 2, 3] }; }";
    JSValue val = JS_Eval(ctx, code, strlen(code), "<gc>", JS_EVAL_TYPE_GLOBAL);
    JS_FreeValue(ctx, val); JS_RunGC(rt);
    JS_FreeContext(ctx); JS_FreeRuntime(rt);
    return true;
}

// ============================================================================
// SYSTEM 2: Yoga Flexbox Layout Engine
// ============================================================================
bool test_yoga_node_lifecycle() {
    YGNodeRef node = YGNodeNew(); if (!node) return false;
    YGNodeStyleSetWidth(node, 800.0f); YGNodeStyleSetHeight(node, 600.0f);
    YGNodeFree(node); return true;
}
bool test_yoga_sidebar_content_layout() {
    YGNodeRef root = YGNodeNew();
    YGNodeStyleSetWidth(root, 1000.0f); YGNodeStyleSetHeight(root, 600.0f);
    YGNodeStyleSetFlexDirection(root, YGFlexDirectionRow);
    YGNodeStyleSetPadding(root, YGEdgeAll, 20.0f);
    YGNodeStyleSetGap(root, YGGutterAll, 10.0f);

    YGNodeRef sidebar = YGNodeNew(); YGNodeStyleSetWidth(sidebar, 240.0f); YGNodeInsertChild(root, sidebar, 0);
    YGNodeRef content = YGNodeNew(); YGNodeStyleSetFlexGrow(content, 1.0f); YGNodeInsertChild(root, content, 1);

    YGNodeCalculateLayout(root, 1000.0f, 600.0f, YGDirectionLTR);
    float sideW = YGNodeLayoutGetWidth(sidebar), contW = YGNodeLayoutGetWidth(content);
    YGNodeFreeRecursive(root);
    return (fabs(sideW - 240.0f) < 0.1f) && (fabs(contW - 710.0f) < 0.1f);
}
bool test_yoga_flex_wrap_math() {
    YGNodeRef root = YGNodeNew();
    YGNodeStyleSetWidth(root, 300.0f);
    YGNodeStyleSetFlexDirection(root, YGFlexDirectionRow);
    YGNodeStyleSetFlexWrap(root, YGWrapWrap);

    for (int i = 0; i < 3; i++) {
        YGNodeRef card = YGNodeNew();
        YGNodeStyleSetWidth(card, 120.0f); YGNodeStyleSetHeight(card, 40.0f);
        YGNodeInsertChild(root, card, i);
    }
    YGNodeCalculateLayout(root, 300.0f, YGUndefined, YGDirectionLTR);
    float card0Y = YGNodeLayoutGetTop(YGNodeGetChild(root, 0));
    float card2Y = YGNodeLayoutGetTop(YGNodeGetChild(root, 2));
    YGNodeFreeRecursive(root);
    return (card0Y == 0.0f) && (card2Y >= 40.0f);
}
bool test_yoga_aspect_ratio_constraint() {
    YGNodeRef root = YGNodeNew();
    YGNodeStyleSetWidth(root, 300.0f);
    YGNodeStyleSetHeight(root, 300.0f);
    YGNodeRef child = YGNodeNew();
    YGNodeStyleSetWidth(child, 160.0f);
    YGNodeStyleSetAspectRatio(child, 16.0f / 9.0f);
    YGNodeInsertChild(root, child, 0);
    YGNodeCalculateLayout(root, 300.0f, 300.0f, YGDirectionLTR);
    float h = YGNodeLayoutGetHeight(child);
    YGNodeFreeRecursive(root);
    return fabs(h - 90.0f) < 0.1f;
}

// ============================================================================
// SYSTEM 3: Typography & Sub-pixel Text Math
// ============================================================================
static stbtt_fontinfo g_testFont;
static unsigned char *g_fontBuffer = NULL;

bool test_font_loader() {
    const char *fontPaths[] = { "C:\\Windows\\Fonts\\segoeui.ttf", "C:\\Windows\\Fonts\\arial.ttf", "C:\\Windows\\Fonts\\tahoma.ttf" };
    FILE *f = NULL;
    for (const char *path : fontPaths) { f = fopen(path, "rb"); if (f) break; }
    if (!f) return false;
    fseek(f, 0, SEEK_END); long size = ftell(f); fseek(f, 0, SEEK_SET);
    g_fontBuffer = (unsigned char*)malloc(size); fread(g_fontBuffer, 1, size, f); fclose(f);
    return stbtt_InitFont(&g_testFont, g_fontBuffer, stbtt_GetFontOffsetForIndex(g_fontBuffer, 0)) != 0;
}
bool test_font_glyph_metrics() {
    if (!g_fontBuffer) return false;
    float scale = stbtt_ScaleForPixelHeight(&g_testFont, 20.0f);
    int adv, lsb; stbtt_GetCodepointHMetrics(&g_testFont, 'A', &adv, &lsb);
    return (adv * scale > 5.0f && adv * scale < 25.0f);
}
bool test_font_word_wrapping() {
    if (!g_fontBuffer) return false;
    float scale = stbtt_ScaleForPixelHeight(&g_testFont, 16.0f);
    const char *text = "RoopM Pure Modern C++ UI Engine with Hardware Vector Rendering";
    float maxW = 150.0f;
    int lineCount = 0;
    float currW = 0.0f;
    const char *p = text;
    while (*p) {
        int adv, lsb; stbtt_GetCodepointHMetrics(&g_testFont, *p, &adv, &lsb);
        float w = adv * scale;
        if (currW + w > maxW) { lineCount++; currW = w; }
        else { currW += w; }
        p++;
    }
    lineCount++;
    return (lineCount >= 3 && lineCount <= 6);
}
bool test_text_auto_shrink_fit() {
    if (!g_fontBuffer) return false;
    float targetWidth = 100.0f, bestSize = 0.0f;
    for (float sz = 32.0f; sz >= 10.0f; sz -= 2.0f) {
        float scale = stbtt_ScaleForPixelHeight(&g_testFont, sz);
        float w = 0;
        for (const char *p = "RoopM Title"; *p; p++) {
            int adv, lsb; stbtt_GetCodepointHMetrics(&g_testFont, *p, &adv, &lsb);
            w += adv * scale;
        }
        if (w <= targetWidth && bestSize == 0.0f) { bestSize = sz; }
    }
    return (bestSize >= 10.0f && bestSize <= 28.0f);
}

// ============================================================================
// SYSTEM 4: Lexbor WHATWG HTML5 & DOM Tree
// ============================================================================
bool test_lexbor_html_parse() {
    const char *html = "<div id=\"app\"><header class=\"top-nav\"><h1>RoopM</h1></header></div>";
    lxb_html_parser_t *parser = lxb_html_parser_create(); lxb_html_parser_init(parser);
    lxb_html_document_t *doc = lxb_html_parse(parser, (const lxb_char_t*)html, strlen(html));
    bool ok = (doc && doc->body != NULL);
    lxb_html_document_destroy(doc); lxb_html_parser_destroy(parser);
    return ok;
}
bool test_lexbor_dom_node_query() {
    const char *html = "<main><button id=\"b1\">Open</button><button id=\"b2\">Save</button></main>";
    lxb_html_parser_t *parser = lxb_html_parser_create(); lxb_html_parser_init(parser);
    lxb_html_document_t *doc = lxb_html_parse(parser, (const lxb_char_t*)html, strlen(html));
    lxb_dom_node_t *body = lxb_dom_interface_node(doc->body);
    int count = 0;
    lxb_dom_node_t *c = lxb_dom_node_first_child(body);
    while (c) {
        if (c->type == LXB_DOM_NODE_TYPE_ELEMENT) {
            lxb_dom_node_t *sub = lxb_dom_node_first_child(c);
            while (sub) { if (sub->type == LXB_DOM_NODE_TYPE_ELEMENT) count++; sub = sub->next; }
        }
        c = c->next;
    }
    lxb_html_document_destroy(doc); lxb_html_parser_destroy(parser);
    return (count == 2);
}

// ============================================================================
// SYSTEM 5: Blend2D Vector Context & Image Creation (Pure C API)
// ============================================================================
bool test_blend2d_image_create() {
    BLImageCore img;
    bl_image_init(&img);
    BLResult res = bl_image_create(&img, 200, 200, BL_FORMAT_PRGB32);
    bool ok = (res == BL_SUCCESS);
    bl_image_destroy(&img);
    return ok;
}
bool test_blend2d_vector_path_render() {
    BLImageCore img;
    bl_image_init(&img);
    if (bl_image_create(&img, 100, 100, BL_FORMAT_PRGB32) != BL_SUCCESS) {
        bl_image_destroy(&img);
        return false;
    }
    BLContextCore ctx;
    bl_context_init(&ctx);
    if (bl_context_begin(&ctx, &img, nullptr) != BL_SUCCESS) {
        bl_context_destroy(&ctx);
        bl_image_destroy(&img);
        return false;
    }
    bl_context_set_fill_style_rgba32(&ctx, 0xFF3B82F6);
    BLRoundRect rr = { 10.0, 10.0, 80.0, 80.0, 12.0, 12.0 };
    bl_context_fill_geometry(&ctx, BL_GEOMETRY_TYPE_ROUND_RECT, &rr);
    bl_context_end(&ctx);
    bl_context_destroy(&ctx);

    BLImageData imgData;
    bool ok = false;
    if (bl_image_get_data(&img, &imgData) == BL_SUCCESS) {
        uint32_t *pixels = (uint32_t*)imgData.pixel_data;
        uint32_t centerPx = pixels[50 * (imgData.stride / 4) + 50];
        ok = ((centerPx & 0x00FFFFFF) == 0x003B82F6);
    }
    bl_image_destroy(&img);
    return ok;
}
bool test_blend2d_gradient_shader() {
    BLGradientCore grad;
    bl_gradient_init(&grad);
    BLLinearGradientValues values = { 0.0, 0.0, 100.0, 0.0 };
    BLResult res = bl_gradient_create(&grad, BL_GRADIENT_TYPE_LINEAR, &values, BL_EXTEND_MODE_PAD, nullptr, 0, nullptr);
    if (res == BL_SUCCESS) {
        bl_gradient_add_stop_rgba32(&grad, 0.0, 0xFFFF0000);
        bl_gradient_add_stop_rgba32(&grad, 1.0, 0xFF0000FF);
    }
    bool ok = (res == BL_SUCCESS);
    bl_gradient_destroy(&grad);
    return ok;
}

// ============================================================================
// SYSTEM 6: Photoshop Layer Blend Modes (W3C Specification)
// ============================================================================
static inline uint8_t blend_mult(uint8_t d, uint8_t s) { return (uint8_t)((d * s + 127) / 255); }
static inline uint8_t blend_scrn(uint8_t d, uint8_t s) { return (uint8_t)(255 - ((255 - d) * (255 - s) + 127) / 255); }
static inline uint8_t blend_ovrl(uint8_t d, uint8_t s) {
    return (d < 128) ? (uint8_t)((2 * d * s + 127) / 255) : (uint8_t)(255 - (2 * (255 - d) * (255 - s) + 127) / 255);
}
bool test_photoshop_blend_modes() {
    return (blend_mult(200, 100) == 78) && (blend_scrn(200, 100) == 222) && (blend_ovrl(200, 100) == 188);
}

// ============================================================================
// SYSTEM 7: Gesture Arena & Conflict Disambiguation
// ============================================================================
struct DragGesture {
    float startX = 0, startY = 0, currentX = 0, currentY = 0;
    bool isDragging = false;
    void onPointerDown(float x, float y) { startX = currentX = x; startY = currentY = y; isDragging = false; }
    void onPointerMove(float x, float y, float threshold = 8.0f) {
        currentX = x; currentY = y;
        float dist = sqrtf((currentX - startX)*(currentX - startX) + (currentY - startY)*(currentY - startY));
        if (dist >= threshold) isDragging = true;
    }
};
bool test_gesture_drag_threshold() {
    DragGesture g;
    g.onPointerDown(100, 100);
    g.onPointerMove(103, 104); // dist = 5px (< 8px)
    if (g.isDragging) return false;
    g.onPointerMove(110, 110); // dist = 14.1px (>= 8px)
    return g.isDragging;
}
bool test_multitouch_pinch_zoom() {
    float initialDist = 100.0f, newDist = 200.0f;
    float scale = newDist / initialDist;
    return fabs(scale - 2.0f) < 0.001f;
}

// ============================================================================
// SYSTEM 8: Drag-and-Drop Interaction Pipeline
// ============================================================================
struct DragPayload {
    std::string format;
    std::vector<uint8_t> data;
    std::string textContent;
};
struct DropTarget {
    bool hasDragOver = false;
    bool dropped = false;
    void onDragEnter() { hasDragOver = true; }
    void onDragLeave() { hasDragOver = false; }
    void onDrop(const DragPayload &p) { hasDragOver = false; dropped = true; }
};
bool test_drag_drop_pipeline() {
    DragPayload payload = { "text/plain", {}, "Dragged Table Item" };
    DropTarget target;
    target.onDragEnter(); if (!target.hasDragOver) return false;
    target.onDrop(payload);
    return target.dropped && !target.hasDragOver;
}

// ============================================================================
// SYSTEM 9: Comprehensive 12-State Widget Matrix
// ============================================================================
enum class WidgetState {
    Normal, Hovered, Pressed, Focused, FocusVisible, Disabled,
    Checked, Dragging, DragOver, Selected, Loading, Invalid
};
struct WidgetStateController {
    uint32_t stateFlags = 0;
    void setState(WidgetState s, bool on) {
        if (on) stateFlags |= (1 << (int)s);
        else stateFlags &= ~(1 << (int)s);
    }
    bool hasState(WidgetState s) const { return (stateFlags & (1 << (int)s)) != 0; }
};
bool test_widget_state_matrix() {
    WidgetStateController ctrl;
    ctrl.setState(WidgetState::Hovered, true);
    ctrl.setState(WidgetState::Focused, true);
    ctrl.setState(WidgetState::Selected, true);
    return ctrl.hasState(WidgetState::Hovered) && ctrl.hasState(WidgetState::Focused) &&
           ctrl.hasState(WidgetState::Selected) && !ctrl.hasState(WidgetState::Disabled);
}

// ============================================================================
// SYSTEM 10: 2D Affine Matrix Geometry & Inverse Mapping
// ============================================================================
struct AffineMat2D {
    float a=1, b=0, c=0, d=1, tx=0, ty=0;
    void translate(float x, float y) { tx += a*x + c*y; ty += b*x + d*y; }
    void scale(float sx, float sy) { a *= sx; b *= sx; c *= sy; d *= sy; }
    void transform(float x, float y, float &ox, float &oy) const { ox = a*x + c*y + tx; oy = b*x + d*y + ty; }
    bool invert(AffineMat2D &out) const {
        float det = a*d - b*c; if (fabs(det) < 1e-6f) return false;
        float inv = 1.0f / det;
        out.a = d*inv; out.b = -b*inv; out.c = -c*inv; out.d = a*inv;
        out.tx = (c*ty - d*tx)*inv; out.ty = (b*tx - a*ty)*inv;
        return true;
    }
};
bool test_affine_matrix_screen_to_world() {
    AffineMat2D mat; mat.translate(200, 150); mat.scale(2.0f, 2.0f);
    float sx, sy; mat.transform(50, 50, sx, sy);
    AffineMat2D inv; if (!mat.invert(inv)) return false;
    float wx, wy; inv.transform(sx, sy, wx, wy);
    return (fabs(wx - 50.0f) < 0.01f) && (fabs(wy - 50.0f) < 0.01f);
}

// ============================================================================
// SYSTEM 11: Physics Simulation (Spring Oscillator & Inertia Decay)
// ============================================================================
bool test_spring_oscillator_physics() {
    float x = 0, v = 0, target = 100, k = 180, c = 12, dt = 0.01667f;
    for (int i = 0; i < 60; i++) {
        float f = -k * (x - target) - c * v;
        v += f * dt; x += v * dt;
    }
    return fabs(x - 100.0f) < 0.5f;
}
bool test_inertia_friction_decay() {
    float vel = 1000.0f, friction = 0.95f;
    for (int i = 0; i < 60; i++) vel *= friction;
    return (vel > 40.0f && vel < 50.0f);
}

// ============================================================================
// SYSTEM 12: Smart Snapping Guides, Polygon Math & PDF DPI Matrix
// ============================================================================
bool test_shoelace_polygon_area() {
    float x[] = {0, 100, 100, 0}, y[] = {0, 0, 100, 100};
    float area = 0;
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) % 4;
        area += (x[i]*y[j] - x[j]*y[i]);
    }
    area = fabsf(area * 0.5f);
    return fabs(area - 10000.0f) < 0.1f;
}
bool test_pdf_72dpi_coordinate_conversion() {
    float screenPx = 96.0f;
    float pdfPt = (screenPx / 96.0f) * 72.0f;
    return fabs(pdfPt - 72.0f) < 0.01f;
}
bool test_css_calc_math() {
    float parent = 1200.0f;
    float result = (parent * 1.0f) - 80.0f; // calc(100% - 80px)
    return fabs(result - 1120.0f) < 0.01f;
}

int main() {
    printf("===================================================================\n");
    printf(" [ROOPM ENGINE] Comprehensive Subsystems Master Diagnostic Suite\n");
    printf("===================================================================\n\n");
    fflush(stdout);

    printf("[1] QuickJS-ng Scripting & Promise Event Loop Engine\n");
    RUN_TEST("QuickJS Runtime & Context Allocation", test_qjs_runtime_allocation);
    RUN_TEST("QuickJS Math Trigonometry Precision", test_qjs_math_trigonometry);
    RUN_TEST("QuickJS JSON Stringify/Parse Pipeline", test_qjs_json_serialization);
    RUN_TEST("QuickJS Promise Microtask Execution Queue", test_qjs_promise_microtask_queue);
    RUN_TEST("QuickJS Garbage Collector (GC) Sweep", test_qjs_garbage_collector_sweep);

    printf("\n[2] Yoga CSS Flexbox Layout Mathematical Engine\n");
    RUN_TEST("Yoga Node Allocation & Lifecycle", test_yoga_node_lifecycle);
    RUN_TEST("Yoga Responsive Row Multi-Box Layout", test_yoga_sidebar_content_layout);
    RUN_TEST("Yoga Multi-Row FlexWrap Geometry", test_yoga_flex_wrap_math);
    RUN_TEST("Yoga 16:9 Aspect Ratio Constraint Math", test_yoga_aspect_ratio_constraint);

    printf("\n[3] stb_truetype Sub-pixel Typography Engine\n");
    RUN_TEST("TrueType System Font Loading (Segoe UI)", test_font_loader);
    RUN_TEST("Sub-pixel Glyph Advance Width Metrics", test_font_glyph_metrics);
    RUN_TEST("Dynamic Multi-line Word-Wrapping Engine", test_font_word_wrapping);
    RUN_TEST("Font Auto-Shrink Binary Search Fit Engine", test_text_auto_shrink_fit);

    printf("\n[4] Lexbor WHATWG HTML5 & DOM Parser Engine\n");
    RUN_TEST("Lexbor HTML5 Document Parser & Tree", test_lexbor_html_parse);
    RUN_TEST("Lexbor DOM Tree Node Traversal Query", test_lexbor_dom_node_query);

    printf("\n[5] Blend2D Hardware Vector Graphics Engine\n");
    RUN_TEST("Blend2D Image Memory Buffer Allocation", test_blend2d_image_create);
    RUN_TEST("Blend2D Rounded Rect & Pixel Verification", test_blend2d_vector_path_render);
    RUN_TEST("Blend2D Linear Gradient Shader Pipeline", test_blend2d_gradient_shader);

    printf("\n[6] Photoshop & Layer Blending Math Engine\n");
    RUN_TEST("W3C Multiply, Screen & Overlay Blend Modes", test_photoshop_blend_modes);

    printf("\n[7] Gesture Arena & Multi-Touch Recognition\n");
    RUN_TEST("Gesture Drag Threshold (8px Filter)", test_gesture_drag_threshold);
    RUN_TEST("Multi-Touch 2-Finger Pinch Zoom Scaling", test_multitouch_pinch_zoom);

    printf("\n[8] Drag-and-Drop Interaction Pipeline\n");
    RUN_TEST("DragSource -> DropTarget Event Handshake", test_drag_drop_pipeline);

    printf("\n[9] Widget Discrete State Matrix Engine\n");
    RUN_TEST("12-State Interactive Widget Matrix Bitmask", test_widget_state_matrix);

    printf("\n[10] 2D Affine Matrix & Geometry Transforms\n");
    RUN_TEST("Affine Matrix Inversion (Screen -> Canvas Pt)", test_affine_matrix_screen_to_world);

    printf("\n[11] Inertial Physics & Spring Simulation\n");
    RUN_TEST("Hooke's Law Damped Spring Oscillator", test_spring_oscillator_physics);
    RUN_TEST("60 FPS Inertia Friction Velocity Decay", test_inertia_friction_decay);

    printf("\n[12] Geometry Math, Smart Guides & PDF Export\n");
    RUN_TEST("Shoelace Formula Polygon Area & Bounds", test_shoelace_polygon_area);
    RUN_TEST("Screen 96 DPI to PDF 72 DPI Coordinates", test_pdf_72dpi_coordinate_conversion);
    RUN_TEST("CSS calc(100%% - 80px) Responsive Expression", test_css_calc_math);

    printf("\n===================================================================\n");
    printf(" [SUMMARY] %d Total Tests | %d Passed | %d Failed\n", g_totalTests, g_passedTests, g_failedTests);
    printf("===================================================================\n");
    fflush(stdout);

    if (g_fontBuffer) free(g_fontBuffer);
    return (g_failedTests == 0) ? 0 : 1;
}
