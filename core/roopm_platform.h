/*
 * Roopm Engine - Platform Abstraction Layer
 *
 * This header defines the interface between the Core Engine and Platform
 * Shells. Platform implementations (Windows, Android, Web) must implement
 * RoopmPlatform.
 */

#ifndef ROOPM_PLATFORM_H
#define ROOPM_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Framebuffer - The Unified Memory Buffer
// =============================================================================

typedef struct {
  uint32_t *pixels; // RGBA pixel data (0xAARRGGBB format)
  int width;        // Buffer width in pixels
  int height;       // Buffer height in pixels
  int stride;       // Bytes per row (may include padding)
  float dpi;        // Device DPI for scaling (1.0 = 96dpi baseline)
} RoopmFramebuffer;

// =============================================================================
// Input - Unified Input State
// =============================================================================

typedef struct {
  // Pointer (Mouse on PC, Touch on Mobile)
  float pointerX;           // Current X position (DPI-adjusted)
  float pointerY;           // Current Y position (DPI-adjusted)
  bool pointerDown;         // Is currently pressed?
  bool pointerJustPressed;  // Was just pressed this frame?
  bool pointerJustReleased; // Was just released this frame?

  // Scroll/Wheel
  float scrollDeltaX; // Horizontal scroll delta
  float scrollDeltaY; // Vertical scroll delta

  // Keyboard
  uint32_t lastKeyCode;     // Last key pressed (platform-specific code)
  char lastChar;            // Last character typed (UTF-8 single byte)
  bool keyDown[256];        // Key states by scancode
  bool keyJustPressed[256]; // Keys just pressed this frame

  // Time
  float deltaTime;  // Seconds since last frame
  double totalTime; // Total seconds since app start

  // Window
  bool windowResized; // Was window resized this frame?
  bool shouldClose;   // Should app close? (window close button)
} RoopmInput;

// =============================================================================
// Platform Interface - Must be implemented by each platform
// =============================================================================

typedef struct RoopmPlatform RoopmPlatform;

struct RoopmPlatform {
  // Get the framebuffer to draw into
  RoopmFramebuffer *(*getFramebuffer)(RoopmPlatform *self);

  // Get current input state
  RoopmInput *(*getInput)(RoopmPlatform *self);

  // Present the framebuffer to screen (blit)
  void (*present)(RoopmPlatform *self);

  // Request a redraw (for event-driven rendering)
  void (*requestRedraw)(RoopmPlatform *self);

  // Platform-specific data (opaque pointer)
  void *userData;
};

// =============================================================================
// Color Helpers
// =============================================================================

#define ROOPM_RGBA(r, g, b, a)                                                 \
  (((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) |      \
   (uint32_t)(b))
#define ROOPM_RGB(r, g, b) ROOPM_RGBA(r, g, b, 255)

// Common colors
#define ROOPM_WHITE ROOPM_RGB(255, 255, 255)
#define ROOPM_BLACK ROOPM_RGB(0, 0, 0)
#define ROOPM_RED ROOPM_RGB(255, 0, 0)
#define ROOPM_GREEN ROOPM_RGB(0, 255, 0)
#define ROOPM_BLUE ROOPM_RGB(0, 0, 255)
#define ROOPM_GRAY ROOPM_RGB(128, 128, 128)
#define ROOPM_TRANSPARENT 0x00000000

#ifdef __cplusplus
}
#endif

#endif // ROOPM_PLATFORM_H
