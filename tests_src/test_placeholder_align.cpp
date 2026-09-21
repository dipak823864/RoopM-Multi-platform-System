#include <stdio.h>
#include <memory>
#include <vector>
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

using namespace UIEngine;

int main() {
    WebEngine engine;
    if (engine.loadWorkspace("workspace")) {
        engine.computeLayout(1920.0f, 0.0f);
        
        std::vector<std::shared_ptr<DOMNode>> stack = { engine.rootNode };
        while (!stack.empty()) {
            auto n = stack.back();
            stack.pop_back();
            if (n && n->tag == "p" && n->hasClass("inject-placeholder")) {
                printf("Found .inject-placeholder:\n");
                printf("  innerText: '%s'\n", n->innerText.c_str());
                printf("  textAlign: %s\n", n->style.textAlign == TextAlign::Center ? "Center" : (n->style.textAlign == TextAlign::Right ? "Right" : "Left"));
                printf("  layoutX: %.1f, layoutY: %.1f, layoutWidth: %.1f, layoutHeight: %.1f\n",
                       n->layoutX, n->layoutY, n->layoutWidth, n->layoutHeight);
                printf("  textRuns count: %zu\n", n->textRuns.size());
                break;
            }
            if (n) {
                for (const auto &c : n->children) stack.push_back(c);
            }
        }
    }
    return 0;
}
