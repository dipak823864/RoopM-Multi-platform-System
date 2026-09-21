#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

#include "core/roopm.h"
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

int main() {
    printf("===================================================================\n");
    printf(" [ROOPM ENGINE] DEEP RUNTIME CSS TRANSFORM & HOVER DIAGNOSTICS\n");
    printf("===================================================================\n\n");

    UIEngine::WebEngine engine;
    bool ok = engine.loadWorkspace("workspace");
    if (!ok) {
        printf("[ERROR] Could not load workspace folder!\n");
        return 1;
    }
    printf("[1/4] Workspace Loaded Successfully.\n");

    // 1. Inspect Parsed CSS Rules for :hover and transform
    printf("\n[2/4] Inspecting Parsed CSS Rules:\n");
    int hoverRuleCount = 0;
    for (size_t i = 0; i < engine.cssParser.rules.size(); i++) {
        const auto &rule = engine.cssParser.rules[i];
        if (rule.isHover || rule.selector.find("hover") != std::string::npos || rule.selector.find("svg-tile") != std::string::npos) {
            hoverRuleCount++;
            printf("  Rule #%zu: Selector='%s' (isHover=%s, specificity=%d)\n", 
                   i, rule.selector.c_str(), rule.isHover ? "TRUE" : "FALSE", rule.specificity);
            for (const auto &decl : rule.declarations) {
                printf("     -> Prop: '%s' = '%s'\n", decl.property.c_str(), decl.value.c_str());
            }
        }
    }
    printf("  Total Matching Hover Rules Found: %d\n", hoverRuleCount);

    // 2. Compute 1st Layout Pass
    printf("\n[3/4] Running Yoga Layout Pass (1200x800 Viewport)...\n");
    engine.computeLayout(1200.0f, 0.0f);

    // 3. Search and Inspect all SVG Tile Nodes
    printf("\n[4/4] Searching for .svg-tile Nodes and Testing Hover Lift Mechanics:\n");
    
    std::function<void(const std::shared_ptr<UIEngine::DOMNode>&)> inspectNode = [&](const std::shared_ptr<UIEngine::DOMNode> &node) {
        if (!node) return;

        if (node->hasClass("svg-tile")) {
            printf("\n  ===============================================================\n");
            printf("  Found Node: tag='%s', id='%s'\n", node->tag.c_str(), node->id.c_str());
            printf("  Classes: ");
            for (const auto &c : node->classes) printf(".%s ", c.c_str());
            printf("\n");
            printf("  Layout Box: X=%.1f, Y=%.1f, W=%.1f, H=%.1f\n", 
                   node->layoutX, node->layoutY, node->layoutWidth, node->layoutHeight);
            printf("  Style Flags: hasHoverBg=%s (0x%08X), hasHoverTransform=%s (hoverTY=%.1f)\n",
                   node->style.hasHoverBg ? "TRUE" : "FALSE", node->style.hoverBackgroundColor,
                   node->style.hasHoverTransform ? "TRUE" : "FALSE", node->style.hoverTransformY);

            // Simulate Mouse Hover on Center of this Card
            float testX = node->layoutX + node->layoutWidth * 0.5f;
            float testY = node->layoutY + node->layoutHeight * 0.5f;
            printf("  Testing Simulated Mouse Pointer at (X=%.1f, Y=%.1f)...\n", testX, testY);

            // Step 1: Initial State (Mouse away)
            engine.update(0.016f, 0.0f, 0.0f);
            printf("    Frame 0 (Idle): isHovered=%s, animTY=%.3f\n", 
                   node->isHovered ? "TRUE" : "FALSE", node->animTransformY);

            // Step 2: Hover Mouse on Card for 10 consecutive frames
            for (int f = 1; f <= 10; f++) {
                engine.update(0.016f, testX, testY);
                printf("    Frame %2d (Hover): isHovered=%s, animTY=%.3f (actualY=%.1f)\n", 
                       f, node->isHovered ? "TRUE" : "FALSE", node->animTransformY, node->layoutY + node->animTransformY);
            }
        }

        for (const auto &child : node->children) {
            inspectNode(child);
        }
    };

    inspectNode(engine.rootNode);

    printf("\n===================================================================\n");
    printf(" [DIAGNOSTIC COMPLETED SUCCESSFULLY]\n");
    printf("===================================================================\n");
    return 0;
}
