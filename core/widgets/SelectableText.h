#ifndef ROOPM_SELECTABLE_TEXT_H
#define ROOPM_SELECTABLE_TEXT_H

#include "core/base/UIElement.h"
#include <string>
#include <algorithm>

namespace UIEngine {

class SelectableText : public UIElement {
public:
    std::string text = "";
    float fontSize = 12.0f;
    uint32_t textColor = 0xFFFFFFFF;
    
    int selectionStart = -1;
    int selectionEnd = -1;
    bool isSelecting = false;

    SelectableText(const std::string &id = "", const std::string &initialText = "")
        : UIElement(id), text(initialText) {
        m_bounds.width = 300.0f;
        m_bounds.height = 30.0f;
        selectionStart = 8;
        selectionEnd = 24;
    }

    bool hasSelection() const {
        return selectionStart >= 0 && selectionEnd >= 0 && selectionStart != selectionEnd;
    }

    void selectRange(int start, int end) {
        int len = (int)text.length();
        selectionStart = std::clamp(std::min(start, end), 0, len);
        selectionEnd = std::clamp(std::max(start, end), 0, len);
        markRenderDirty();
    }

    int getCharIndexAt(float localX, float charAvgWidth = 6.8f) const {
        if (text.empty()) return 0;
        int idx = (int)((localX) / charAvgWidth);
        return std::clamp(idx, 0, (int)text.length());
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;

        if (event.type == EventType::PointerDown && m_bounds.contains(event.x, event.y)) {
            isSelecting = true;
            int idx = getCharIndexAt(event.x - m_bounds.x);
            selectionStart = idx;
            selectionEnd = idx;
            markRenderDirty();
            return true;
        } else if (event.type == EventType::PointerMove && isSelecting) {
            int idx = getCharIndexAt(event.x - m_bounds.x);
            selectionEnd = idx;
            markRenderDirty();
            return true;
        } else if (event.type == EventType::PointerUp && isSelecting) {
            isSelecting = false;
            return true;
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_SELECTABLE_TEXT_H
