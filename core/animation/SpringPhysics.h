#ifndef ROOPM_SPRING_PHYSICS_H
#define ROOPM_SPRING_PHYSICS_H

#include <cmath>
#include <algorithm>

namespace UIEngine {

class SpringSimulation {
public:
    float stiffness = 180.0f;
    float damping = 12.0f;
    float mass = 1.0f;
    
    float value = 0.0f;
    float velocity = 0.0f;
    float targetValue = 0.0f;

    SpringSimulation(float _val = 0.0f, float _k = 180.0f, float _c = 12.0f)
        : stiffness(_k), damping(_c), value(_val), targetValue(_val) {}

    void setTarget(float target) { targetValue = target; }

    void step(float dt) {
        float force = -stiffness * (value - targetValue) - damping * velocity;
        float acceleration = force / mass;
        velocity += acceleration * dt;
        value += velocity * dt;
    }

    bool isSettled(float threshold = 0.001f) const {
        return std::fabs(value - targetValue) < threshold && std::fabs(velocity) < threshold;
    }
};

enum class EasingType {
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutCubic,
    EaseInExpo,
    EaseOutElastic,
    EaseOutBounce
};

inline float evaluateEasing(EasingType type, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    switch (type) {
    case EasingType::Linear: return t;
    case EasingType::EaseInQuad: return t * t;
    case EasingType::EaseOutQuad: return t * (2.0f - t);
    case EasingType::EaseInOutCubic: return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
    case EasingType::EaseInExpo: return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f));
    case EasingType::EaseOutBounce: {
        float n1 = 7.5625f, d1 = 2.75f;
        if (t < 1.0f / d1) return n1 * t * t;
        else if (t < 2.0f / d1) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
        else if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
        else { t -= 2.625f / d1; return n1 * t * t + 0.984375f; }
    }
    default: return t;
    }
}

} // namespace UIEngine

#endif // ROOPM_SPRING_PHYSICS_H
