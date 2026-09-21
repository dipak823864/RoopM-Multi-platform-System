#ifndef ROOPM_CHECKBOX_H
#define ROOPM_CHECKBOX_H

#include "core/base/UIElement.h"
#include <functional>

namespace UIEngine {

enum class CheckState {
    Unchecked,
    Checked,
    Indeterminate
};

class Checkbox : public UIElement {
public:
    CheckState checkState = CheckState::Unchecked;
    std::string label = "";
    bool allowIndeterminate = false;

    std::function<void(CheckState)> onChanged;

    Checkbox(const std::string &id = "", const std::string &lbl = "", CheckState initial = CheckState::Unchecked)
        : UIElement(id), label(lbl), checkState(initial) {
        m_bounds.width = 160.0f;
        m_bounds.height = 24.0f;
    }

    void cycleNext() {
        if (checkState == CheckState::Unchecked) checkState = CheckState::Checked;
        else if (checkState == CheckState::Checked) checkState = allowIndeterminate ? CheckState::Indeterminate : CheckState::Unchecked;
        else checkState = CheckState::Unchecked;

        markRenderDirty();
        if (onChanged) onChanged(checkState);
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;
        if (event.type == EventType::PointerDown && m_bounds.contains(event.x, event.y)) {
            cycleNext();
            event.handled = true;
            return true;
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_CHECKBOX_H
