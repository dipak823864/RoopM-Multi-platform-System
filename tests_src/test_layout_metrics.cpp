#include <stdio.h>
#include <memory>
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/WebEngine.h"

using namespace UIEngine;

int main() {
    WebEngine engine;

    // 1. Audit Button Height & Children in general_spec_test.html
    printf("[1] Auditing Button & Paragraph in general_spec_test.html\n");
    if (engine.navigate("workspace/general_spec_test.html")) {
        engine.computeLayout(1920.0f, 0.0f);
        auto btn = engine.idNodeMap["btn-bubble-test"];
        if (btn) {
            printf("  Button Layout: X=%.1f, Y=%.1f, W=%.1f, H=%.1f, PadTop=%.1f, PadBottom=%.1f, Children=%zu\n",
                   btn->layoutX, btn->layoutY, btn->layoutWidth, btn->layoutHeight, btn->style.padTop, btn->style.padBottom, btn->children.size());
            for (size_t i = 0; i < btn->children.size(); i++) {
                auto c = btn->children[i];
                printf("    Child %zu (%s): text='%s', X=%.1f, Y=%.1f, W=%.1f, H=%.1f\n",
                       i, c->tag.c_str(), c->innerText.c_str(), c->layoutX, c->layoutY, c->layoutWidth, c->layoutHeight);
            }
        }
    }

    // 2. Audit Fleet Page 2 Grid Widths in fleet.html
    printf("\n[2] Auditing Card Widths & Wrapping in fleet.html\n");
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
