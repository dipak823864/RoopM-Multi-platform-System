#ifndef ROOPM_DISPLAY_METRICS_H
#define ROOPM_DISPLAY_METRICS_H

#include <stdint.h>

namespace UIEngine {

struct DisplayMetrics {
    float dpiScale = 1.0f;       // 1.0 = 96 DPI (Baseline desktop)
    float screenWidthPx = 1920.0f;
    float screenHeightPx = 1080.0f;
    float rootFontSize = 16.0f;  // For rem calculations
    
    // Dynamic universal unit conversion
    inline float dp(float val) const { return val * dpiScale; }
    inline float pt(float val) const { return val * (dpiScale * (96.0f / 72.0f)); }
    inline float rem(float val) const { return val * rootFontSize * dpiScale; }
    inline float em(float val, float currentFontSize) const { return val * currentFontSize * dpiScale; }
    inline float vw(float val) const { return (val / 100.0f) * screenWidthPx; }
    inline float vh(float val) const { return (val / 100.0f) * screenHeightPx; }
    inline float snap(float val) const { return (float)((int)(val + 0.5f)); }
};

} // namespace UIEngine

#endif // ROOPM_DISPLAY_METRICS_H
