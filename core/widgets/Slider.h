#ifndef ROOPM_SLIDER_H
#define ROOPM_SLIDER_H

#include "core/base/UIElement.h"
#include "core/style/ThemeEngine.h"
#include "core/roopm.h"
#include <functional>
#include <algorithm>

namespace UIEngine {

class Slider : public UIElement {
public:
    float value = 0.5f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    std::string label = "Volume";
    bool isDragging = false;
    std::function<void(float)> onValueChanged;

    Slider(const std::string &id = "", const std::string &lbl = "Volume", float initial = 0.5f)
        : UIElement(id), label(lbl), value(initial) {
        m_bounds.width = 240.0f;
        m_bounds.height = 44.0f;
    }

    void onRender(void *ctx) override {
        if (!m_isVisible) return;
        bool isDark = ThemeEngine::instance().isDark;
        uint32_t textSec = isDark ? ROOPM_RGBA(255, 255, 255, 170) : ROOPM_RGBA(30, 41, 59, 180);

        float trackY = m_bounds.y + 10.0f;
        roopm_draw_rect_rounded(m_bounds.x, trackY, m_bounds.width, 6.0f, 3.0f, ROOPM_RGBA(0, 0, 0, 80));
        roopm_draw_rect_rounded(m_bounds.x, trackY, m_bounds.width * value, 6.0f, 3.0f, ROOPM_RGB(59, 130, 246));
        roopm_draw_circle(m_bounds.x + m_bounds.width * value, trackY + 3.0f, 9.0f, ROOPM_WHITE);

        char buf[64];
        snprintf(buf, sizeof(buf), "%s: %.0f%%", label.c_str(), value * 100.0f);
        roopm_draw_text(m_bounds.x, m_bounds.y + 26.0f, buf, 11.0f, textSec);
        UIElement::onRender(ctx);
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;
        Rect hitBox{m_bounds.x - 4.0f, m_bounds.y, m_bounds.width + 8.0f, 30.0f};

        if (event.type == EventType::PointerDown && hitBox.contains(event.x, event.y)) {
            isDragging = true;
            value = std::clamp((event.x - m_bounds.x) / m_bounds.width, 0.0f, 1.0f);
            if (onValueChanged) onValueChanged(value);
            event.handled = true;
            return true;
        } else if (event.type == EventType::PointerMove && isDragging) {
            value = std::clamp((event.x - m_bounds.x) / m_bounds.width, 0.0f, 1.0f);
            if (onValueChanged) onValueChanged(value);
            event.handled = true;
            return true;
        } else if (event.type == EventType::PointerUp && isDragging) {
            isDragging = false;
            event.handled = true;
            return true;
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_SLIDER_H
