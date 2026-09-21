#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <vector>
#include <chrono>

#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"
#include "core/web/HTMLDOMBuilder.h"
#include "core/web/JSDOMRuntime.h"
#include "core/web/WebEngine.h"

using namespace UIEngine;

static int g_pass = 0;
static int g_fail = 0;

#define AUDIT_TEST(title, cond) do { \
    printf("  [AUDIT] %-52s ... ", title); \
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
    printf(" [ROOPM ENGINE] LIVE WORKSPACE COMPLETE AUDIT & VERIFICATION\n");
    printf("===================================================================\n\n");

    WebEngine engine;

    // 1. Audit Page 1: workspace/index.html
    printf("[1] Auditing Page 1: workspace/index.html (Music Studio + Canvas)\n");
    bool p1Loaded = engine.loadWorkspace("workspace");
    AUDIT_TEST("Load workspace/index.html & style.css", p1Loaded);
    AUDIT_TEST("Root Node Created Successfully", engine.rootNode != nullptr);

    if (engine.rootNode) {
        engine.computeLayout(1920.0f, 0.0f);
        AUDIT_TEST("Layout Calculated (Width > 0 && Height > 0)", engine.rootNode->layoutWidth > 0.0f && engine.documentHeight > 0.0f);
        AUDIT_TEST("Search Bar Input Node Indexed", engine.idNodeMap.count("search-bar") > 0);
        AUDIT_TEST("Canvas Waveform Node Indexed", engine.idNodeMap.count("waveform-canvas") > 0);
        AUDIT_TEST("Live Timer Badge Node Indexed", engine.idNodeMap.count("live-timer") > 0);

        auto canvasNode = engine.idNodeMap["waveform-canvas"];
        AUDIT_TEST("Canvas Node Allocated Buffer (480x90)", canvasNode && canvasNode->isCanvas && canvasNode->canvasW == 480 && canvasNode->canvasH == 90);
    }

    // 2. Audit Page 2: workspace/fleet.html
    printf("\n[2] Auditing Page 2: workspace/fleet.html (Space Fleet 4-Card Grid)\n");
    bool p2Loaded = engine.navigate("workspace/fleet.html");
    AUDIT_TEST("Navigate to workspace/fleet.html", p2Loaded);
    if (engine.rootNode) {
        engine.computeLayout(1920.0f, 0.0f);
        AUDIT_TEST("Page 2 Layout Calculated", engine.rootNode->layoutWidth > 0.0f);
        AUDIT_TEST("Card 1 (Centodieci) Indexed", engine.idNodeMap.count("card-centodieci") > 0);
        AUDIT_TEST("Card 2 (Chiron) Indexed", engine.idNodeMap.count("card-chiron") > 0);
    }

    // 3. Audit Page 3: workspace/telemetry.html
    printf("\n[3] Auditing Page 3: workspace/telemetry.html (Universal Tables Suite)\n");
    bool p3Loaded = engine.navigate("workspace/telemetry.html");
    AUDIT_TEST("Navigate to workspace/telemetry.html", p3Loaded);
    if (engine.rootNode) {
        engine.computeLayout(1920.0f, 0.0f);
        AUDIT_TEST("Page 3 Tables Layout Calculated", engine.rootNode->layoutWidth > 0.0f && engine.documentHeight > 0.0f);
    }

    // 4. Audit History Navigation (Back / Forward)
    printf("\n[4] Auditing Multi-Page Navigation History Stack\n");
    bool wentBack = engine.goBack(); // Back to fleet.html
    AUDIT_TEST("History Go Back to fleet.html", wentBack);
    bool wentBack2 = engine.goBack(); // Back to index.html
    AUDIT_TEST("History Go Back to index.html", wentBack2);
    bool wentFwd = engine.goForward(); // Forward to fleet.html
    AUDIT_TEST("History Go Forward to fleet.html", wentFwd);

    printf("\n===================================================================\n");
    printf(" [AUDIT SUMMARY] Total: %d | Passed: %d | Failed: %d\n", g_pass + g_fail, g_pass, g_fail);
    printf("===================================================================\n");

    return (g_fail == 0) ? 0 : 1;
}
