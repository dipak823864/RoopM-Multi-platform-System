#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

#include "core/roopm.h"
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

int main() {
    FILE *out = fopen("OUTPUT.TXT", "w");
    if (!out) out = stdout;

    fprintf(out, "===================================================================\n");
    fprintf(out, " [ROOPM ENGINE] DEEP FORENSIC TABLE & CORNER DIAGNOSTIC TRACE\n");
    fprintf(out, "===================================================================\n\n");

    UIEngine::WebEngine engine;
    bool ok = engine.loadWorkspace("workspace");
    if (!ok) {
        fprintf(out, "[ERROR] Could not load workspace!\n");
        fclose(out);
        return 1;
    }

    engine.navigate("workspace/telemetry.html");
    engine.computeLayout(1200.0f, 0.0f);

    fprintf(out, "[1] FULL DOM TREE & COMPUTED STYLES FOR TABLE:\n");
    fprintf(out, "-------------------------------------------------------------------\n");

    std::function<void(const std::shared_ptr<UIEngine::DOMNode>&, int)> dumpTree = [&](const std::shared_ptr<UIEngine::DOMNode> &node, int depth) {
        if (!node) return;
        
        std::string indent(depth * 2, ' ');
        fprintf(out, "%sTag: <%s> id='%s' class='", indent.c_str(), node->tag.c_str(), node->id.c_str());
        for (const auto &c : node->classes) fprintf(out, ".%s ", c.c_str());
        fprintf(out, "'\n");
        fprintf(out, "%s  -> Layout Box: X=%.1f, Y=%.1f, W=%.1f, H=%.1f\n", indent.c_str(), node->layoutX, node->layoutY, node->layoutWidth, node->layoutHeight);
        fprintf(out, "%s  -> Style: bg=0x%08X, border=0x%08X (w=%.1f), radius=%.1f, pad=[%.1f, %.1f, %.1f, %.1f], text='%s'\n",
                indent.c_str(), node->style.backgroundColor, node->style.borderColor, node->style.borderWidth,
                node->style.borderRadius, node->style.padTop, node->style.padRight, node->style.padBottom, node->style.padLeft,
                node->innerText.c_str());

        for (const auto &child : node->children) {
            dumpTree(child, depth + 1);
        }
    };

    dumpTree(engine.rootNode, 0);

    fprintf(out, "\n[2] COLUMN BOUNDARY GAP ANALYSIS:\n");
    fprintf(out, "-------------------------------------------------------------------\n");
    
    // Search for all table rows and check adjacent cell boundaries
    std::function<void(const std::shared_ptr<UIEngine::DOMNode>&)> checkRowGaps = [&](const std::shared_ptr<UIEngine::DOMNode> &node) {
        if (!node) return;
        if (node->tag == "tr" || node->hasClass("table-header-row") || node->hasClass("table-data-row")) {
            fprintf(out, "\nRow Tag: <%s> (W=%.1f, H=%.1f at X=%.1f, Y=%.1f):\n", 
                    node->tag.c_str(), node->layoutWidth, node->layoutHeight, node->layoutX, node->layoutY);
            for (size_t i = 0; i < node->children.size(); i++) {
                auto cell = node->children[i];
                float cellRight = cell->layoutX + cell->layoutWidth;
                fprintf(out, "   Cell #%zu (<%s>): Left=%.1f, Right=%.1f, Width=%.1f, bg=0x%08X, text='%s'\n",
                        i, cell->tag.c_str(), cell->layoutX, cellRight, cell->layoutWidth, cell->style.backgroundColor, cell->innerText.c_str());
                if (i + 1 < node->children.size()) {
                    auto nextCell = node->children[i + 1];
                    float gap = nextCell->layoutX - cellRight;
                    fprintf(out, "      ==> GAP to next cell: %.2f px %s\n", gap, (fabs(gap) > 0.01f ? "🚨 [GAP DETECTED!]" : "✅ [FLUSH]"));
                }
            }
        }
        for (const auto &child : node->children) checkRowGaps(child);
    };

    checkRowGaps(engine.rootNode);

    fprintf(out, "\n===================================================================\n");
    fprintf(out, " [DIAGNOSTIC DATA CAPTURED SUCCESSFULLY]\n");
    fprintf(out, "===================================================================\n");
    
    fclose(out);
    return 0;
}
