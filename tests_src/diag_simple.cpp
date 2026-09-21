#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>
#include <string>

#include "core/roopm.h"
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

// Simple iterative node search without std::function recursion
void collectAllNodes(const std::shared_ptr<UIEngine::DOMNode> &root, std::vector<std::shared_ptr<UIEngine::DOMNode>> &outList) {
    if (!root) return;
    std::vector<std::shared_ptr<UIEngine::DOMNode>> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        auto curr = stack.back();
        stack.pop_back();
        outList.push_back(curr);
        for (auto it = curr->children.rbegin(); it != curr->children.rend(); ++it) {
            stack.push_back(*it);
        }
    }
}

int main() {
    FILE *out = fopen("OUTPUT.TXT", "w");
    if (!out) out = stdout;

    fprintf(out, "===================================================================\n");
    fprintf(out, " [ROOPM ENGINE] EXACT ZERO-ERROR PIXEL & TABLE DIAGNOSTIC REPORT\n");
    fprintf(out, "===================================================================\n\n");

    UIEngine::WebEngine engine;
    bool ok = engine.loadWorkspace("workspace");
    if (!ok) {
        fprintf(out, "🚨 [ERROR]: Could not load workspace!\n");
        if (out != stdout) fclose(out);
        return 1;
    }

    // 1. Inspect Like Buttons on Page 1
    engine.navigate("workspace/index.html");
    engine.computeLayout(1200.0f, 0.0f);

    std::vector<std::shared_ptr<UIEngine::DOMNode>> allNodesP1;
    collectAllNodes(engine.rootNode, allNodesP1);

    fprintf(out, "[1] LIKE BUTTONS DIAGNOSIS (Page 1):\n");
    fprintf(out, "-------------------------------------------------------------------\n");
    int likeCount = 0;
    for (auto &node : allNodesP1) {
        if (node->hasClass("like-btn")) {
            likeCount++;
            bool hasCb = (engine.jsRuntime.clickCallbacks.count(node->id) > 0);
            fprintf(out, "  Like Button #%d: id='%s', Box=[%.1f, %.1f, %.1f, %.1f], CallbackRegistered=%s\n",
                    likeCount, node->id.c_str(), node->layoutX, node->layoutY, node->layoutWidth, node->layoutHeight,
                    hasCb ? "YES ✅" : "NO 🚨 (EMPTY ID / UNREGISTERED!)");

            // Simulate Click
            float cx = node->layoutX + node->layoutWidth * 0.5f;
            float cy = node->layoutY + node->layoutHeight * 0.5f;
            bool handled = engine.handlePointerClick(cx, cy);
            fprintf(out, "     -> Click at (%.1f, %.1f): Handled=%s, Has 'liked' class=%s\n",
                    cx, cy, handled ? "YES" : "NO", node->hasClass("liked") ? "YES ✅" : "NO 🚨");
        }
    }

    // 2. Inspect Table on Page 3
    engine.navigate("workspace/telemetry.html");
    engine.computeLayout(1200.0f, 0.0f);

    std::vector<std::shared_ptr<UIEngine::DOMNode>> allNodesP3;
    collectAllNodes(engine.rootNode, allNodesP3);

    fprintf(out, "\n[2] TABLE GEOMETRY & BORDER DIAGNOSIS (Page 3):\n");
    fprintf(out, "-------------------------------------------------------------------\n");
    for (auto &node : allNodesP3) {
        if (node->hasClass("benchmark-table") || node->tag == "table") {
            fprintf(out, "Table Element: tag=<%s>, id='%s', Box=[%.1f, %.1f, %.1f, %.1f]\n",
                    node->tag.c_str(), node->id.c_str(), node->layoutX, node->layoutY, node->layoutWidth, node->layoutHeight);
            fprintf(out, "  -> Style: borderWidth=%.1f px, borderColor=0x%08X, borderRadius=%.1f px\n",
                    node->style.borderWidth, node->style.borderColor, node->style.borderRadius);
        }
        if (node->tag == "tr") {
            fprintf(out, "  Row Element: tag=<tr class='");
            for (const auto &c : node->classes) fprintf(out, ".%s ", c.c_str());
            fprintf(out, "'>, Box=[%.1f, %.1f, %.1f, %.1f], bg=0x%08X, bbWidth=%.1f, bbColor=0x%08X\n",
                    node->layoutX, node->layoutY, node->layoutWidth, node->layoutHeight,
                    node->style.backgroundColor, node->style.borderBottomWidth, node->style.borderBottomColor);
        }
    }

    fprintf(out, "\n===================================================================\n");
    fprintf(out, " [DIAGNOSTIC COMPLETED SUCCESSFULLY]\n");
    fprintf(out, "===================================================================\n");

    if (out != stdout) fclose(out);
    return 0;
}
