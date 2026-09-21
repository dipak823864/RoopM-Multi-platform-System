#include <stdio.h>
#include <memory>
#include <vector>
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

using namespace UIEngine;

int main() {
    FILE *out = fopen("OUTPUT.TXT", "a");
    if (!out) out = stdout;

    fprintf(out, "===================================================================\n");
    fprintf(out, " [ROOPM ENGINE] SIDEBAR NAV ITEM & TEXT WRAP DIAGNOSTIC REPORT\n");
    fprintf(out, "===================================================================\n\n");

    WebEngine engine;
    if (engine.loadWorkspace("workspace")) {
        engine.computeLayout(1920.0f, 0.0f);

        fprintf(out, "Auditing Sidebar Navigation Items on Page 1:\n");
        fprintf(out, "-------------------------------------------------------------------\n");

        std::vector<std::shared_ptr<DOMNode>> stack = { engine.rootNode };
        int navCount = 0;
        while (!stack.empty()) {
            auto n = stack.back();
            stack.pop_back();

            if (n && n->hasClass("nav-item")) {
                navCount++;
                float textW = roopm_measure_text_ex(n->innerText.c_str(), n->style.fontSize, n->style.isBold);
                float availableContentW = n->layoutWidth - n->style.padLeft - n->style.padRight;

                fprintf(out, "Nav Item #%d:\n", navCount);
                fprintf(out, "  Tag: <%s>, Text: '%s'\n", n->tag.c_str(), n->innerText.c_str());
                fprintf(out, "  Layout Box: X=%.1f, Y=%.1f, Width=%.1f, Height=%.1f\n",
                        n->layoutX, n->layoutY, n->layoutWidth, n->layoutHeight);
                fprintf(out, "  Padding: Left=%.1f, Right=%.1f, Top=%.1f, Bottom=%.1f\n",
                        n->style.padLeft, n->style.padRight, n->style.padTop, n->style.padBottom);
                fprintf(out, "  Available Width for Text: %.1f px | Measured Text Width: %.1f px\n",
                        availableContentW, textW);
                fprintf(out, "  Wraps into Multiple Lines?: %s (Wrapped Lines Count: %zu)\n",
                        (textW > availableContentW) ? "YES 🚨 (OVERFLOWS WIDTH -> FORCES 2 LINES!)" : "NO ✅ (FITS SINGLE LINE)",
                        n->wrappedLines.size());
                fprintf(out, "-------------------------------------------------------------------\n");
            }

            if (n) {
                for (auto it = n->children.rbegin(); it != n->children.rend(); ++it) {
                    stack.push_back(*it);
                }
            }
        }
    }

    fprintf(out, "\n===================================================================\n");
    if (out != stdout) fclose(out);
    return 0;
}
