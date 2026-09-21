#ifndef ROOPM_CORNER_RADII_H
#define ROOPM_CORNER_RADII_H

#include <cmath>
#include <algorithm>

namespace UIEngine {

struct CornerRadii {
    float topLeft = 0.0f;
    float topRight = 0.0f;
    float bottomRight = 0.0f;
    float bottomLeft = 0.0f;

    CornerRadii() = default;
    CornerRadii(float all) : topLeft(all), topRight(all), bottomRight(all), bottomLeft(all) {}
    CornerRadii(float tl, float tr, float br, float bl)
        : topLeft(tl), topRight(tr), bottomRight(br), bottomLeft(bl) {}

    // Evaluates Signed Distance to 4-corner rounded box boundary
    float evaluateSDF(float px, float py, float width, float height) const {
        float halfW = width * 0.5f;
        float halfH = height * 0.5f;
        float qx = std::fabs(px - halfW);
        float qy = std::fabs(py - halfH);

        // Select radius based on quadrant
        float r = 0.0f;
        if (px < halfW && py < halfH) r = topLeft;
        else if (px >= halfW && py < halfH) r = topRight;
        else if (px >= halfW && py >= halfH) r = bottomRight;
        else r = bottomLeft;

        r = std::min({r, halfW, halfH});

        float boxW = halfW - r;
        float boxH = halfH - r;
        float dx = qx - boxW;
        float dy = qy - boxH;

        float outsideDist = std::sqrt(std::max(0.0f, dx) * std::max(0.0f, dx) + std::max(0.0f, dy) * std::max(0.0f, dy));
        float insideDist = std::min(std::max(dx, dy), 0.0f);
        return outsideDist + insideDist - r;
    }
};

} // namespace UIEngine

#endif // ROOPM_CORNER_RADII_H
