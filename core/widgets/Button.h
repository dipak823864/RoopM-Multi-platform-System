#ifndef ROOPM_BUTTON_H
#define ROOPM_BUTTON_H

#include "core/base/UIElement.h"
#include "core/style/ThemeEngine.h"
#include "core/roopm.h"
#include <functional>

namespace UIEngine {

class Button : public UIElement {
public:
    std::string text = "Button";
    std::function<void()> onClick;
    float cornerRadius = 10.0f;

    Button(const std::string &id = "", const std::string &label = "Button")
        : UIElement(id), text(label) {
        m_bounds.width = 140.0f;
        m_bounds.height = 38.0f;
    }

    void onRender(void *ctx) override {
        if (!m_isVisible) return;
        uint32_t btnBg = hasState(WidgetState::Pressed) ? ROOPM_RGB(37, 99, 235) :
                        (hasState(WidgetState::Hovered) ? ROOPM_RGB(59, 130, 246) : ROOPM_RGB(30, 58, 138));
        roopm_draw_rect_rounded(m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height, cornerRadius, btnBg);
        roopm_draw_rect_rounded_outline(m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height, cornerRadius, ROOPM_RGBA(255, 255, 255, 60), 1.0f);
        roopm_draw_text_centered(m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height, text.c_str(), 12.0f, ROOPM_WHITE);
        UIElement::onRender(ctx);
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;

        if (event.type == EventType::PointerDown && m_bounds.contains(event.x, event.y)) {
            setState(WidgetState::Pressed);
            event.handled = true;
            return true;
        } else if (event.type == EventType::PointerMove) {
            if (m_bounds.contains(event.x, event.y)) {
                if (!hasState(WidgetState::Pressed)) setState(WidgetState::Hovered);
            } else {
                setState(WidgetState::Normal);
            }
        } else if (event.type == EventType::PointerUp) {
            if (hasState(WidgetState::Pressed) && m_bounds.contains(event.x, event.y)) {
                setState(WidgetState::Hovered);
                if (onClick) onClick();
                event.handled = true;
                return true;
            }
            setState(WidgetState::Normal);
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_BUTTON_H
