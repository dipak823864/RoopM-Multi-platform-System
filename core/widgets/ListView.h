#ifndef ROOPM_LIST_VIEW_H
#define ROOPM_LIST_VIEW_H

#include "core/base/UIElement.h"
#include <functional>
#include <cmath>
#include <algorithm>

namespace UIEngine {

class ListView : public UIElement {
public:
    int totalItemCount = 1000;
    float itemHeight = 44.0f;
    int selectedIndex = -1;

    std::function<void(int index)> onItemSelected;

    ListView(const std::string &id = "", int itemCount = 1000)
        : UIElement(id), totalItemCount(itemCount) {
        m_bounds.width = 300.0f;
        m_bounds.height = 300.0f;
    }

    float getTotalContentHeight() const {
        return (float)totalItemCount * itemHeight;
    }

    // Virtualization Window Math: Calculates only indices visible on screen
    void getVisibleRange(float scrollOffsetY, int &outFirst, int &outLast) const {
        float startY = std::max(0.0f, -scrollOffsetY);
        outFirst = (int)(startY / itemHeight);
        outLast = (int)((startY + m_bounds.height) / itemHeight) + 1;
        outFirst = std::clamp(outFirst, 0, totalItemCount);
        outLast = std::clamp(outLast, 0, totalItemCount);
    }

    void selectItem(int index) {
        if (index >= 0 && index < totalItemCount) {
            selectedIndex = index;
            markRenderDirty();
            if (onItemSelected) onItemSelected(selectedIndex);
        }
    }
};

} // namespace UIEngine

#endif // ROOPM_LIST_VIEW_H
