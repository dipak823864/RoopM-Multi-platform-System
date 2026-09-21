#ifndef ROOPM_SHADOW_H
#define ROOPM_SHADOW_H

#include <stdint.h>
#include "core/base/UIElement.h"

namespace UIEngine {

struct DropShadow {
    float offsetX = 0.0f;
    float offsetY = 4.0f;
    float blurRadius = 12.0f;
    float spreadRadius = 0.0f;
    uint32_t color = 0x66000000;

    Rect getExpandedBounds(const Rect &bounds) const {
        float pad = blurRadius + spreadRadius;
        return {
            bounds.x + offsetX - pad,
            bounds.y + offsetY - pad,
            bounds.width + pad * 2.0f,
            bounds.height + pad * 2.0f
        };
    }
};

struct InnerShadow {
    float offsetX = 0.0f;
    float offsetY = 2.0f;
    float blurRadius = 6.0f;
    uint32_t color = 0x44FFFFFF; // Acrylic top highlight
};

} // namespace UIEngine

#endif // ROOPM_SHADOW_H
