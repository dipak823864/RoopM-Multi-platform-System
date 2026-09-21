#include <stdio.h>
#include <memory>
#include <vector>
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

using namespace UIEngine;

int main() {
    WebEngine engine;

    // 1. Check Paragraph in general_spec_test.html
    printf("--- [1] Paragraph Inspection in general_spec_test.html ---\n");
    if (engine.navigate("workspace/general_spec_test.html")) {
        engine.computeLayout(1920.0f, 0.0f);
        
        std::vector<std::shared_ptr<DOMNode>> stack = { engine.rootNode };
        while (!stack.empty()) {
            auto n = stack.back();
            stack.pop_back();
            if (n && n->tag == "p" && n->innerText.find("Standard paragraph") != std::string::npos) {
                printf("  Paragraph innerText: '%s'\n", n->innerText.c_str());
                printf("  Paragraph children count: %zu\n", n->children.size());
                for (size_t i = 0; i < n->children.size(); i++) {
                    auto c = n->children[i];
                    printf("    Child %zu (%s): text='%s', isBold=%d\n",
                           i, c->tag.c_str(), c->innerText.c_str(), c->style.isBold);
                }
                break;
            }
            if (n) {
                for (const auto &c : n->children) stack.push_back(c);
            }
        }
    }

    // 2. Check Fleet Page 2 Grid & Cards
    printf("\n--- [2] Fleet Page 2 Grid Widths & Wrapping ---\n");
    if (engine.navigate("workspace/fleet.html")) {
        engine.computeLayout(1920.0f, 0.0f);
        const char *cardIds[] = { "card-centodieci", "card-chiron", "card-cockpit", "card-wheel" };
        for (int i = 0; i < 4; i++) {
            auto card = engine.idNodeMap[cardIds[i]];
            if (card) {
                printf("  Card %d (%s): X=%.1f, Y=%.1f, W=%.1f, H=%.1f\n",
                       i + 1, cardIds[i], card->layoutX, card->layoutY, card->layoutWidth, card->layoutHeight);
            }
        }
    }

    return 0;
}
