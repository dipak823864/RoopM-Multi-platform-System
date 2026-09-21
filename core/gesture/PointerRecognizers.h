#ifndef ROOPM_POINTER_RECOGNIZERS_H
#define ROOPM_POINTER_RECOGNIZERS_H

#include "core/base/UIElement.h"
#include <functional>

namespace UIEngine {

class TapRecognizer {
public:
    float maxDistance = 10.0f;
    double maxDoubleTapInterval = 0.30;
    
    std::function<void(const Vec2 &pos)> onTap;
    std::function<void(const Vec2 &pos)> onDoubleTap;
    std::function<void(const Vec2 &pos)> onTripleTap;

    int tapCount = 0;
    double lastTapTime = 0.0;
    Vec2 startPos;

    void onPointerDown(const Vec2 &pos, double timestamp) {
        startPos = pos;
        if (timestamp - lastTapTime > maxDoubleTapInterval) {
            tapCount = 0;
        }
    }

    void onPointerUp(const Vec2 &pos, double timestamp) {
        if ((pos - startPos).length() <= maxDistance) {
            tapCount++;
            lastTapTime = timestamp;
            if (tapCount == 1 && onTap) onTap(pos);
            else if (tapCount == 2 && onDoubleTap) onDoubleTap(pos);
            else if (tapCount == 3) {
                if (onTripleTap) onTripleTap(pos);
                tapCount = 0;
            }
        } else {
            tapCount = 0;
        }
    }
};

class LongPressRecognizer {
public:
    double triggerDuration = 0.50;
    float maxDistance = 10.0f;
    
    std::function<void(const Vec2 &pos)> onLongPress;
    std::function<void(const Vec2 &pos, float duration)> onHoldTick;

    bool isDown = false;
    bool isTriggered = false;
    double downTime = 0.0;
    Vec2 startPos;

    void onPointerDown(const Vec2 &pos, double timestamp) {
        isDown = true;
        isTriggered = false;
        downTime = timestamp;
        startPos = pos;
    }

    void update(const Vec2 &pos, double currentTimestamp) {
        if (isDown && !isTriggered) {
            if ((pos - startPos).length() > maxDistance) {
                isDown = false;
                return;
            }
            double elapsed = currentTimestamp - downTime;
            if (elapsed >= triggerDuration) {
                isTriggered = true;
                if (onLongPress) onLongPress(pos);
            }
        } else if (isDown && isTriggered && onHoldTick) {
            onHoldTick(pos, (float)(currentTimestamp - downTime));
        }
    }

    void onPointerUp() { isDown = false; isTriggered = false; }
};

class PanGestureRecognizer {
public:
    std::function<void(const Vec2 &delta, const Vec2 &velocity)> onPanUpdate;
    std::function<void(const Vec2 &velocity)> onPanEnd;

    Vec2 lastPos;
    Vec2 velocity;
    bool isPanning = false;

    void onPointerDown(const Vec2 &pos) {
        lastPos = pos;
        velocity = {0, 0};
        isPanning = true;
    }

    void onPointerMove(const Vec2 &pos, float dt) {
        if (!isPanning) return;
        Vec2 delta = pos - lastPos;
        if (dt > 0.0001f) {
            velocity = delta * (1.0f / dt);
        }
        lastPos = pos;
        if (onPanUpdate) onPanUpdate(delta, velocity);
    }

    void onPointerUp() {
        if (isPanning && onPanEnd) onPanEnd(velocity);
        isPanning = false;
    }
};

class FlingRecognizer {
public:
    float friction = 0.95f;
    float minVelocity = 50.0f;
    std::function<void(const Vec2 &offset)> onFlingTick;

    Vec2 velocity;
    bool isFlinging = false;

    void trigger(const Vec2 &initialVelocity) {
        velocity = initialVelocity;
        isFlinging = velocity.length() > minVelocity;
    }

    void update(float dt) {
        if (!isFlinging) return;
        Vec2 offset = velocity * dt;
        velocity = velocity * friction;
        if (onFlingTick) onFlingTick(offset);
        if (velocity.length() < minVelocity) isFlinging = false;
    }
};

} // namespace UIEngine

#endif // ROOPM_POINTER_RECOGNIZERS_H
