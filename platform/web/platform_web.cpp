#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../../core/roopm_engine.h"

// =============================================================================
// Web State
// =============================================================================

struct WebState {
    uint32_t *pixels;
    int currWidth;
    int currHeight;
    float dpi;
    bool needsResize;
    double lastTime;
};

static WebState g_web = {0};

// =============================================================================
// Input Callbacks
// =============================================================================

static EM_BOOL on_resize(int eventType, const EmscriptenUiEvent *e, void *userData) {
    g_web.needsResize = true;
    return EM_TRUE;
}

static EM_BOOL on_mouse(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    RoopmInput *input = roopm_get_input();
    
    // Convert client coordinates to canvas coordinates if needed
    // For now assuming canvas fills window or input is relative to target
    input->mouseX = (float)e->targetX;
    input->mouseY = (float)e->targetY;
    
    // Scale for High DPI if canvas is scaled
    double cssW, cssH;
    emscripten_get_element_css_size("#canvas", &cssW, &cssH);
    if (cssW > 0 && g_web.currWidth > 0) {
        float scale = (float)g_web.currWidth / (float)cssW;
        input->mouseX *= scale;
        input->mouseY *= scale;
    }

    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) {
        if (e->button == 0) input->mouseLeft = true;
    } else if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
        if (e->button == 0) input->mouseLeft = false;
    }
    
    return EM_TRUE;
}

static EM_BOOL on_wheel(int eventType, const EmscriptenWheelEvent *e, void *userData) {
    RoopmInput *input = roopm_get_input();
    // Normalize scroll delta
    float delta = (float)e->deltaY;
    if (e->deltaMode == DOM_DELTA_LINE) delta *= 20.0f;
    if (e->deltaMode == DOM_DELTA_PAGE) delta *= 100.0f;
    
    // Web usually sends positive for DOWN.
    // Our engine expects positive for UP (scroll content down) ??
    // Wait, platform_win32: wheel delta > 0 (UP) -> input->scrollDeltaY > 0
    // platform_android: drag UP -> lastY - y > 0 -> input->scrollDeltaY > 0.
    
    // Web deltaY > 0 means SCROLL DOWN (content moves up).
    // So we want negative here to match Windows wheel logic?
    // No, Windows Delta > 0 is UP.
    // Web Delta > 0 is DOWN.
    // So Web Delta * -1 is roughly Windows Delta.
    
    // Let's stick to "Positive = Scroll Down" logic we established in app.cpp?
    // In app.cpp: g_scrollY += input->scrollDeltaY * scrollMultiplier;
    // We want g_scrollY to INCREASE when we scroll DOWN.
    // So input->scrollDeltaY should be POSITIVE when scrolling DOWN.
    
    // Web deltaY is POSITIVE when scrolling DOWN. So passed directly.
    input->scrollDeltaY = delta * 0.1f; 
    
    return EM_TRUE;
}

static EM_BOOL on_touch(int eventType, const EmscriptenTouchEvent *e, void *userData) {
    RoopmInput *input = roopm_get_input();
    
    if (e->numTouches > 0) {
        const EmscriptenTouchPoint *t = &e->touches[0];
        
        float tx = (float)t->targetX;
        float ty = (float)t->targetY;
        
        double cssW, cssH;
        emscripten_get_element_css_size("#canvas", &cssW, &cssH);
        if (cssW > 0 && g_web.currWidth > 0) {
            float scale = (float)g_web.currWidth / (float)cssW;
            tx *= scale;
            ty *= scale;
        }

        input->mouseX = tx;
        input->mouseY = ty;
        
        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) input->mouseLeft = true;
        if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) input->mouseLeft = false;
        
        // Prevent default to stop browser scrolling
        return EM_TRUE; 
    }
    return EM_FALSE;
}

// =============================================================================
// Main Loop
// =============================================================================

void web_main_loop() {
    double now = emscripten_get_now() / 1000.0;
    float dt = (float)(now - g_web.lastTime);
    g_web.lastTime = now;
    
    // Clamp huge usage spikes (e.g. tab switching)
    if (dt > 0.1f) dt = 0.1f;

    // Handle Resize
    if (g_web.needsResize) {
        g_web.needsResize = false;
        
        double w, h;
        emscripten_get_element_css_size("#canvas", &w, &h);
        
        // High DPI support
        float dpr = emscripten_get_device_pixel_ratio();
        int newW = (int)(w * dpr);
        int newH = (int)(h * dpr);
        
        if (newW != g_web.currWidth || newH != g_web.currHeight) {
            g_web.currWidth = newW;
            g_web.currHeight = newH;
            
            // Resize canvas buffer
            emscripten_set_canvas_element_size("#canvas", newW, newH);
            
            // Reallocate pixel buffer
            if (g_web.pixels) free(g_web.pixels);
            g_web.pixels = (uint32_t*)malloc(newW * newH * sizeof(uint32_t));
            
            // Provide simple DPI approximation
            // Base DPI 96 * dpr
            g_web.dpi = 96.0f * dpr / 96.0f; // Simplified scale factor
        }
    }
    
    if (!g_web.pixels) return;

    // Run Frame
    roopm_platform_update(g_web.currWidth, g_web.currHeight, g_web.pixels, g_web.dpi, dt);
    
    // Blit to Canvas
    // optimized way: get 2D context image data and put it
    // But easier via embedded JS for pixel copy
    EM_ASM({
        var canvas = document.getElementById('canvas');
        var ctx = canvas.getContext('2d');
        var w = $0;
        var h = $1;
        var ptr = $2; // Pointer to g_web.pixels
        
        // Create ImageData if size changed or not created
        if (!canvas.imgData || canvas.imgData.width != w || canvas.imgData.height != h) {
            canvas.imgData = ctx.createImageData(w, h);
        }
        
        // Copy WASM memory to JS Uint8ClampedArray
        // HEAPU8 view of memory
        var imgData = canvas.imgData;
        var data = imgData.data;
        
        // Fast copy?
        // Source is RGBA 32-bit (uint32). HTML5 Canvas expects RGBA 8-bit.
        // C++: 0xAABBGGRR (Little Endian uint32) -> R, G, B, A in bytes?
        // Wait, roopm_render uses 0xAABBGGRR.
        // Memory: [RR] [GG] [BB] [AA] (Low->High)
        // Canvas expects: R, G, B, A order.
        // So direct byte copy works!
        
        // The heap is HUGE, we need subarray
        var src = new Uint8ClampedArray(Module.HEAPU8.buffer, ptr, w * h * 4);
        data.set(src);
        
        ctx.putImageData(imgData, 0, 0);
    }, g_web.currWidth, g_web.currHeight, g_web.pixels);
}

// =============================================================================
// Entry Point
// =============================================================================

int main() {
    printf("Roopm Web Init\\n");
    
    // Initial Setup
    g_web.needsResize = true; // Force initial size calculation
    g_web.lastTime = emscripten_get_now() / 1000.0;
    
    // Register Callbacks
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, false, on_resize);
    emscripten_set_mousemove_callback("#canvas", NULL, false, on_mouse);
    emscripten_set_mousedown_callback("#canvas", NULL, false, on_mouse);
    emscripten_set_mouseup_callback("#canvas", NULL, false, on_mouse);
    emscripten_set_wheel_callback("#canvas", NULL, false, on_wheel);
    
    emscripten_set_touchstart_callback("#canvas", NULL, false, on_touch);
    emscripten_set_touchend_callback("#canvas", NULL, false, on_touch);
    emscripten_set_touchmove_callback("#canvas", NULL, false, on_touch);
    
    // Start Loop
    emscripten_set_main_loop(web_main_loop, 0, 1);
    
    return 0;
}
