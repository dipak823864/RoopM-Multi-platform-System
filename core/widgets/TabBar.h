#ifndef ROOPM_TAB_BAR_H
#define ROOPM_TAB_BAR_H

#include "core/base/UIElement.h"
#include "core/animation/SpringPhysics.h"
#include <vector>
#include <functional>

namespace UIEngine {

class TabBar : public UIElement {
public:
    std::vector<std::string> tabs;
    int selectedIndex = 0;
    SpringSimulation indicatorSpring{0.0f, 200.0f, 15.0f};

    std::function<void(int)> onTabChanged;

    TabBar(const std::string &id = "", const std::vector<std::string> &tabList = {})
        : UIElement(id), tabs(tabList) {
        m_bounds.height = 42.0f;
        m_bounds.width = 300.0f;
        indicatorSpring.value = 0.0f;
        indicatorSpring.setTarget(0.0f);
    }

    void selectTab(int index) {
        if (index >= 0 && index < (int)tabs.size()) {
            selectedIndex = index;
            indicatorSpring.setTarget((float)selectedIndex);
            markRenderDirty();
            if (onTabChanged) onTabChanged(selectedIndex);
        }
    }

    void onUpdate(float dt) override {
        UIElement::onUpdate(dt);
        if (!indicatorSpring.isSettled()) {
            indicatorSpring.step(dt);
            markRenderDirty();
        }
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible || tabs.empty()) return false;

        if (event.type == EventType::PointerDown && m_bounds.contains(event.x, event.y)) {
            float tabW = m_bounds.width / (float)tabs.size();
            int clickedIndex = (int)((event.x - m_bounds.x) / tabW);
            selectTab(clickedIndex);
            event.handled = true;
            return true;
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_TAB_BAR_H
