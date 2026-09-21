#ifndef ROOPM_GESTURE_ARENA_H
#define ROOPM_GESTURE_ARENA_H

#include <stdint.h>
#include <vector>
#include <cmath>

namespace UIEngine {

struct TouchPointer {
    int32_t id = 0;
    float startX = 0, startY = 0;
    float currentX = 0, currentY = 0;
    float velocityX = 0, velocityY = 0;
    double downTimestamp = 0;
    bool isActive = false;
};

enum class GestureType {
    None,
    Tap,
    DoubleTap,
    LongPress,
    Pan,
    Fling,
    PinchToZoom,
    Rotation,
    EdgeSwipe
};

class GestureArena {
public:
    TouchPointer pointers[10];
    int activePointerCount = 0;
    
    GestureType currentGesture = GestureType::None;
    float pinchScale = 1.0f;
    float rotationRadians = 0.0f;

    void onPointerDown(int id, float x, float y, double time) {
        for (auto &p : pointers) {
            if (!p.isActive) {
                p.id = id; p.startX = p.currentX = x; p.startY = p.currentY = y;
                p.downTimestamp = time; p.isActive = true;
                activePointerCount++;
                break;
            }
        }
        resolveAmbiguity();
    }

    void onPointerMove(int id, float x, float y) {
        for (auto &p : pointers) {
            if (p.isActive && p.id == id) {
                p.velocityX = x - p.currentX;
                p.velocityY = y - p.currentY;
                p.currentX = x;
                p.currentY = y;
                break;
            }
        }
        resolveAmbiguity();
    }

    void onPointerUp(int id) {
        for (auto &p : pointers) {
            if (p.isActive && p.id == id) {
                p.isActive = false;
                if (activePointerCount > 0) activePointerCount--;
                break;
            }
        }
        if (activePointerCount == 0) currentGesture = GestureType::None;
    }

private:
    void resolveAmbiguity() {
        if (activePointerCount == 2) {
            TouchPointer &p1 = pointers[0];
            TouchPointer &p2 = pointers[1];
            float dx = p2.currentX - p1.currentX;
            float dy = p2.currentY - p1.currentY;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            float origDx = p2.startX - p1.startX;
            float origDy = p2.startY - p1.startY;
            float origDist = std::sqrt(origDx * origDx + origDy * origDy);
            
            if (origDist > 0.001f) {
                pinchScale = dist / origDist;
                currentGesture = GestureType::PinchToZoom;
            }
        } else if (activePointerCount == 1) {
            TouchPointer &p = pointers[0];
            float dx = p.currentX - p.startX;
            float dy = p.currentY - p.startY;
            if (std::sqrt(dx * dx + dy * dy) > 8.0f) {
                currentGesture = GestureType::Pan;
            }
        }
    }
};

} // namespace UIEngine

#endif // ROOPM_GESTURE_ARENA_H
