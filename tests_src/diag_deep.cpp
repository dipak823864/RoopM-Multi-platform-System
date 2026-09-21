#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <vector>
#include <map>
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
    FILE *out = fopen("OUTPUT.TXT", "w");
    if (!out) out = stdout;

    fprintf(out, "===================================================================\n");
    fprintf(out, " [ROOPM ENGINE] EXACT FORENSIC CLICK & TABLE DIAGNOSTIC REPORT\n");
    fprintf(out, "===================================================================\n\n");

    UIEngine::WebEngine engine;
    bool ok = engine.loadWorkspace("workspace");
    if (!ok) {
        fprintf(out, "[ERROR] Could not load workspace!\n");
        fclose(out);
        return 1;
    }

    // 1. Diagnose Like Buttons on Page 1
    engine.navigate("workspace/index.html");
    engine.computeLayout(1200.0f, 0.0f);

    fprintf(out, "[1] LIKE BUTTONS DIAGNOSIS (Page 1):\n");
    fprintf(out, "-------------------------------------------------------------------\n");
    
    std::vector<std::shared_ptr<UIEngine::DOMNode>> likeBtns;
    std::function<void(const std::shared_ptr<UIEngine::DOMNode>&)> findLike = [&](const std::shared_ptr<UIEngine::DOMNode> &n) {
        if (!n) return;
        if (n->hasClass("like-btn")) likeBtns.push_back(n);
        for (const auto &c : n->children) findLike(c);
    };
    findLike(engine.rootNode);

    fprintf(out, "Found %zu Like Buttons.\n", likeBtns.size());
    for (size_t i = 0; i < likeBtns.size(); i++) {
        auto btn = likeBtns[i];
        bool hasCb = (engine.jsRuntime.clickCallbacks.count(btn->id) > 0);
        fprintf(out, "  Button #%zu: id='%s', Box=[%.1f, %.1f, %.1f, %.1f], HasClickCallback=%s\n",
                i, btn->id.c_str(), btn->layoutX, btn->layoutY, btn->layoutWidth, btn->layoutHeight,
                hasCb ? "YES ✅" : "NO 🚨 (NOT REGISTERED!)");

        // Simulate click
        float cx = btn->layoutX + btn->layoutWidth * 0.5f;
        float cy = btn->layoutY + btn->layoutHeight * 0.5f;
        bool handled = engine.handlePointerClick(cx, cy);
        fprintf(out, "     -> Click at (%.1f, %.1f) Handled: %s | Has 'liked' class: %s\n",
                cx, cy, handled ? "YES" : "NO", btn->hasClass("liked") ? "YES ✅ (TOGGLED!)" : "NO 🚨 (LOCKED!)");
    }

    // 2. Diagnose Table Rows & Dividing Lines on Page 3
    fprintf(out, "\n[2] TABLE ROWS & DIVIDING LINES DIAGNOSIS (Page 3):\n");
    fprintf(out, "-------------------------------------------------------------------\n");

    engine.navigate("workspace/telemetry.html");
    engine.computeLayout(1200.0f, 0.0f);

    std::shared_ptr<UIEngine::DOMNode> tableNode = nullptr;
    std::function<void(const std::shared_ptr<UIEngine::DOMNode>&)> findTable = [&](const std::shared_ptr<UIEngine::DOMNode> &n) {
        if (!n) return;
        if (n->tag == "table" || n->hasClass("benchmark-table")) tableNode = n;
        for (const auto &c : n->children) findTable(c);
    };
    findTable(engine.rootNode);

    if (tableNode) {
        fprintf(out, "Table Node: id='%s', borderRadius=%.1f px, borderWidth=%.1f px, borderCol=0x%08X\n",
                tableNode->id.c_str(), tableNode->style.borderRadius, tableNode->style.borderWidth, tableNode->style.borderColor);

        std::function<void(const std::shared_ptr<UIEngine::DOMNode>&, int)> inspectRows = [&](const std::shared_ptr<UIEngine::DOMNode> &n, int idx) {
            if (!n) return;
            if (n->tag == "tr" || n->hasClass("table-header-row") || n->hasClass("table-data-row")) {
                fprintf(out, "  Row #%d (<%s>): id='%s', Y=%.1f, H=%.1f, bg=0x%08X, bbWidth=%.1f px, bbCol=0x%08X (Will Draw Line: %s)\n",
                        idx, n->tag.c_str(), n->id.c_str(), n->layoutY, n->layoutHeight, n->style.backgroundColor,
                        n->style.borderBottomWidth, n->style.borderBottomColor,
                        (n->style.borderBottomWidth > 0.0f && (n->style.borderBottomColor >> 24) != 0 ? "YES ✅" : "NO 🚨 (LINE MISSING!)"));
            }
            for (size_t i = 0; i < n->children.size(); i++) inspectRows(n->children[i], (int)i);
        };
        inspectRows(tableNode, 0);
    } else {
        fprintf(out, "🚨 Table not found!\n");
    }

    fprintf(out, "\n===================================================================\n");
    fclose(out);
    return 0;
}
