#include <stdio.h>
#include <memory>
#include <vector>
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"

using namespace UIEngine;

int main() {
    FILE *out = fopen("OUTPUT.TXT", "w");
    if (!out) out = stdout;

    fprintf(out, "===================================================================\n");
    fprintf(out, " [ROOPM ENGINE] FLAW #4 (W3C CSS SPECIFICITY SCOREBOARD) TEST\n");
    fprintf(out, "===================================================================\n\n");

    CSSParser parser;

    // 1. Check Specificity Scores
    int sUniversal = parser.calculateSpecificity("*");
    int sTag       = parser.calculateSpecificity("button");
    int sDescTag   = parser.calculateSpecificity("div > p");
    int sClass     = parser.calculateSpecificity(".btn");
    int sTagClass  = parser.calculateSpecificity("button.btn");
    int sMultiCls  = parser.calculateSpecificity(".sidebar .nav-item");
    int sId        = parser.calculateSpecificity("#main");
    int sIdClass   = parser.calculateSpecificity("#sidebar .nav-item");

    fprintf(out, "W3C Specificity Score Audit:\n");
    fprintf(out, "  '*'                     -> %d (Expected: 0)     %s\n", sUniversal, sUniversal == 0 ? "PASSED ✅" : "FAILED ❌");
    fprintf(out, "  'button'                -> %d (Expected: 1)     %s\n", sTag, sTag == 1 ? "PASSED ✅" : "FAILED ❌");
    fprintf(out, "  'div > p'               -> %d (Expected: 2)     %s\n", sDescTag, sDescTag == 2 ? "PASSED ✅" : "FAILED ❌");
    fprintf(out, "  '.btn'                  -> %d (Expected: 100)   %s\n", sClass, sClass == 100 ? "PASSED ✅" : "FAILED ❌");
    fprintf(out, "  'button.btn'            -> %d (Expected: 101)   %s\n", sTagClass, sTagClass == 101 ? "PASSED ✅" : "FAILED ❌");
    fprintf(out, "  '.sidebar .nav-item'    -> %d (Expected: 200)   %s\n", sMultiCls, sMultiCls == 200 ? "PASSED ✅" : "FAILED ❌");
    fprintf(out, "  '#main'                 -> %d (Expected: 10000) %s\n", sId, sId == 10000 ? "PASSED ✅" : "FAILED ❌");
    fprintf(out, "  '#sidebar .nav-item'    -> %d (Expected: 10100) %s\n", sIdClass, sIdClass == 10100 ? "PASSED ✅" : "FAILED ❌");

    // 2. Cascade Priority Test
    auto btn = std::make_shared<DOMNode>("button");
    btn->classes.push_back("primary-btn");
    btn->id = "cta-btn";

    std::string testCss = 
        "* { padding: 0px; }\n"
        "button { padding: 8px; }\n"
        ".primary-btn { padding: 16px; }\n"
        "#cta-btn { padding: 24px; }\n";

    CSSParser cascadeParser;
    cascadeParser.parse(testCss);
    cascadeParser.applyToNode(btn);

    fprintf(out, "\nCascade Override Audit on <button id='cta-btn' class='primary-btn'>:\n");
    fprintf(out, "  Final padLeft = %.1f px (Expected: 24.0 px from #cta-btn) %s\n",
            btn->style.padLeft, (btn->style.padLeft == 24.0f) ? "PASSED ✅" : "FAILED ❌");

    fprintf(out, "\n===================================================================\n");
    if (out != stdout) fclose(out);
    return 0;
}
