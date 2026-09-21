struct android_app *g_android_app_ptr = nullptr;
#include "core/platform/IVFS.h"
#include "core/web/WebEngine.h"
extern UIEngine::WebEngine g_webEngine;
/*
 * Roopm Engine - Android Platform Shell
 * NativeActivity implementation using ANativeWindow for framebuffer.
 */

#include <android/input.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "core/roopm.h"
#include "core/roopm_platform.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RoopM", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "RoopM", __VA_ARGS__)

typedef struct {
  RoopmPlatform base;
  RoopmFramebuffer fb;
  RoopmInput input;

  struct android_app *app;
  ANativeWindow *window;
  bool running;
  bool needsRedraw;
  bool initialized;

  struct timespec lastTime;
} AndroidPlatform;

static AndroidPlatform g_android;

static RoopmFramebuffer *android_getFramebuffer(RoopmPlatform *self) {
  AndroidPlatform *p = (AndroidPlatform *)self;
  return &p->fb;
}

static RoopmInput *android_getInput(RoopmPlatform *self) {
  AndroidPlatform *p = (AndroidPlatform *)self;
  return &p->input;
}

static void android_present(RoopmPlatform *self) {
  AndroidPlatform *p = (AndroidPlatform *)self;
  if (!p->window || !p->fb.pixels) return;

  ANativeWindow_Buffer buffer;
  if (ANativeWindow_lock(p->window, &buffer, NULL) < 0) {
    return;
  }

  uint32_t *src = p->fb.pixels;
  uint32_t *dst = (uint32_t *)buffer.bits;
  int minW = p->fb.width < buffer.width ? p->fb.width : buffer.width;
  int minH = p->fb.height < buffer.height ? p->fb.height : buffer.height;

  for (int y = 0; y < minH; y++) {
    uint32_t *srcRow = src + y * p->fb.width;
    uint32_t *dstRow = dst + y * buffer.stride;
    for (int x = 0; x < minW; x++) {
      uint32_t pixel = srcRow[x];
      // Fast ARGB to ABGR conversion (swap R and B)
      dstRow[x] = (pixel & 0xFF00FF00) | ((pixel & 0x00FF0000) >> 16) | ((pixel & 0x000000FF) << 16);
    }
  }

  ANativeWindow_unlockAndPost(p->window);
}

static void android_requestRedraw(RoopmPlatform *self) {
  AndroidPlatform *p = (AndroidPlatform *)self;
  p->needsRedraw = true;
  if (p->app && p->app->looper) {
    ALooper_wake(p->app->looper);
  }
}

static void android_init_window(ANativeWindow *window) {
  if (!window) return;

  g_android.window = window;
  int w = ANativeWindow_getWidth(window);
  int h = ANativeWindow_getHeight(window);
  if (w <= 0 || h <= 0) return; // Prevent 0-byte black buffers

  ANativeWindow_setBuffersGeometry(window, w, h, WINDOW_FORMAT_RGBX_8888);
  g_webEngine.markLayoutDirty();

  if (g_android.fb.pixels) {
    free(g_android.fb.pixels);
  }

  g_android.fb.width = w;
  g_android.fb.height = h;
  g_android.fb.stride = w * sizeof(uint32_t);
  
  // High quality standard scaling (380dp target width)
  g_android.fb.dpi = (float)w / 380.0f;
  if (g_android.fb.dpi < 1.0f) g_android.fb.dpi = 1.0f;
  g_android.fb.pixels = (uint32_t *)malloc(w * h * sizeof(uint32_t));

  if (!g_android.fb.pixels) return;

  memset(g_android.fb.pixels, 255, w * h * sizeof(uint32_t));
  g_android.input.windowResized = true;
  g_android.needsRedraw = true;
}

static void android_term_window(void) {
  if (g_android.fb.pixels) {
    free(g_android.fb.pixels);
    g_android.fb.pixels = NULL;
  }
  g_android.window = NULL;
}

static int32_t android_handle_input(struct android_app *app, AInputEvent *event) {
  (void)app;
  int type = AInputEvent_getType(event);

  if (type == AINPUT_EVENT_TYPE_MOTION) {
    int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
    float x = AMotionEvent_getX(event, 0) / g_android.fb.dpi;
    float y = AMotionEvent_getY(event, 0) / g_android.fb.dpi;

    g_android.input.pointerX = x;
    g_android.input.pointerY = y;

    switch (action) {
    case AMOTION_EVENT_ACTION_DOWN:
      g_android.input.pointerDown = true;
      g_android.input.pointerJustPressed = true;
      break;
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_CANCEL:
      g_android.input.pointerDown = false;
      g_android.input.pointerJustReleased = true;
      break;
    case AMOTION_EVENT_ACTION_MOVE:
      // Touch coordinates are directly passed into pointerX/pointerY for 1:1 physics
      break;
    }
    g_android.needsRedraw = true;
    return 1;
  }
  return 0;
}

static void android_handle_cmd(struct android_app *app, int32_t cmd) {
  switch (cmd) {
  case APP_CMD_INIT_WINDOW:
    android_init_window(app->window);
    break;
  case APP_CMD_TERM_WINDOW:
    android_term_window();
    break;
  case APP_CMD_CONFIG_CHANGED:
  case APP_CMD_WINDOW_RESIZED:
    if (app->window) android_init_window(app->window);
    break;
  case APP_CMD_GAINED_FOCUS:
    g_android.needsRedraw = true;
    break;
  case APP_CMD_DESTROY:
    g_android.running = false;
    g_android.input.shouldClose = true;
    break;
  }
}

static void android_update_time(void) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  double nowSec = now.tv_sec + now.tv_nsec / 1e9;
  double lastSec = g_android.lastTime.tv_sec + g_android.lastTime.tv_nsec / 1e9;

  g_android.input.deltaTime = (float)(nowSec - lastSec);
  if (g_android.input.deltaTime > 0.05f) g_android.input.deltaTime = 0.05f;
  if (g_android.input.deltaTime <= 0.0f) g_android.input.deltaTime = 0.016f;
  g_android.input.totalTime = nowSec;
  g_android.lastTime = now;
}

static void android_reset_input_events(void) {
  g_android.input.pointerJustPressed = false;
  g_android.input.pointerJustReleased = false;
  g_android.input.scrollDeltaX = 0;
  g_android.input.scrollDeltaY = 0;
  g_android.input.lastChar = 0;
  g_android.input.windowResized = false;
  memset(g_android.input.keyJustPressed, 0, sizeof(g_android.input.keyJustPressed));
}

extern void app_init(void);
extern void app_update(void);
extern void app_shutdown(void);

void android_main(struct android_app *app) {
  LOGI("=== RoopM Starting ===");
  memset(&g_android, 0, sizeof(g_android));

  g_android.app = app;
  g_android_app_ptr = app;
  g_android.running = true;
  g_android.needsRedraw = true;

  g_android.base.getFramebuffer = android_getFramebuffer;
  g_android.base.getInput = android_getInput;
  g_android.base.present = android_present;
  g_android.base.requestRedraw = android_requestRedraw;
  g_android.base.userData = &g_android;

  app->onAppCmd = android_handle_cmd;
  app->onInputEvent = android_handle_input;

  clock_gettime(CLOCK_MONOTONIC, &g_android.lastTime);

  while (!g_android.window && g_android.running) {
    int events;
    struct android_poll_source *source;
    while (ALooper_pollOnce(100, NULL, &events, (void **)&source) >= 0) {
      if (source) source->process(app, source);
      if (app->destroyRequested) {
        g_android.running = false;
        break;
      }
    }
  }

  if (!g_android.running || !g_android.window) return;

    // 🌟 Pure W3C Virtual File System (Streaming directly from APK via AAssetManager)
  g_webEngine.setVFS(std::make_shared<UIEngine::AndroidAssetVFS>(app->activity->assetManager));

  roopm_init(&g_android.base);
  app_init();
  g_android.initialized = true;

  while (g_android.running) {
    int events;
    struct android_poll_source *source;
    while (ALooper_pollOnce(0, NULL, &events, (void **)&source) >= 0) {
      if (source) source->process(app, source);
      if (app->destroyRequested) {
        g_android.running = false;
        break;
      }
    }

    if (!g_android.running) break;

    if (g_android.window && g_android.fb.pixels) {
      android_update_time();
      roopm_begin_frame();
      app_update();
      roopm_end_frame();
      android_reset_input_events();
      usleep(16000); // 60 FPS
    }
  }

  if (g_android.initialized) {
    app_shutdown();
    roopm_shutdown();
  }
  android_term_window();
}