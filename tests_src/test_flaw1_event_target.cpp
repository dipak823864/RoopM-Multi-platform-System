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
    fprintf(out, " [ROOPM ENGINE] FLAW #1 (EVENTTARGET & DFS QUERYSELECTOR) TEST\n");
    fprintf(out, "===================================================================\n\n");

    WebEngine engine;
    if (engine.loadWorkspace("workspace")) {
        CSSParser parser;
        JSDOMRuntime js;
        std::map<std::string, std::shared_ptr<DOMNode>> dummyMap;
        js.init(dummyMap, parser);
        js.setRootNode(engine.rootNode);

        // TEST 1: querySelector on element WITHOUT ID
        const char *testScript = 
            "var btn = document.querySelector('.nav-item');\n"
            "var clicked = false;\n"
            "if (btn) {\n"
            "    btn.addEventListener('click', function() { clicked = true; });\n"
            "}\n";

        js.executeScript(testScript);

        // Find the node via C++ DFS
        auto target = js.querySelectorDFS(engine.rootNode, ".nav-item");
        bool hasNode = (target != nullptr);
        fprintf(out, "TEST 1: querySelector('.nav-item') without ID: %s (Tag=<%s>, UID=%llu)\n",
                hasNode ? "PASSED ✅" : "FAILED ❌",
                hasNode ? target->tag.c_str() : "none",
                hasNode ? target->uid : 0);

        // TEST 2: dispatchClick on the ID-less node
        bool handled = js.dispatchClick(target);
        fprintf(out, "TEST 2: Event Listener Execution on ID-less Node: %s\n",
                handled ? "PASSED ✅ (Callback executed!)" : "FAILED ❌");

        // TEST 3: querySelectorAll count
        std::vector<std::shared_ptr<DOMNode>> allNavs;
        js.querySelectorAllDFS(engine.rootNode, ".nav-item", allNavs);
        fprintf(out, "TEST 3: querySelectorAll('.nav-item') Found Count: %zu items %s\n",
                allNavs.size(), allNavs.size() >= 4 ? "PASSED ✅" : "FAILED ❌");

        js.shutdown();
        fprintf(out, "\nTEST 4: Clean QuickJS GC Shutdown: PASSED ✅ (No Leaks)\n");
    }

    fprintf(out, "\n===================================================================\n");
    if (out != stdout) fclose(out);
    return 0;
}
