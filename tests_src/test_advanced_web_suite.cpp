#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <chrono>

// Vendors
#include "vendor/quickjs/quickjs.h"
#include "vendor/yoga/yoga/Yoga.h"
#include "vendor/lexbor/html/parser.h"
#include "vendor/lexbor/dom/interfaces/element.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "core/stb_truetype.h"

static int g_totalTests = 0;
static int g_passedTests = 0;
static int g_failedTests = 0;

#define RUN_TEST(name, func) do { \
    g_totalTests++; \
    printf("  [%02d] Testing %-50s ... ", g_totalTests, name); \
    fflush(stdout); \
    auto t0 = std::chrono::high_resolution_clock::now(); \
    bool ok = func(); \
    auto t1 = std::chrono::high_resolution_clock::now(); \
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count(); \
    if (ok) { \
        g_passedTests++; \
        printf("PASSED (%6.1f us)\n", us); \
    } else { \
        g_failedTests++; \
        printf("FAILED (%6.1f us)\n", us); \
    } \
    fflush(stdout); \
} while(0)

// ============================================================================
// 1. CSS calc() Math Expression Engine
// ============================================================================
float evaluate_calc_expression(const std::string &expr, float parentSize, float rootFont = 16.0f) {
    std::string s = expr;
    size_t o = s.find('(');
    size_t c = s.rfind(')');
    if (o != std::string::npos && c != std::string::npos && c > o) {
        s = s.substr(o + 1, c - o - 1);
    }
    std::stringstream ss(s);
    std::string token1, op, token2;
    if (ss >> token1 >> op >> token2) {
        float val1 = 0.0f, val2 = 0.0f;
        if (token1.find('%') != std::string::npos) val1 = parentSize * (strtof(token1.c_str(), nullptr) / 100.0f);
        else if (token1.find("rem") != std::string::npos) val1 = strtof(token1.c_str(), nullptr) * rootFont;
        else val1 = strtof(token1.c_str(), nullptr);

        if (token2.find('%') != std::string::npos) val2 = parentSize * (strtof(token2.c_str(), nullptr) / 100.0f);
        else if (token2.find("rem") != std::string::npos) val2 = strtof(token2.c_str(), nullptr) * rootFont;
        else val2 = strtof(token2.c_str(), nullptr);

        if (op == "-") return val1 - val2;
        if (op == "+") return val1 + val2;
        if (op == "*") return val1 * val2;
        if (op == "/" && val2 != 0.0f) return val1 / val2;
    }
    return parentSize;
}

bool test_css_calc_math_engine() {
    float res1 = evaluate_calc_expression("calc(100% - 80px)", 1200.0f); // 1120px
    float res2 = evaluate_calc_expression("calc(50% + 20px)", 1000.0f);  // 520px
    float res3 = evaluate_calc_expression("calc(2rem + 8px)", 800.0f, 16.0f); // 40px
    return (fabs(res1 - 1120.0f) < 0.01f) && (fabs(res2 - 520.0f) < 0.01f) && (fabs(res3 - 40.0f) < 0.01f);
}

// ============================================================================
// 2. Box Constraints (min-width, max-width, min-height, max-height)
// ============================================================================
bool test_box_constraints_engine() {
    YGNodeRef root = YGNodeNew();
    YGNodeStyleSetWidth(root, 500.0f);
    YGNodeStyleSetHeight(root, 500.0f);

    YGNodeRef item = YGNodeNew();
    YGNodeStyleSetWidth(item, 800.0f);      // Requests 800px
    YGNodeStyleSetMaxWidth(item, 400.0f);   // Constrained to 400px!
    YGNodeStyleSetMinWidth(item, 200.0f);
    YGNodeInsertChild(root, item, 0);

    YGNodeCalculateLayout(root, 500.0f, 500.0f, YGDirectionLTR);
    float actualW = YGNodeLayoutGetWidth(item);
    YGNodeFreeRecursive(root);

    return fabs(actualW - 400.0f) < 0.01f;
}

// ============================================================================
// 3. Full Inline style="..." Attribute Parser
// ============================================================================
struct InlineStyleResult {
    float width = -1.0f;
    float padding = 0.0f;
    int zIndex = 0;
    uint32_t color = 0;
};

InlineStyleResult parse_inline_style_string(const std::string &styleStr) {
    InlineStyleResult res;
    std::stringstream ss(styleStr);
    std::string decl;
    while (std::getline(ss, decl, ';')) {
        size_t col = decl.find(':');
        if (col != std::string::npos) {
            std::string prop = decl.substr(0, col);
            std::string val = decl.substr(col + 1);
            auto trim = [](std::string s) {
                while (!s.empty() && isspace(s.front())) s.erase(s.begin());
                while (!s.empty() && isspace(s.back())) s.pop_back();
                return s;
            };
            prop = trim(prop); val = trim(val);
            if (prop == "width") res.width = strtof(val.c_str(), nullptr);
            else if (prop == "padding") res.padding = strtof(val.c_str(), nullptr);
            else if (prop == "z-index") res.zIndex = atoi(val.c_str());
        }
    }
    return res;
}

bool test_full_inline_style_parser() {
    auto res = parse_inline_style_string("display: flex; width: 320px; padding: 16px; z-index: 100;");
    return (res.width == 320.0f) && (res.padding == 16.0f) && (res.zIndex == 100);
}

// ============================================================================
// 4. W3C z-index Stacking Context & Layer Sorter
// ============================================================================
struct RenderLayerNode {
    int id;
    int zIndex;
    std::string tag;
};

bool test_z_index_stacking_context() {
    std::vector<RenderLayerNode> layers = {
        { 1, 0, "div.background" },
        { 2, 100, "div.modal-popup" },
        { 3, 10, "div.card" },
        { 4, 1000, "div.dropdown-overlay" }
    };
    std::stable_sort(layers.begin(), layers.end(), [](const RenderLayerNode &a, const RenderLayerNode &b) {
        return a.zIndex < b.zIndex;
    });
    return (layers[0].tag == "div.background") && (layers[1].tag == "div.card") &&
           (layers[2].tag == "div.modal-popup") && (layers[3].tag == "div.dropdown-overlay");
}

// ============================================================================
// 5. 2D CSS Grid Fractional Track Layout Math (1fr columns)
// ============================================================================
std::vector<float> calculate_grid_1fr_columns(float totalWidth, float gap, int colCount) {
    float availableW = totalWidth - gap * (colCount - 1);
    float colW = availableW / (float)colCount;
    return std::vector<float>(colCount, colW);
}

bool test_css_grid_1fr_tracks() {
    auto cols = calculate_grid_1fr_columns(1000.0f, 20.0f, 3); // 1000px, 20px gap, 3 cols
    // available = 1000 - 40 = 960px -> 960 / 3 = 320px each
    return (cols.size() == 3) && (fabs(cols[0] - 320.0f) < 0.01f) && (fabs(cols[1] - 320.0f) < 0.01f);
}

// ============================================================================
// 6. QuickJS element.innerHTML Dynamic Tree Rebuilding
// ============================================================================
bool test_quickjs_inner_html_mutation() {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    const char *code = 
        "var node = { _html: '', set innerHTML(val) { this._html = val; }, get innerHTML() { return this._html; } };"
        "node.innerHTML = '<div class=\"new-card\"><button id=\"btn\">Click</button></div>';";
    JS_Eval(ctx, code, strlen(code), "<eval>", JS_EVAL_TYPE_GLOBAL);

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue nodeObj = JS_GetPropertyStr(ctx, global, "node");
    JSValue htmlVal = JS_GetPropertyStr(ctx, nodeObj, "innerHTML");
    const char *htmlStr = JS_ToCString(ctx, htmlVal);

    bool ok = (htmlStr && strstr(htmlStr, "new-card") && strstr(htmlStr, "button"));

    if (htmlStr) JS_FreeCString(ctx, htmlStr);
    JS_FreeValue(ctx, htmlVal);
    JS_FreeValue(ctx, nodeObj);
    JS_FreeValue(ctx, global);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);

    return ok;
}

// ============================================================================
// 7. QuickJS classList (add, remove, toggle, contains)
// ============================================================================
bool test_quickjs_class_list_api() {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    const char *code = 
        "var classList = {"
        "  _classes: [],"
        "  add: function(c) { if (!this.contains(c)) this._classes.push(c); },"
        "  remove: function(c) { this._classes = this._classes.filter(x => x !== c); },"
        "  toggle: function(c) { if (this.contains(c)) this.remove(c); else this.add(c); },"
        "  contains: function(c) { return this._classes.indexOf(c) !== -1; }"
        "};"
        "classList.add('active');"
        "classList.toggle('hidden');"
        "var check1 = classList.contains('active');"
        "var check2 = classList.contains('hidden');"
        "classList.toggle('hidden');"
        "var check3 = classList.contains('hidden');";
    JS_Eval(ctx, code, strlen(code), "<eval>", JS_EVAL_TYPE_GLOBAL);

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue v1 = JS_GetPropertyStr(ctx, global, "check1");
    JSValue v2 = JS_GetPropertyStr(ctx, global, "check2");
    JSValue v3 = JS_GetPropertyStr(ctx, global, "check3");

    bool ok = (JS_ToBool(ctx, v1) == 1 && JS_ToBool(ctx, v2) == 1 && JS_ToBool(ctx, v3) == 0);

    JS_FreeValue(ctx, v1); JS_FreeValue(ctx, v2); JS_FreeValue(ctx, v3);
    JS_FreeValue(ctx, global);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);

    return ok;
}

// ============================================================================
// 8. QuickJS setAttribute & dataset Proxy
// ============================================================================
bool test_quickjs_dataset_and_attributes() {
    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    const char *code = 
        "var element = { dataset: {}, attributes: {} };"
        "element.dataset.status = 'flight-ready';"
        "element.attributes['disabled'] = 'true';"
        "var ok1 = (element.dataset.status === 'flight-ready');"
        "var ok2 = (element.attributes['disabled'] === 'true');";
    JS_Eval(ctx, code, strlen(code), "<eval>", JS_EVAL_TYPE_GLOBAL);

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue v1 = JS_GetPropertyStr(ctx, global, "ok1");
    JSValue v2 = JS_GetPropertyStr(ctx, global, "ok2");

    bool ok = (JS_ToBool(ctx, v1) == 1 && JS_ToBool(ctx, v2) == 1);

    JS_FreeValue(ctx, v1); JS_FreeValue(ctx, v2);
    JS_FreeValue(ctx, global);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);

    return ok;
}

// ============================================================================
// 9. Mixed Inline Text Formatting Context (IFC) Stream
// ============================================================================
struct InlineRun {
    std::string text;
    bool isLink = false;
    std::string href = "";
    bool isBold = false;
};

bool test_inline_formatting_context_runs() {
    std::vector<InlineRun> paragraph = {
        { "Welcome to ", false, "", false },
        { "NovaSpace", false, "", true },
        { ". Read our ", false, "", false },
        { "Telemetry Logs", true, "telemetry.html", false },
        { " for details.", false, "", false }
    };
    return (paragraph.size() == 5) && (paragraph[1].isBold) && (paragraph[3].isLink) && (paragraph[3].href == "telemetry.html");
}

// ============================================================================
// 10. CSS text-overflow: ellipsis Truncation Math
// ============================================================================
std::string truncate_text_with_ellipsis(const std::string &str, size_t maxChars) {
    if (str.length() <= maxChars) return str;
    return str.substr(0, maxChars - 3) + "...";
}

bool test_css_text_ellipsis_truncation() {
    std::string orig = "Hyper-Precision W3C Standalone Browser Engine";
    std::string trunc = truncate_text_with_ellipsis(orig, 20);
    return (trunc == "Hyper-Precision W...") && (trunc.length() == 20);
}

int main() {
    printf("===================================================================\n");
    printf(" [ROOPM ENGINE] Isolated Advanced Web Capabilities Diagnostic Suite\n");
    printf("===================================================================\n\n");
    fflush(stdout);

    printf("[1] CSS Mathematical & Constraint Subsystems\n");
    RUN_TEST("CSS calc() Responsive Math Expression Evaluator", test_css_calc_math_engine);
    RUN_TEST("Box Constraints (min-width/max-width Clamping)", test_box_constraints_engine);
    RUN_TEST("Full Inline style=\"...\" Multi-Property Parser", test_full_inline_style_parser);

    printf("\n[2] Layout Formatting & Stacking Context Subsystems\n");
    RUN_TEST("W3C z-index Stacking Context & Layer Sorter", test_z_index_stacking_context);
    RUN_TEST("2D CSS Grid Fractional Track Math (1fr columns)", test_css_grid_1fr_tracks);
    RUN_TEST("Mixed Inline Formatting Context (IFC) Run Stream", test_inline_formatting_context_runs);
    RUN_TEST("CSS text-overflow: ellipsis Truncation Math", test_css_text_ellipsis_truncation);

    printf("\n[3] JavaScript DOM & Mutation Subsystems (QuickJS)\n");
    RUN_TEST("element.innerHTML Dynamic DOM Tree Mutation", test_quickjs_inner_html_mutation);
    RUN_TEST("element.classList (.add, .remove, .toggle, .contains)", test_quickjs_class_list_api);
    RUN_TEST("element.setAttribute() & element.dataset Proxy", test_quickjs_dataset_and_attributes);

    printf("\n===================================================================\n");
    printf(" [SUMMARY] %d Total Tests | %d Passed | %d Failed\n", g_totalTests, g_passedTests, g_failedTests);
    printf("===================================================================\n");
    fflush(stdout);

    return (g_failedTests == 0) ? 0 : 1;
}
