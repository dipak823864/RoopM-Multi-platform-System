#ifndef ROOPM_DOCKING_HOST_H
#define ROOPM_DOCKING_HOST_H

#include "core/base/UIElement.h"
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace UIEngine {

enum class DockZone {
    None,
    Left,
    Right,
    Top,
    Bottom,
    Center
};

struct DockNode {
    std::string id;
    std::string title;
    Rect bounds{0, 0, 0, 0};
    float splitRatio = 0.5f;
    DockZone splitDirection = DockZone::None;
    bool isFloating = false;

    std::shared_ptr<DockNode> firstChild;
    std::shared_ptr<DockNode> secondChild;
    std::vector<std::string> tabbedPanelIds;

    bool isLeaf() const { return !firstChild && !secondChild; }
};

class DockingHost : public UIElement {
public:
    std::shared_ptr<DockNode> rootDockNode;

    DockingHost(const std::string &id = "") : UIElement(id) {
        rootDockNode = std::make_shared<DockNode>();
        rootDockNode->id = "root_dock";
    }

    DockZone getDropZoneAt(const Rect &targetRect, float px, float py) const {
        if (!targetRect.contains(px, py)) return DockZone::None;

        float edgeW = targetRect.width * 0.25f;
        float edgeH = targetRect.height * 0.25f;

        if (px < targetRect.x + edgeW) return DockZone::Left;
        if (px > targetRect.x + targetRect.width - edgeW) return DockZone::Right;
        if (py < targetRect.y + edgeH) return DockZone::Top;
        if (py > targetRect.y + targetRect.height - edgeH) return DockZone::Bottom;

        return DockZone::Center;
    }

    void recalculateNodeBounds(std::shared_ptr<DockNode> node, const Rect &allocated) {
        if (!node) return;
        node->bounds = allocated;

        if (!node->isLeaf()) {
            if (node->splitDirection == DockZone::Left || node->splitDirection == DockZone::Right) {
                float w1 = allocated.width * node->splitRatio;
                float w2 = allocated.width - w1;
                recalculateNodeBounds(node->firstChild, {allocated.x, allocated.y, w1, allocated.height});
                recalculateNodeBounds(node->secondChild, {allocated.x + w1, allocated.y, w2, allocated.height});
            } else if (node->splitDirection == DockZone::Top || node->splitDirection == DockZone::Bottom) {
                float h1 = allocated.height * node->splitRatio;
                float h2 = allocated.height - h1;
                recalculateNodeBounds(node->firstChild, {allocated.x, allocated.y, allocated.width, h1});
                recalculateNodeBounds(node->secondChild, {allocated.x, allocated.y + h1, allocated.width, h2});
            }
        }
    }
};

} // namespace UIEngine

#endif // ROOPM_DOCKING_HOST_H
