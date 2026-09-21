#ifndef ROOPM_DROPDOWN_H
#define ROOPM_DROPDOWN_H

#include "core/base/UIElement.h"
#include "core/base/OverlayManager.h"
#include "core/style/ThemeEngine.h"
#include "core/roopm.h"
#include <vector>
#include <string>
#include <functional>

namespace UIEngine {

class Dropdown : public UIElement {
public:
    std::vector<std::string> options;
    int selectedIndex = 0;
    bool isOpen = false;
    std::function<void(int, const std::string &)> onSelectionChanged;

    Dropdown(const std::string &id = "", const std::vector<std::string> &items = {})
        : UIElement(id), options(items) {
        m_bounds.width = 240.0f;
        m_bounds.height = 36.0f;
    }

    void select(int index) {
        if (index >= 0 && index < (int)options.size()) {
            selectedIndex = index;
            isOpen = false;
            markRenderDirty();
            if (onSelectionChanged) onSelectionChanged(selectedIndex, options[selectedIndex]);
        }
    }

    std::string getSelectedText() const {
        return (selectedIndex >= 0 && selectedIndex < (int)options.size()) ? options[selectedIndex] : "";
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;
        if (event.type == EventType::PointerDown && m_bounds.contains(event.x, event.y)) {
            isOpen = !isOpen;
            event.handled = true;
            return true;
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_DROPDOWN_H
