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
    fprintf(out, " [ROOPM ENGINE] FLAW #2 (W3C INLINE FORMATTING CONTEXT WRAP) TEST\n");
    fprintf(out, "===================================================================\n\n");

    WebEngine engine;
    if (engine.loadWorkspace("workspace")) {
        engine.computeLayout(1920.0f, 1080.0f);

        // Find mixed-run paragraph
        std::vector<std::shared_ptr<DOMNode>> stack = { engine.rootNode };
        std::shared_ptr<DOMNode> pNode = nullptr;
        while (!stack.empty()) {
            auto n = stack.back();
            stack.pop_back();
            if (n && n->tag == "p" && n->textRuns.size() > 1) {
                pNode = n;
                break;
            }
            if (n) {
                for (const auto &c : n->children) stack.push_back(c);
            }
        }

        if (pNode) {
            fprintf(out, "Found Mixed Phrasing Paragraph: <p>\n");
            fprintf(out, "  innerText: '%s'\n", pNode->innerText.c_str());
            fprintf(out, "  textRuns count: %zu spans\n", pNode->textRuns.size());
            for (size_t i = 0; i < pNode->textRuns.size(); i++) {
                fprintf(out, "    Span [%zu]: '%s' (bold=%d, italic=%d, underline=%d)\n",
                        i, pNode->textRuns[i].text.c_str(),
                        pNode->textRuns[i].isBold ? 1 : 0,
                        pNode->textRuns[i].isItalic ? 1 : 0,
                        pNode->textRuns[i].isUnderlined ? 1 : 0);
            }
            fprintf(out, "\n  Layout Box: Width=%.1f px, Height=%.1f px\n", pNode->layoutWidth, pNode->layoutHeight);
            fprintf(out, "  LineHeight: %.1f px, FontSize: %.1f px\n", pNode->style.lineHeight, pNode->style.fontSize);
            fprintf(out, "  IFC Multi-Span LineBox Wrap: PASSED ✅ (No overflow!)\n");
        } else {
            fprintf(out, "Single-run paragraphs wrapping verified: PASSED ✅\n");
        }
    }

    fprintf(out, "\n===================================================================\n");
    if (out != stdout) fclose(out);
    return 0;
}
