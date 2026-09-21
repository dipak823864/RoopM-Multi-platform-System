#include <stdio.h>
#include <memory>
#include <vector>
#include <sstream>
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

using namespace UIEngine;

int main() {
    FILE *out = fopen("OUTPUT.TXT", "w");
    if (!out) out = stdout;

    fprintf(out, "===================================================================\n");
    fprintf(out, " [ROOPM ENGINE] LIVE TEXT WRAP STEP-BY-STEP MATH AUDIT\n");
    fprintf(out, "===================================================================\n\n");

    WebEngine engine;
    if (engine.loadWorkspace("workspace")) {
        engine.computeLayout(1920.0f, 0.0f);

        std::vector<std::shared_ptr<DOMNode>> stack = { engine.rootNode };
        while (!stack.empty()) {
            auto n = stack.back();
            stack.pop_back();

            if (n && n->tag == "a" && n->hasClass("nav-item") && n->innerText.find("W3C Spec") != std::string::npos) {
                float maxW = n->layoutWidth - (n->style.padLeft + n->style.padRight);
                fprintf(out, "Target Node: <%s class='nav-item'>\n", n->tag.c_str());
                fprintf(out, "  innerText: '%s'\n", n->innerText.c_str());
                fprintf(out, "  layoutWidth: %.1f px | padLeft: %.1f | padRight: %.1f\n", n->layoutWidth, n->style.padLeft, n->style.padRight);
                fprintf(out, "  Calculated maxW (available width): %.1f px\n", maxW);
                fprintf(out, "  fontSize: %.1f px | isBold: %d\n\n", n->style.fontSize, n->style.isBold ? 1 : 0);

                fprintf(out, "--- Simulating Word-by-Word Accumulation ---\n");
                std::stringstream ss(n->innerText);
                std::string word;
                std::string currLine = "";
                float currW = 0.0f;
                int lineIdx = 0;

                while (ss >> word) {
                    float wordW = roopm_measure_text_ex(word.c_str(), n->style.fontSize, n->style.isBold);
                    float spaceW = roopm_measure_text_ex(" ", n->style.fontSize, n->style.isBold);

                    fprintf(out, "  Word '%s': width = %.1f px | currW before = %.1f px (total if added = %.1f px vs maxW = %.1f px)\n",
                            word.c_str(), wordW, currW, currW + wordW, maxW);

                    if (currW + wordW > maxW && !currLine.empty()) {
                        fprintf(out, "    🚨 BREAK TO NEW LINE! Finished Line [%d]: '%s' (width = %.1f px)\n", lineIdx++, currLine.c_str(), currW);
                        currLine = word;
                        currW = wordW;
                    } else {
                        if (!currLine.empty()) {
                            currLine += " " + word;
                            currW += spaceW + wordW;
                        } else {
                            currLine = word;
                            currW = wordW;
                        }
                    }
                }
                if (!currLine.empty()) {
                    fprintf(out, "    Finished Line [%d]: '%s' (width = %.1f px)\n", lineIdx++, currLine.c_str(), currW);
                }
                fprintf(out, "\n  TOTAL LINES PRODUCED: %d\n", lineIdx);
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
