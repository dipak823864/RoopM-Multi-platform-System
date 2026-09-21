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
    fprintf(out, " [ROOPM ENGINE] EXACT RENDER PARAMETERS AUDIT\n");
    fprintf(out, "===================================================================\n\n");

    WebEngine engine;
    if (engine.loadWorkspace("workspace")) {
        engine.computeLayout(1920.0f, 0.0f);

        std::vector<std::shared_ptr<DOMNode>> stack = { engine.rootNode };
        while (!stack.empty()) {
            auto n = stack.back();
            stack.pop_back();

            if (n && n->tag == "a" && n->hasClass("nav-item") && n->innerText.find("W3C Spec") != std::string::npos) {
                fprintf(out, "Target Node: <%s class='nav-item'>\n", n->tag.c_str());
                fprintf(out, "  innerText: '%s'\n", n->innerText.c_str());
                fprintf(out, "  textRuns.size(): %zu\n", n->textRuns.size());
                fprintf(out, "  fontSize: %.1f px\n", n->style.fontSize);
                fprintf(out, "  lineHeight: %.1f px (🚨 If 0, all lines draw on top of each other!)\n", n->style.lineHeight);
                fprintf(out, "  layoutWidth: %.1f px, layoutHeight: %.1f px\n\n", n->layoutWidth, n->layoutHeight);

                // Run wrapText and simulate lines rendering
                std::stringstream ss(n->innerText);
                std::string word, currLine;
                float currW = 0, maxW = n->layoutWidth - (n->style.padLeft + n->style.padRight);
                std::vector<std::string> lines;
                while (ss >> word) {
                    float w = roopm_measure_text_ex(word.c_str(), n->style.fontSize, n->style.isBold);
                    float sp = roopm_measure_text_ex(" ", n->style.fontSize, n->style.isBold);
                    if (currW + w > maxW && !currLine.empty()) {
                        lines.push_back(currLine);
                        currLine = word;
                        currW = w;
                    } else {
                        if (!currLine.empty()) { currLine += " " + word; currW += sp + w; }
                        else { currLine = word; currW = w; }
                    }
                }
                if (!currLine.empty()) lines.push_back(currLine);

                float effLineH = n->style.lineHeight > 0 ? n->style.lineHeight : (n->style.fontSize * 1.35f);
                float totalH = (float)lines.size() * effLineH;
                float startY = (n->layoutHeight - totalH) * 0.5f;

                fprintf(out, "Render Simulation (Total Lines = %zu):\n", lines.size());
                for (size_t i = 0; i < lines.size(); i++) {
                    float lineY = startY + i * n->style.lineHeight;
                    fprintf(out, "  Line [%zu]: '%s' -> drawn at lineY = %.1f (using raw lineHeight = %.1f)\n",
                            i, lines[i].c_str(), lineY, n->style.lineHeight);
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
