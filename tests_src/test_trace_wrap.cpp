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
    fprintf(out, " [ROOPM ENGINE] LIVE TEXT WRAP EXECUTION TRACE\n");
    fprintf(out, "===================================================================\n\n");

    WebEngine engine;
    if (engine.loadWorkspace("workspace")) {
        engine.computeLayout(1920.0f, 0.0f);

        std::vector<std::shared_ptr<DOMNode>> stack = { engine.rootNode };
        while (!stack.empty()) {
            auto n = stack.back();
            stack.pop_back();

            if (n && n->tag == "a" && n->hasClass("nav-item") && n->innerText.find("W3C Spec") != std::string::npos) {
                fprintf(out, "Target Node Found: <%s class='%s'>\n", n->tag.c_str(), n->classes.empty() ? "" : n->classes[0].c_str());
                fprintf(out, "  innerText: '%s'\n", n->innerText.c_str());
                fprintf(out, "  layoutX=%.1f, layoutY=%.1f, layoutWidth=%.1f, layoutHeight=%.1f\n",
                        n->layoutX, n->layoutY, n->layoutWidth, n->layoutHeight);
                fprintf(out, "  style.padLeft=%.1f, style.padRight=%.1f\n", n->style.padLeft, n->style.padRight);
                fprintf(out, "  textRuns.size() = %zu\n", n->textRuns.size());

                // Run wrapText explicitly and inspect
                engine.wrapText(n);
                fprintf(out, "  After wrapText: wrappedLines.size() = %zu\n", n->wrappedLines.size());
                for (size_t i = 0; i < n->wrappedLines.size(); i++) {
                    float w = roopm_measure_text_ex(n->wrappedLines[i].c_str(), n->style.fontSize, n->style.isBold);
                    fprintf(out, "    Line [%zu]: '%s' (measured width = %.1f px)\n", i, n->wrappedLines[i].c_str(), w);
                }

                float maxW = n->layoutWidth - (n->style.padLeft + n->style.padRight);
                fprintf(out, "  Available Width for Text: %.1f px\n", maxW);
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
