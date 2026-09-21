#ifndef ROOPM_RANGE_SLIDER_H
#define ROOPM_RANGE_SLIDER_H

#include "core/base/UIElement.h"
#include <functional>
#include <algorithm>

namespace UIEngine {

class RangeSlider : public UIElement {
public:
    float lowValue = 0.2f;
    float highValue = 0.8f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    
    int activeThumb = 0; // 0 = none, 1 = low, 2 = high
    std::function<void(float, float)> onRangeChanged;

    RangeSlider(const std::string &id = "", float low = 0.2f, float high = 0.8f, float minV = 0.0f, float maxV = 1.0f)
        : UIElement(id), lowValue(low), highValue(high), minValue(minV), maxValue(maxV) {
        m_bounds.width = 240.0f;
        m_bounds.height = 24.0f;
    }

    bool isDragging() const { return activeThumb != 0; }

    void setRange(float low, float high) {
        lowValue = std::clamp(low, minValue, maxValue);
        highValue = std::clamp(high, lowValue, maxValue);
        markRenderDirty();
        if (onRangeChanged) onRangeChanged(lowValue, highValue);
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;

        if (event.type == EventType::PointerDown && m_bounds.contains(event.x, event.y)) {
            float norm = (event.x - m_bounds.x) / m_bounds.width;
            float clickVal = minValue + norm * (maxValue - minValue);
            float distLow = std::fabs(clickVal - lowValue);
            float distHigh = std::fabs(clickVal - highValue);

            activeThumb = (distLow <= distHigh) ? 1 : 2;
            if (activeThumb == 1) setRange(clickVal, highValue);
            else setRange(lowValue, clickVal);

            event.handled = true;
            return true;
        } else if (event.type == EventType::PointerMove && activeThumb != 0) {
            float norm = (event.x - m_bounds.x) / m_bounds.width;
            float currentVal = minValue + norm * (maxValue - minValue);
            if (activeThumb == 1) setRange(currentVal, highValue);
            else setRange(lowValue, currentVal);

            event.handled = true;
            return true;
        } else if (event.type == EventType::PointerUp && activeThumb != 0) {
            activeThumb = 0;
            event.handled = true;
            return true;
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_RANGE_SLIDER_H
