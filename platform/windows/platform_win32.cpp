/*
 * Roopm Engine - Windows Platform Shell
 * Win32 GDI Framebuffer implementation with smooth high-DPI scaling.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "core/roopm.h"
#include "core/roopm_platform.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#include <shellscalingapi.h>

#pragma comment(lib, "shcore.lib")

typedef struct {
  RoopmPlatform base;
  RoopmFramebuffer fb;
  RoopmInput input;

  HWND hwnd;
  HDC hdc;
  BITMAPINFO bmi;
  bool running;
  bool needsRedraw;

  LARGE_INTEGER perfFreq;
  LARGE_INTEGER lastTime;
} Win32Platform;

static Win32Platform g_win32;

static RoopmFramebuffer *win32_getFramebuffer(RoopmPlatform *self) {
  Win32Platform *p = (Win32Platform *)self;
  return &p->fb;
}

static RoopmInput *win32_getInput(RoopmPlatform *self) {
  Win32Platform *p = (Win32Platform *)self;
  return &p->input;
}

static void win32_present(RoopmPlatform *self) {
  Win32Platform *p = (Win32Platform *)self;
  StretchDIBits(p->hdc, 0, 0, p->fb.width, p->fb.height, 0, 0, p->fb.width,
                p->fb.height, p->fb.pixels, &p->bmi, DIB_RGB_COLORS, SRCCOPY);
}

static void win32_requestRedraw(RoopmPlatform *self) {
  Win32Platform *p = (Win32Platform *)self;
  p->needsRedraw = true;
  InvalidateRect(p->hwnd, NULL, FALSE);
}

static LRESULT CALLBACK Win32WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CLOSE:
    g_win32.running = false;
    g_win32.input.shouldClose = true;
    return 0;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  case WM_SIZE: {
    int newW = LOWORD(lParam);
    int newH = HIWORD(lParam);
    if (newW > 0 && newH > 0 && (newW != g_win32.fb.width || newH != g_win32.fb.height)) {
      if (g_win32.fb.pixels) free(g_win32.fb.pixels);
      g_win32.fb.width = newW;
      g_win32.fb.height = newH;
      g_win32.fb.stride = newW * sizeof(uint32_t);
      g_win32.fb.pixels = (uint32_t *)malloc(newW * newH * sizeof(uint32_t));

      g_win32.bmi.bmiHeader.biWidth = newW;
      g_win32.bmi.bmiHeader.biHeight = -newH;

      g_win32.input.windowResized = true;
      g_win32.needsRedraw = true;
    }
    return 0;
  }

  case WM_MOUSEMOVE:
    g_win32.input.pointerX = (float)GET_X_LPARAM(lParam) / g_win32.fb.dpi;
    g_win32.input.pointerY = (float)GET_Y_LPARAM(lParam) / g_win32.fb.dpi;
    g_win32.needsRedraw = true;
    return 0;

  case WM_LBUTTONDOWN:
    g_win32.input.pointerDown = true;
    g_win32.input.pointerJustPressed = true;
    g_win32.needsRedraw = true;
    SetCapture(hwnd);
    return 0;

  case WM_LBUTTONUP:
    g_win32.input.pointerDown = false;
    g_win32.input.pointerJustReleased = true;
    g_win32.needsRedraw = true;
    ReleaseCapture();
    return 0;

  case WM_MOUSEWHEEL:
    g_win32.input.scrollDeltaY = -(float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
    g_win32.needsRedraw = true;
    return 0;

  case WM_KEYDOWN:
    if (wParam < 256) {
      g_win32.input.keyDown[wParam] = true;
      g_win32.input.keyJustPressed[wParam] = true;
      g_win32.input.lastKeyCode = (uint32_t)wParam;
    }
    g_win32.needsRedraw = true;
    return 0;

  case WM_KEYUP:
    if (wParam < 256) g_win32.input.keyDown[wParam] = false;
    return 0;

  case WM_CHAR:
    g_win32.input.lastChar = (char)wParam;
    g_win32.needsRedraw = true;
    return 0;

  case WM_PAINT: {
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    win32_present(&g_win32.base);
    EndPaint(hwnd, &ps);
    return 0;
  }
  }
  return DefWindowProcA(hwnd, msg, wParam, lParam);
}

static bool win32_init(const char *title, int width, int height) {
  memset(&g_win32, 0, sizeof(g_win32));
  SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

  HDC screenDC = GetDC(NULL);
  int dpi = GetDeviceCaps(screenDC, LOGPIXELSX);
  ReleaseDC(NULL, screenDC);

  g_win32.fb.dpi = (float)dpi / 96.0f;
  if (g_win32.fb.dpi < 1.0f) g_win32.fb.dpi = 1.0f;

  int scaledW = (int)(width * g_win32.fb.dpi);
  int scaledH = (int)(height * g_win32.fb.dpi);

  WNDCLASSA wc = {0};
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = Win32WndProc;
  wc.hInstance = GetModuleHandleA(NULL);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = "RoopMWindowClass";
  RegisterClassA(&wc);

  RECT rect = {0, 0, scaledW, scaledH};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

  g_win32.hwnd = CreateWindowA(
      "RoopMWindowClass", title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
      rect.bottom - rect.top, NULL, NULL, GetModuleHandleA(NULL), NULL);

  if (!g_win32.hwnd) return false;

  g_win32.hdc = GetDC(g_win32.hwnd);
  g_win32.fb.width = scaledW;
  g_win32.fb.height = scaledH;
  g_win32.fb.stride = scaledW * sizeof(uint32_t);
  g_win32.fb.pixels = (uint32_t *)malloc(scaledW * scaledH * sizeof(uint32_t));

  memset(&g_win32.bmi, 0, sizeof(g_win32.bmi));
  g_win32.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  g_win32.bmi.bmiHeader.biWidth = scaledW;
  g_win32.bmi.bmiHeader.biHeight = -scaledH;
  g_win32.bmi.bmiHeader.biPlanes = 1;
  g_win32.bmi.bmiHeader.biBitCount = 32;
  g_win32.bmi.bmiHeader.biCompression = BI_RGB;

  g_win32.base.getFramebuffer = win32_getFramebuffer;
  g_win32.base.getInput = win32_getInput;
  g_win32.base.present = win32_present;
  g_win32.base.requestRedraw = win32_requestRedraw;
  g_win32.base.userData = &g_win32;

  QueryPerformanceFrequency(&g_win32.perfFreq);
  QueryPerformanceCounter(&g_win32.lastTime);

  g_win32.running = true;
  g_win32.needsRedraw = true;
  return true;
}

static void win32_shutdown(void) {
  if (g_win32.fb.pixels) {
    free(g_win32.fb.pixels);
    g_win32.fb.pixels = NULL;
  }
  if (g_win32.hdc) ReleaseDC(g_win32.hwnd, g_win32.hdc);
  if (g_win32.hwnd) DestroyWindow(g_win32.hwnd);
}

static void win32_update_time(void) {
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  g_win32.input.deltaTime = (float)(now.QuadPart - g_win32.lastTime.QuadPart) / (float)g_win32.perfFreq.QuadPart;
  if (g_win32.input.deltaTime > 0.05f) g_win32.input.deltaTime = 0.05f;
  g_win32.input.totalTime += g_win32.input.deltaTime;
  g_win32.lastTime = now;
}

static void win32_reset_input_events(void) {
  g_win32.input.pointerJustPressed = false;
  g_win32.input.pointerJustReleased = false;
  g_win32.input.scrollDeltaX = 0;
  g_win32.input.scrollDeltaY = 0;
  g_win32.input.lastChar = 0;
  g_win32.input.windowResized = false;
  memset(g_win32.input.keyJustPressed, 0, sizeof(g_win32.input.keyJustPressed));
}

extern void app_init(void);
extern void app_update(void);
extern void app_shutdown(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;

  if (!win32_init("RoopM", 800, 640)) return 1;

  roopm_init(&g_win32.base);
  app_init();

  while (g_win32.running) {
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        g_win32.running = false;
        break;
      }
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    }
    if (!g_win32.running) break;

    win32_update_time();
    roopm_begin_frame();
    app_update();
    roopm_end_frame();
    win32_reset_input_events();
    Sleep(16); // ~60 FPS
  }

  app_shutdown();
  roopm_shutdown();
  win32_shutdown();
  return 0;
}