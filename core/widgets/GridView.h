#ifndef ROOPM_GRID_VIEW_H
#define ROOPM_GRID_VIEW_H

#include "core/base/UIElement.h"
#include <cmath>
#include <algorithm>

namespace UIEngine {

class GridView : public UIElement {
public:
    int totalItemCount = 120;
    float itemWidth = 100.0f;
    float itemHeight = 90.0f;
    float spacing = 12.0f;
    int selectedIndex = -1;

    GridView(const std::string &id = "", int count = 120)
        : UIElement(id), totalItemCount(count) {
        m_bounds.width = 320.0f;
        m_bounds.height = 300.0f;
    }

    int getColumnCount() const {
        int cols = (int)((m_bounds.width + spacing) / (itemWidth + spacing));
        return std::max(1, cols);
    }

    int getRowCount() const {
        int cols = getColumnCount();
        return (totalItemCount + cols - 1) / cols;
    }

    float getTotalContentHeight() const {
        int rows = getRowCount();
        return (float)rows * (itemHeight + spacing) - spacing;
    }

    Rect getItemRect(int index, float scrollOffsetY) const {
        int cols = getColumnCount();
        int row = index / cols;
        int col = index % cols;
        float x = m_bounds.x + (float)col * (itemWidth + spacing);
        float y = m_bounds.y + (float)row * (itemHeight + spacing) + scrollOffsetY;
        return {x, y, itemWidth, itemHeight};
    }
};

} // namespace UIEngine

#endif // ROOPM_GRID_VIEW_H
