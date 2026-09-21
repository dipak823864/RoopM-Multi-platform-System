#include <stdio.h>
#include <memory>
#include <vector>
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

using namespace UIEngine;

int main() {
    FILE *out = fopen("OUTPUT.TXT", "w");
    if (!out) out = stdout;

    fprintf(out, "===================================================================\n");
    fprintf(out, " [ROOPM ENGINE] EXACT LIVE WEBENGINE::RENDER AUDIT\n");
    fprintf(out, "===================================================================\n\n");

    WebEngine engine;
    if (engine.loadWorkspace("workspace")) {
        engine.computeLayout(1920.0f, 1080.0f);

        std::vector<std::shared_ptr<DOMNode>> stack = { engine.rootNode };
        while (!stack.empty()) {
            auto n = stack.back();
            stack.pop_back();

            if (n && n->tag == "a" && n->hasClass("nav-item") && n->innerText.find("W3C Spec") != std::string::npos) {
                fprintf(out, "Found Target Node in Workspace:\n");
                fprintf(out, "  Tag: <%s class='%s'>\n", n->tag.c_str(), n->classes.empty() ? "" : n->classes[0].c_str());
                fprintf(out, "  innerText: '%s'\n", n->innerText.c_str());
                fprintf(out, "  node->layoutWidth = %.1f px\n", n->layoutWidth);
                fprintf(out, "  node->layoutHeight = %.1f px\n", n->layoutHeight);
                fprintf(out, "  node->style.padLeft = %.1f, style.padRight = %.1f\n", n->style.padLeft, n->style.padRight);
                fprintf(out, "  Current wrappedLines.size() = %zu\n", n->wrappedLines.size());
                for (size_t i = 0; i < n->wrappedLines.size(); i++) {
                    fprintf(out, "    Line [%zu]: '%s'\n", i, n->wrappedLines[i].c_str());
                }
                break;
            }

            if (n) {
                for (const auto &c : n->children) stack.push_back(c);
            }
        }
    }

    fprintf(out, "\n===================================================================\n");
    if (out != stdout) fclose(out);
    return 0;
}
