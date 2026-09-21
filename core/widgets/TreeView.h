#ifndef ROOPM_TREE_VIEW_H
#define ROOPM_TREE_VIEW_H

#include "core/base/UIElement.h"
#include <vector>
#include <string>

namespace UIEngine {

struct TreeNode {
    std::string label;
    bool isExpanded = true;
    bool isSelected = false;
    std::vector<TreeNode> children;
};

class TreeView : public UIElement {
public:
    TreeNode rootNode;
    float rowHeight = 32.0f;
    float indentWidth = 20.0f;

    TreeView(const std::string &id = "") : UIElement(id) {
        m_bounds.width = 260.0f;
        m_bounds.height = 300.0f;
    }

    int countVisibleNodes(const TreeNode &node) const {
        int count = 1;
        if (node.isExpanded) {
            for (const auto &child : node.children) {
                count += countVisibleNodes(child);
            }
        }
        return count;
    }
};

} // namespace UIEngine

#endif // ROOPM_TREE_VIEW_H
