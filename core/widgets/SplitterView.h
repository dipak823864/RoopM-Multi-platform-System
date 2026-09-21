#ifndef ROOPM_SPLITTER_VIEW_H
#define ROOPM_SPLITTER_VIEW_H

#include "core/base/UIElement.h"
#include <algorithm>

namespace UIEngine {

enum class SplitDirection {
    Horizontal,
    Vertical
};

class SplitterView : public UIElement {
public:
    SplitDirection direction = SplitDirection::Horizontal;
    float splitRatio = 0.5f;
    float splitterThickness = 6.0f;
    float minPaneSize = 60.0f;
    bool isDragging = false;

    SplitterView(const std::string &id = "", SplitDirection dir = SplitDirection::Horizontal)
        : UIElement(id), direction(dir) {
        m_bounds.width = 600.0f;
        m_bounds.height = 300.0f;
        splitRatio = 0.48f;
    }

    Rect getSplitterHandleRect() const {
        if (direction == SplitDirection::Horizontal) {
            float handleX = m_bounds.x + m_bounds.width * splitRatio - splitterThickness * 0.5f;
            return {handleX, m_bounds.y, splitterThickness, m_bounds.height};
        } else {
            float handleY = m_bounds.y + m_bounds.height * splitRatio - splitterThickness * 0.5f;
            return {m_bounds.x, handleY, m_bounds.width, splitterThickness};
        }
    }

    void setSplitRatio(float r) {
        if (m_bounds.width <= 1.0f || m_bounds.height <= 1.0f) {
            splitRatio = std::clamp(r, 0.15f, 0.85f);
            return;
        }
        if (direction == SplitDirection::Horizontal) {
            float minRatio = minPaneSize / m_bounds.width;
            float maxRatio = 1.0f - minRatio;
            if (minRatio >= maxRatio) { splitRatio = 0.5f; return; }
            splitRatio = std::clamp(r, minRatio, maxRatio);
        } else {
            float minRatio = minPaneSize / m_bounds.height;
            float maxRatio = 1.0f - minRatio;
            if (minRatio >= maxRatio) { splitRatio = 0.5f; return; }
            splitRatio = std::clamp(r, minRatio, maxRatio);
        }
        markLayoutDirty();
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;
        Rect handle = getSplitterHandleRect();
        Rect expandedHandle{handle.x - 4.0f, handle.y, handle.width + 8.0f, handle.height};

        if (event.type == EventType::PointerDown && expandedHandle.contains(event.x, event.y)) {
            isDragging = true;
            event.handled = true;
            return true;
        } else if (event.type == EventType::PointerMove && isDragging) {
            float r = (direction == SplitDirection::Horizontal) ? 
                      ((event.x - m_bounds.x) / m_bounds.width) : 
                      ((event.y - m_bounds.y) / m_bounds.height);
            setSplitRatio(r);
            event.handled = true;
            return true;
        } else if (event.type == EventType::PointerUp && isDragging) {
            isDragging = false;
            event.handled = true;
            return true;
        }
        return UIElement::onEvent(event);
    }
};

} // namespace UIEngine

#endif // ROOPM_SPLITTER_VIEW_H
