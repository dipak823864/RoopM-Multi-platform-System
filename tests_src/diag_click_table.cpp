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
    fprintf(out, " [ROOPM ENGINE] LIVE CLICK & TABLE FORENSIC DIAGNOSTIC\n");
    fprintf(out, "===================================================================\n\n");

    UIEngine::WebEngine engine;
    bool ok = engine.loadWorkspace("workspace");
    if (!ok) {
        fprintf(out, "[ERROR] Could not load workspace!\n");
        fclose(out);
        return 1;
    }

    // -------------------------------------------------------------
    // PART 1: DIAGNOSE LIKE BUTTON CLICK DISPATCH
    // -------------------------------------------------------------
    fprintf(out, "[1] DIAGNOSING LIKE BUTTON CLICK PIPELINE (Page 1):\n");
    fprintf(out, "-------------------------------------------------------------------\n");
    
    engine.navigate("workspace/index.html");
    engine.computeLayout(1200.0f, 0.0f);

    std::vector<std::shared_ptr<UIEngine::DOMNode>> likeBtns;
    std::function<void(const std::shared_ptr<UIEngine::DOMNode>&)> findLikeBtns = [&](const std::shared_ptr<UIEngine::DOMNode> &node) {
        if (!node) return;
        if (node->hasClass("like-btn")) likeBtns.push_back(node);
        for (const auto &c : node->children) findLikeBtns(c);
    };
    findLikeBtns(engine.rootNode);

    fprintf(out, "Found %zu Like Buttons on Page 1.\n", likeBtns.size());
    for (size_t i = 0; i < likeBtns.size(); i++) {
        auto btn = likeBtns[i];
        bool hasCallback = (engine.jsRuntime.clickCallbacks.count(btn->id) > 0);
        fprintf(out, "  Button #%zu: id='%s', class='", i, btn->id.c_str());
        for (const auto &c : btn->classes) fprintf(out, ".%s ", c.c_str());
        fprintf(out, "', Box=[%.1f, %.1f, %.1f, %.1f], ClickCallbackRegistered=%s\n",
                btn->layoutX, btn->layoutY, btn->layoutWidth, btn->layoutHeight,
                hasCallback ? "YES ✅" : "NO 🚨 (LISTENER MISSING!)");

        // Simulate Mouse Click on Center of this Like Button
        float clickX = btn->layoutX + btn->layoutWidth * 0.5f;
        float clickY = btn->layoutY + btn->layoutHeight * 0.5f;
        fprintf(out, "     -> Simulating Click at (%.1f, %.1f)...\n", clickX, clickY);
        bool handled = engine.handlePointerClick(clickX, clickY);
        fprintf(out, "     -> Click Handled: %s | Has 'liked' class after click: %s\n",
                handled ? "YES" : "NO", btn->hasClass("liked") ? "YES ✅ (TOGGLED!)" : "NO 🚨 (FAILED TO TOGGLE!)");
    }

    // -------------------------------------------------------------
    // PART 2: DIAGNOSE TABLE BORDERS & CORNERS (Page 3)
    // -------------------------------------------------------------
    fprintf(out, "\n[2] DIAGNOSING TABLE BORDERS & CORNERS (Page 3):\n");
    fprintf(out, "-------------------------------------------------------------------\n");
    
    engine.navigate("workspace/telemetry.html");
    engine.computeLayout(1200.0f, 0.0f);

    std::shared_ptr<UIEngine::DOMNode> tableNode = nullptr;
    std::function<void(const std::shared_ptr<UIEngine::DOMNode>&)> findTable = [&](const std::shared_ptr<UIEngine::DOMNode> &node) {
        if (!node) return;
        if (node->tag == "table" || node->hasClass("benchmark-table")) tableNode = node;
        for (const auto &c : node->children) findTable(c);
    };
    findTable(engine.rootNode);

    if (tableNode) {
        fprintf(out, "Table Found: id='%s', class='", tableNode->id.c_str());
        for (const auto &c : tableNode->classes) fprintf(out, ".%s ", c.c_str());
        fprintf(out, "'\n");
        fprintf(out, "  -> Box: X=%.1f, Y=%.1f, W=%.1f, H=%.1f\n", tableNode->layoutX, tableNode->layoutY, tableNode->layoutWidth, tableNode->layoutHeight);
        fprintf(out, "  -> Border: width=%.1f px, color=0x%08X, borderRadius=%.1f px (Is Rounded: %s)\n",
                tableNode->style.borderWidth, tableNode->style.borderColor, tableNode->style.borderRadius,
                (tableNode->style.borderRadius > 0 ? "YES ✅" : "NO 🚨 (SQUARE!)"));

        // Inspect all rows in the table
        std::function<void(const std::shared_ptr<UIEngine::DOMNode>&, int)> dumpRows = [&](const std::shared_ptr<UIEngine::DOMNode> &node, int idx) {
            if (!node) return;
            if (node->tag == "tr" || node->hasClass("table-header-row") || node->hasClass("table-data-row")) {
                fprintf(out, "  Row #%d (<%s>): id='%s', bg=0x%08X, borderBottomWidth=%.1f px, borderBottomCol=0x%08X (Will Draw Line: %s)\n",
                        idx, node->tag.c_str(), node->id.c_str(), node->style.backgroundColor,
                        node->style.borderBottomWidth, node->style.borderBottomColor,
                        (node->style.borderBottomWidth > 0 && (node->style.borderBottomColor >> 24) != 0 ? "YES ✅" : "NO 🚨 (MISSING LINE!)"));
            }
            for (size_t i = 0; i < node->children.size(); i++) {
                dumpRows(node->children[i], (int)i);
            }
        };
        dumpRows(tableNode, 0);
    } else {
        fprintf(out, "🚨 [ERROR]: Table node not found on Page 3!\n");
    }

    fprintf(out, "\n===================================================================\n");
    fprintf(out, " [DIAGNOSTIC COMPLETED]\n");
    fprintf(out, "===================================================================\n");

    fclose(out);
    return 0;
}
