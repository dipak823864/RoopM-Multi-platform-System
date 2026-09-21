#ifdef __ANDROID__
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
extern struct android_app *g_android_app_ptr;
#endif
/*
 * RoopM UI Engine - Universal Standalone Web Browser & 2D Vector Runtime
 * Powered by Lexbor HTML5 + Yoga Flexbox + QuickJS ES2020 + Blend2D + STB Pipeline
 */

#include "core/roopm.h"
#include "core/web/WebEngine.h"
#include <algorithm>
#include <cmath>

UIEngine::WebEngine g_webEngine;
static bool g_engineInitialized = false;

static float g_scrollY = 0.0f;
static float g_scrollVelocity = 0.0f;
static bool g_isDraggingScrollbar = false;
static float g_dragStartPointerY = 0.0f;
static float g_dragStartScrollY = 0.0f;

void app_init(void) {
    roopm_init_fonts();
    g_engineInitialized = g_webEngine.loadWorkspace("workspace");
    if (!g_engineInitialized) {
        printf("[WARN] Could not load workspace folder. Using fallback.\n");
    }
}

void app_update(void) {
    RoopmInput *input = roopm_get_input();
    float w = roopm_get_width();
    float h = roopm_get_height();
    float dt = input->deltaTime > 0 ? input->deltaTime : 0.016f;

    // 1. Clear Canvas Base (🌟 W3C Canvas Root Background Sync)
    uint32_t canvasBg = ROOPM_RGB(3, 7, 18);
    if (g_webEngine.rootNode && (g_webEngine.rootNode->style.backgroundColor >> 24) != 0) {
        canvasBg = g_webEngine.rootNode->style.backgroundColor;
    }
    roopm_clear(canvasBg);

    if (!g_engineInitialized) {
        roopm_draw_text(20.0f, 40.0f, "🚨 RoopM Engine: Failed to load workspace/index.html", 16.0f, 0xFFEF4444);
        roopm_draw_text(20.0f, 70.0f, "--- LIVE ON-SCREEN DIAGNOSTICS ---", 14.0f, 0xFFF59E0B);
        char diagBuf[256];
        snprintf(diagBuf, sizeof(diagBuf), "VFS Initialized: %s", g_webEngine.vfs ? "YES (Active)" : "NO (Null VFS!)");
        roopm_draw_text(20.0f, 95.0f, diagBuf, 13.0f, 0xFFFFFFFF);

#ifdef __ANDROID__
        extern struct android_app *g_android_app_ptr;
#endif
        if (g_webEngine.vfs) {
            bool hasWs = g_webEngine.vfs->exists("workspace/index.html");
            bool hasIdx = g_webEngine.vfs->exists("index.html");
            snprintf(diagBuf, sizeof(diagBuf), "VFS File Check: 'workspace/index.html'=%s | 'index.html'=%s", hasWs ? "YES" : "NO", hasIdx ? "YES" : "NO");
            roopm_draw_text(20.0f, 120.0f, diagBuf, 13.0f, 0xFF38BDF8);

#ifdef __ANDROID__
            AAssetManager *mgr = (g_android_app_ptr && g_android_app_ptr->activity) ? g_android_app_ptr->activity->assetManager : nullptr;
            snprintf(diagBuf, sizeof(diagBuf), "AAssetManager Pointer: %p", (void*)mgr);
            roopm_draw_text(20.0f, 145.0f, diagBuf, 13.0f, mgr ? 0xFF10B981 : 0xFFEF4444);

            if (mgr) {
                AAssetDir *rootD = AAssetManager_openDir(mgr, "");
                const char *fn = AAssetDir_getNextFileName(rootD);
                std::string rootFiles = "";
                while (fn) { rootFiles += std::string(fn) + " "; fn = AAssetDir_getNextFileName(rootD); }
                AAssetDir_close(rootD);

                AAssetDir *wsD = AAssetManager_openDir(mgr, "workspace");
                fn = AAssetDir_getNextFileName(wsD);
                std::string wsFiles = "";
                while (fn) { wsFiles += std::string(fn) + " "; fn = AAssetDir_getNextFileName(wsD); }
                AAssetDir_close(wsD);

                snprintf(diagBuf, sizeof(diagBuf), "Root Assets: [%s]", rootFiles.empty() ? "EMPTY" : rootFiles.c_str());
                roopm_draw_text(20.0f, 170.0f, diagBuf, 12.0f, 0xFFFCD34D);

                snprintf(diagBuf, sizeof(diagBuf), "'workspace' Assets: [%s]", wsFiles.empty() ? "EMPTY" : wsFiles.c_str());
                roopm_draw_text(20.0f, 195.0f, diagBuf, 12.0f, 0xFFFCD34D);
            }
#endif
        }
        return;
    }

    float docH = g_webEngine.documentHeight;
    float maxScroll = std::max(0.0f, docH - h + 60.0f);

    // 2. Interactive Scrollbar Dragging Math
    float thumbW = 7.0f;
    float thumbX = w - thumbW - 3.0f;
    float trackH = h - 16.0f;
    float thumbH = std::max(36.0f, trackH * (h / (docH > 0 ? docH : h)));
    float scrollPct = (maxScroll > 0.0f) ? std::clamp(-g_scrollY / maxScroll, 0.0f, 1.0f) : 0.0f;
    float thumbY = 8.0f + scrollPct * (trackH - thumbH);

    bool isOverScrollbar = (input->pointerX >= thumbX - 6.0f && input->pointerX <= w &&
                            input->pointerY >= thumbY && input->pointerY <= thumbY + thumbH);

    if (input->pointerJustPressed && isOverScrollbar && maxScroll > 0.0f) {
        g_isDraggingScrollbar = true;
        g_dragStartPointerY = input->pointerY;
        g_dragStartScrollY = g_scrollY;
    }

    if (g_isDraggingScrollbar) {
        if (input->pointerDown) {
            float dy = input->pointerY - g_dragStartPointerY;
            float scrollDeltaRatio = dy / (trackH - thumbH);
            g_scrollY = g_dragStartScrollY - scrollDeltaRatio * maxScroll;
            g_scrollVelocity = 0.0f;
        } else {
            g_isDraggingScrollbar = false;
        }
    }

    // 3. Smooth Kinetic Scrolling
    if (!g_isDraggingScrollbar && input->scrollDeltaY != 0.0f) {
        g_scrollVelocity += input->scrollDeltaY * 350.0f;
    }

    if (!g_isDraggingScrollbar) {
        g_scrollY += g_scrollVelocity * dt;
        g_scrollVelocity *= expf(-6.0f * dt);

        if (g_scrollY > 0.0f) {
            g_scrollY += (0.0f - g_scrollY) * (1.0f - expf(-18.0f * dt));
            g_scrollVelocity *= 0.5f;
        } else if (g_scrollY < -maxScroll) {
            g_scrollY += (-maxScroll - g_scrollY) * (1.0f - expf(-18.0f * dt));
            g_scrollVelocity *= 0.5f;
        }
    }

    // 4. Keyboard Navigation: Backspace = History Back, F5 = Live Reload
    if (!g_webEngine.focusedInputNode && input->keyJustPressed[8]) {
        g_webEngine.goBack();
        g_scrollY = 0.0f;
    } else if (input->keyJustPressed[116]) {
        g_webEngine.navigate(g_webEngine.currentDocumentPath);
        g_scrollY = 0.0f;
    } else {
        g_webEngine.handleKeyboardInput(input->lastChar, input->keyJustPressed[8]);
    }

    // 5. Update Web Engine Runtime (60 FPS LERP Transitions & Timers)
    g_webEngine.update(dt, input->pointerX, input->pointerY);

    // 6. User Click Dispatching (1:1 Exact Coordinates)
    if (input->pointerJustReleased && !g_isDraggingScrollbar) {
        g_webEngine.handlePointerClick(input->pointerX, input->pointerY);
    }

    // 7. Compute Responsive Layout & Render W3C DOM Tree
    g_webEngine.computeLayout(w, g_scrollY);
    g_webEngine.render();



    // 8. Render Draggable Scrollbar Thumb
    if (maxScroll > 0.0f || docH > h) {
        uint8_t alpha = (g_isDraggingScrollbar || isOverScrollbar) ? 210 : 120;
        roopm_draw_rect_rounded(thumbX, thumbY, thumbW, thumbH, 3.5f, ROOPM_RGBA(255, 255, 255, alpha));
    }

    // 9. Bottom Status Bar
    float barH = 22.0f;
    float barY = h - barH;
    roopm_draw_rect(0, barY, w, barH, ROOPM_RGBA(0, 0, 0, 220));
    roopm_draw_text(12.0f, barY + 4.0f, "RoopM Universal Web Browser | Lexbor + Yoga + QuickJS + Blend2D + STB (100% W3C Active)", 9.5f, ROOPM_RGBA(255, 255, 255, 170));
}

void app_shutdown(void) {
    g_webEngine.jsRuntime.shutdown();
}
