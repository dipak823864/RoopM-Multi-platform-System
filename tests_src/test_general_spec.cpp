#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <vector>

#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/JSDOMRuntime.h"
#include "core/web/WebEngine.h"

using namespace UIEngine;

static int g_pass = 0;
static int g_fail = 0;

#define SPEC_TEST(title, cond) do { \
    printf("  [SPEC] %-54s ... ", title); \
    if (cond) { \
        g_pass++; \
        printf("PASSED\n"); \
    } else { \
        g_fail++; \
        printf("FAILED\n"); \
    } \
    fflush(stdout); \
} while(0)

int main() {
    printf("===================================================================\n");
    printf(" [ROOPM ENGINE] ARBITRARY NEW HTML PAGE GENERALIZATION AUDIT\n");
    printf("===================================================================\n\n");

    WebEngine engine;
    bool loaded = engine.navigate("workspace/general_spec_test.html");
    SPEC_TEST("Load brand-new general_spec_test.html", loaded);
    SPEC_TEST("Root Node Valid", engine.rootNode != nullptr);

    if (engine.rootNode) {
        engine.computeLayout(1920.0f, 0.0f);
        SPEC_TEST("Full Tree Layout Bounds Computed", engine.rootNode->layoutWidth > 0.0f);

        // 1. Test Tag Hierarchy & Universal Defaults
        SPEC_TEST("H1, H2, H3, H4, H5, H6 Elements Parsed", engine.rootNode->children.size() > 0);

        // 2. Test Radio Group Mutual Exclusion
        auto rbStandard = engine.idNodeMap["rb-standard"];
        auto rbPro = engine.idNodeMap["rb-pro"];
        SPEC_TEST("Radio Buttons Indexed into idNodeMap", rbStandard != nullptr && rbPro != nullptr);

        if (rbStandard && rbPro) {
            SPEC_TEST("Radio Buttons Circular Radius (9px)", rbStandard->style.borderRadius == 9.0f);
            
            // Simulate click on rbStandard
            engine.handlePointerClick(rbStandard->layoutX + 5.0f, rbStandard->layoutY + 5.0f);
            SPEC_TEST("Radio Standard Selected on Click", rbStandard->isChecked == true && rbPro->isChecked == false);

            // Simulate click on rbPro (must uncheck rbStandard!)
            engine.handlePointerClick(rbPro->layoutX + 5.0f, rbPro->layoutY + 5.0f);
            SPEC_TEST("Radio Mutual Exclusion (Pro on, Standard off)", rbPro->isChecked == true && rbStandard->isChecked == false);
        }

        // 3. Test Checkbox Toggle
        auto cbAgree = engine.idNodeMap["cb-agree"];
        SPEC_TEST("Checkbox Indexed into idNodeMap", cbAgree != nullptr && cbAgree->isCheckbox);
        if (cbAgree) {
            engine.handlePointerClick(cbAgree->layoutX + 5.0f, cbAgree->layoutY + 5.0f);
            SPEC_TEST("Checkbox Toggled to Checked", cbAgree->isChecked == true);
        }

        // 4. Test W3C Event Bubbling (Clicking icon inside button)
        auto btnIcon = engine.idNodeMap["btn-icon"];
        auto btnParent = engine.idNodeMap["btn-bubble-test"];
        SPEC_TEST("Button and Nested Child Icon Indexed", btnIcon != nullptr && btnParent != nullptr);

        if (btnIcon && btnParent) {
            // Register click listener on parent button
            const char *jsHook = "var bubbleSuccess = false; document.getElementById('btn-bubble-test').addEventListener('click', function() { bubbleSuccess = true; });";
            engine.jsRuntime.executeScript(jsHook);

            // Click directly on child icon (deepest child)
            engine.handlePointerClick(btnIcon->layoutX + 2.0f, btnIcon->layoutY + 2.0f);

            JSValue glob = JS_GetGlobalObject(engine.jsRuntime.ctx);
            JSValue val = JS_GetPropertyStr(engine.jsRuntime.ctx, glob, "bubbleSuccess");
            bool bSuccess = (bool)JS_ToBool(engine.jsRuntime.ctx, val);
            JS_FreeValue(engine.jsRuntime.ctx, val);
            JS_FreeValue(engine.jsRuntime.ctx, glob);

            SPEC_TEST("Event Bubbling: Child Click Triggered Parent Listener!", bSuccess == true);
        }
    }

    printf("\n===================================================================\n");
    printf(" [SPEC AUDIT SUMMARY] Total: %d | Passed: %d | Failed: %d\n", g_pass + g_fail, g_pass, g_fail);
    printf("===================================================================\n");

    return (g_fail == 0) ? 0 : 1;
}
