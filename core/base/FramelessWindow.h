#ifndef ROOPM_FRAMELESS_WINDOW_H
#define ROOPM_FRAMELESS_WINDOW_H

#include "core/base/UIElement.h"

namespace UIEngine {

enum class WindowHitZone {
    Client,
    Caption,
    ResizeLeft,
    ResizeRight,
    ResizeTop,
    ResizeBottom,
    ResizeTopLeft,
    ResizeTopRight,
    ResizeBottomLeft,
    ResizeBottomRight
};

class FramelessWindowController {
public:
    float borderThickness = 6.0f;
    float captionHeight = 36.0f;

    WindowHitZone hitTest(float px, float py, float winW, float winH) const {
        bool onLeft = (px <= borderThickness);
        bool onRight = (px >= winW - borderThickness);
        bool onTop = (py <= borderThickness);
        bool onBottom = (py >= winH - borderThickness);

        // 4 Corners
        if (onTop && onLeft) return WindowHitZone::ResizeTopLeft;
        if (onTop && onRight) return WindowHitZone::ResizeTopRight;
        if (onBottom && onLeft) return WindowHitZone::ResizeBottomLeft;
        if (onBottom && onRight) return WindowHitZone::ResizeBottomRight;

        // 4 Edges
        if (onLeft) return WindowHitZone::ResizeLeft;
        if (onRight) return WindowHitZone::ResizeRight;
        if (onTop) return WindowHitZone::ResizeTop;
        if (onBottom) return WindowHitZone::ResizeBottom;

        // Caption Bar Drag Zone
        if (py <= captionHeight) return WindowHitZone::Caption;

        return WindowHitZone::Client;
    }

    void calculateResize(WindowHitZone zone, float dx, float dy, Rect startBounds, Rect &outBounds) const {
        outBounds = startBounds;
        if (zone == WindowHitZone::ResizeRight || zone == WindowHitZone::ResizeTopRight || zone == WindowHitZone::ResizeBottomRight) {
            outBounds.width = std::max(200.0f, startBounds.width + dx);
        }
        if (zone == WindowHitZone::ResizeBottom || zone == WindowHitZone::ResizeBottomLeft || zone == WindowHitZone::ResizeBottomRight) {
            outBounds.height = std::max(150.0f, startBounds.height + dy);
        }
        if (zone == WindowHitZone::ResizeLeft || zone == WindowHitZone::ResizeTopLeft || zone == WindowHitZone::ResizeBottomLeft) {
            float newW = std::max(200.0f, startBounds.width - dx);
            outBounds.x = startBounds.x + (startBounds.width - newW);
            outBounds.width = newW;
        }
        if (zone == WindowHitZone::ResizeTop || zone == WindowHitZone::ResizeTopLeft || zone == WindowHitZone::ResizeTopRight) {
            float newH = std::max(150.0f, startBounds.height - dy);
            outBounds.y = startBounds.y + (startBounds.height - newH);
            outBounds.height = newH;
        }
    }
};

} // namespace UIEngine

#endif // ROOPM_FRAMELESS_WINDOW_H
