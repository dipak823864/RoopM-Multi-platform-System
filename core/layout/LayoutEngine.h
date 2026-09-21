#ifndef ROOPM_LAYOUT_ENGINE_H
#define ROOPM_LAYOUT_ENGINE_H

#include "core/base/UIElement.h"
#include <vector>
#include <algorithm>

namespace UIEngine {

enum class FlexDirection {
    Row,
    Column
};

enum class MainAxisAlignment {
    Start,
    Center,
    End,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
};

enum class CrossAxisAlignment {
    Start,
    Center,
    End,
    Stretch
};

class FlexLayout : public UIElement {
public:
    FlexDirection direction = FlexDirection::Row;
    MainAxisAlignment mainAxisAlignment = MainAxisAlignment::Start;
    CrossAxisAlignment crossAxisAlignment = CrossAxisAlignment::Start;
    float gap = 8.0f;

    FlexLayout(const std::string &id = "", FlexDirection dir = FlexDirection::Row)
        : UIElement(id), direction(dir) {}

    Vec2 onLayout(const BoxConstraints &constraints) override {
        float currentMain = 0.0f;
        float maxCross = 0.0f;

        for (auto &child : m_children) {
            Vec2 childSize = child->onLayout(BoxConstraints::loose(constraints.maxWidth, constraints.maxHeight));
            if (direction == FlexDirection::Row) {
                child->setPosition(m_bounds.x + currentMain, m_bounds.y);
                currentMain += childSize.x + gap;
                maxCross = std::max(maxCross, childSize.y);
            } else {
                child->setPosition(m_bounds.x, m_bounds.y + currentMain);
                currentMain += childSize.y + gap;
                maxCross = std::max(maxCross, childSize.x);
            }
        }

        if (!m_children.empty()) currentMain -= gap; // Remove trailing gap

        m_bounds.width = (direction == FlexDirection::Row) ? currentMain : maxCross;
        m_bounds.height = (direction == FlexDirection::Row) ? maxCross : currentMain;
        m_isLayoutDirty = false;
        return {m_bounds.width, m_bounds.height};
    }
};

} // namespace UIEngine

#endif // ROOPM_LAYOUT_ENGINE_H
