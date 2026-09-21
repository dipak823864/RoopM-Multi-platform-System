#ifndef ROOPM_SWITCH_H
#define ROOPM_SWITCH_H

#include "core/base/UIElement.h"
#include "core/animation/SpringPhysics.h"
#include "core/style/ThemeEngine.h"
#include "core/roopm.h"
#include <functional>

namespace UIEngine {

class Switch : public UIElement {
public:
    bool isChecked = false;
    std::string label = "Toggle Mode";
    std::function<void(bool)> onToggle;
    SpringSimulation thumbSpring{0.0f, 220.0f, 16.0f};

    Switch(const std::string &id = "", const std::string &lbl = "Toggle Mode", bool initial = false)
        : UIElement(id), label(lbl), isChecked(initial) {
        m_bounds.width = 220.0f;
        m_bounds.height = 28.0f;
        thumbSpring.value = isChecked ? 1.0f : 0.0f;
        thumbSpring.setTarget(thumbSpring.value);
    }

    void toggle() {
        isChecked = !isChecked;
        thumbSpring.setTarget(isChecked ? 1.0f : 0.0f);
        markRenderDirty();
        if (onToggle) onToggle(isChecked);
    }

    void onUpdate(float dt) override {
        UIElement::onUpdate(dt);
        if (!thumbSpring.isSettled()) {
            thumbSpring.step(dt);
            markRenderDirty();
        }
    }

    void onRender(void *ctx) override {
        if (!m_isVisible) return;
        bool isDark = ThemeEngine::instance().isDark;
        uint32_t swBg = isChecked ? ROOPM_RGB(34, 197, 94) : ROOPM_RGBA(0, 0, 0, 90);
        roopm_draw_rect_rounded(m_bounds.x, m_bounds.y, 48.0f, 26.0f, 13.0f, swBg);
        float knX = m_bounds.x + 3.0f + thumbSpring.value * 22.0f;
        roopm_draw_circle(knX + 10.0f, m_bounds.y + 13.0f, 9.5f, ROOPM_WHITE);
        
        uint32_t tCol = isDark ? ROOPM_RGB(248, 250, 252) : ROOPM_RGB(15, 23, 42);
        roopm_draw_text(m_bounds.x + 60.0f, m_bounds.y + 5.0f, label.c_str(), 12.0f, tCol);
        UIElement::onRender(ctx);
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;
        if (event.type == EventType::PointerDown && m_bounds.contains(event.x, event.y)) {
            toggle();
            event.handled = true;
            return true;
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_SWITCH_H
