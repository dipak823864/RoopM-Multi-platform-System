#include <stdio.h>
#include <memory>
#include <vector>
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

using namespace UIEngine;

void printHierarchy(FILE *out, const std::shared_ptr<DOMNode> &node, int depth) {
    if (!node) return;

    std::string indent(depth * 2, ' ');
    std::string clsStr = node->classes.empty() ? "" : ("." + node->classes[0]);
    fprintf(out, "%s<%s%s>: layoutWidth=%.1f, layoutHeight=%.1f (padL=%.1f, padR=%.1f)\n",
            indent.c_str(), node->tag.c_str(), clsStr.c_str(),
            node->layoutWidth, node->layoutHeight, node->style.padLeft, node->style.padRight);

    for (const auto &c : node->children) {
        // Only recurse into sidebar tree
        if (c->hasClass("sidebar") || c->hasClass("nav-menu") || c->tag == "body" || c->hasClass("app-container") || depth > 0) {
            printHierarchy(out, c, depth + 1);
        }
    }
}

int main() {
    FILE *out = fopen("OUTPUT.TXT", "w");
    if (!out) out = stdout;

    fprintf(out, "===================================================================\n");
    fprintf(out, " [ROOPM ENGINE] SIDEBAR & NAV HIERARCHY WIDTH TRACE\n");
    fprintf(out, "===================================================================\n\n");

    WebEngine engine;
    if (engine.loadWorkspace("workspace")) {
        engine.computeLayout(1920.0f, 1080.0f);
        printHierarchy(out, engine.rootNode, 0);
    }

    fprintf(out, "\n===================================================================\n");
    if (out != stdout) fclose(out);
    return 0;
}
