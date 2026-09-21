#include <unordered_map>
#ifndef ROOPM_JS_DOM_RUNTIME_H
#define ROOPM_JS_DOM_RUNTIME_H

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include "vendor/quickjs/quickjs.h"
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"

namespace UIEngine {

struct JSTimer {
    int id;
    JSValue callback;
    float interval;
    float elapsed;
    bool repeat;
};

class JSDOMRuntime {
public:
    JSRuntime *rt = nullptr;
    JSContext *ctx = nullptr;
    std::map<std::string, std::shared_ptr<DOMNode>> *nodeMap = nullptr;
    CSSParser *cssParserPtr = nullptr;
    std::shared_ptr<DOMNode> rootNodePtr = nullptr;
    std::unordered_map<uint64_t, std::shared_ptr<DOMNode>> uidMap;
    std::vector<JSValue> allAllocatedCallbacks;
    uint32_t nextListenerId = 1;
    std::unordered_map<uint32_t, JSValue> listenerCallbacks;
    std::map<std::string, JSValue> clickCallbacks;
    std::map<std::string, JSValue> inputCallbacks;
    std::vector<JSTimer> timers;
    int nextTimerId = 1;
    int nextAnonNodeId = 1;

    void setRootNode(std::shared_ptr<DOMNode> root) {
        rootNodePtr = root;
        uidMap.clear();
        if (root) indexAllNodes(root);
    }

    void indexAllNodes(const std::shared_ptr<DOMNode> &n) {
        if (!n) return;
        uidMap[n->uid] = n;
        for (const auto &c : n->children) indexAllNodes(c);
    }

    std::shared_ptr<DOMNode> querySelectorDFS(const std::shared_ptr<DOMNode> &curr, const std::string &sel) {
        if (!curr) return nullptr;
        if (cssParserPtr && cssParserPtr->matchesSelector(curr, sel)) return curr;
        for (const auto &child : curr->children) {
            auto res = querySelectorDFS(child, sel);
            if (res) return res;
        }
        return nullptr;
    }

    void querySelectorAllDFS(const std::shared_ptr<DOMNode> &curr, const std::string &sel, std::vector<std::shared_ptr<DOMNode>> &out) {
        if (!curr) return;
        if (cssParserPtr && cssParserPtr->matchesSelector(curr, sel)) {
            out.push_back(curr);
        }
        for (const auto &child : curr->children) {
            querySelectorAllDFS(child, sel, out);
        }
    }

    void init(std::map<std::string, std::shared_ptr<DOMNode>> &nodes, CSSParser &parser) {
        nodeMap = &nodes;
        cssParserPtr = &parser;
        rt = JS_NewRuntime();
        ctx = JS_NewContext(rt);
        JS_SetContextOpaque(ctx, this);
        bindGlobalAPIs();
    }

    void shutdown() {
        if (ctx) {
            for (auto &pair : clickCallbacks) JS_FreeValue(ctx, pair.second);
            for (auto &pair : inputCallbacks) JS_FreeValue(ctx, pair.second);
            for (auto &t : timers) JS_FreeValue(ctx, t.callback);
            if (nodeMap) {
                for (auto &pair : *nodeMap) {
                    pair.second->eventListeners.clear();
                }
            }
        }
        clickCallbacks.clear();
        inputCallbacks.clear();
        timers.clear();
        if (ctx) JS_FreeContext(ctx);
        if (rt) JS_FreeRuntime(rt);
        ctx = nullptr;
        rt = nullptr;
    }

    void update(float dt) {
        if (!ctx) return;
        
        std::vector<JSValue> callbacksToFire;
        for (size_t i = 0; i < timers.size(); ) {
            timers[i].elapsed += dt;
            if (timers[i].elapsed >= timers[i].interval) {
                timers[i].elapsed = 0.0f;
                callbacksToFire.push_back(JS_DupValue(ctx, timers[i].callback));

                if (!timers[i].repeat) {
                    JS_FreeValue(ctx, timers[i].callback);
                    timers.erase(timers.begin() + i);
                    continue;
                }
            }
            i++;
        }

        for (JSValue &cb : callbacksToFire) {
            JSValue global = JS_GetGlobalObject(ctx);
            JSValue ret = JS_Call(ctx, cb, global, 0, nullptr);
            JS_FreeValue(ctx, ret);
            JS_FreeValue(ctx, global);
            JS_FreeValue(ctx, cb);
        }

        JSContext *pctx;
        while (JS_ExecutePendingJob(rt, &pctx) > 0) {}
    }

    void executeScript(const std::string &jsCode) {
        if (!ctx) return;
        JSValue res = JS_Eval(ctx, jsCode.c_str(), jsCode.length(), "app.js", JS_EVAL_TYPE_GLOBAL);
        if (JS_IsException(res)) {
            JSValue exception = JS_GetException(ctx);
            const char *msg = JS_ToCString(ctx, exception);
            printf("[JS ERROR] %s\n", msg ? msg : "unknown");
            if (msg) JS_FreeCString(ctx, msg);
            JS_FreeValue(ctx, exception);
        }
        JS_FreeValue(ctx, res);
    }

    bool dispatchClick(const std::shared_ptr<DOMNode> &hitNode) {
        if (!hitNode || !ctx) return false;
        bool handled = false;
        auto curr = hitNode;
        JSValue global = JS_GetGlobalObject(ctx);

        // 🌟 W3C True Event Bubbling from hit target up to root!
        while (curr) {
            for (const auto &el : curr->eventListeners) {
                if (el.type == "click" && listenerCallbacks.count(el.listenerId)) {
                    JSValue cb = listenerCallbacks[el.listenerId];
                    JSValue ret = JS_Call(ctx, cb, global, 0, nullptr);
                    JS_FreeValue(ctx, ret);
                    handled = true;
                }
            }
            if (!curr->id.empty() && clickCallbacks.count(curr->id)) {
                JSValue cb = clickCallbacks[curr->id];
                JSValue ret = JS_Call(ctx, cb, global, 0, nullptr);
                JS_FreeValue(ctx, ret);
                handled = true;
            }
            curr = curr->parent.lock();
        }
        JS_FreeValue(ctx, global);
        return handled;
    }

    bool dispatchClick(const std::string &id) {
        if (nodeMap && nodeMap->count(id)) {
            return dispatchClick((*nodeMap)[id]);
        }
        if (clickCallbacks.find(id) != clickCallbacks.end()) {
            JSValue cb = clickCallbacks[id];
            JSValue global = JS_GetGlobalObject(ctx);
            JSValue ret = JS_Call(ctx, cb, global, 0, nullptr);
            JS_FreeValue(ctx, ret);
            JS_FreeValue(ctx, global);
            return true;
        }
        return false;
    }

    void dispatchInput(const std::string &id, const std::string &val) {
        if (inputCallbacks.find(id) != inputCallbacks.end()) {
            JSValue cb = inputCallbacks[id];
            JSValue global = JS_GetGlobalObject(ctx);
            JSValue arg = JS_NewString(ctx, val.c_str());
            JSValue ret = JS_Call(ctx, cb, global, 1, &arg);
            JS_FreeValue(ctx, arg);
            JS_FreeValue(ctx, ret);
            JS_FreeValue(ctx, global);
        }
    }

    JSValue wrapDOMNode(const std::shared_ptr<DOMNode> &node) {
        if (!node) return JS_NULL;
        JSValue el = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, el, "_node_uid", JS_NewInt64(ctx, (int64_t)node->uid));
        if (!node->id.empty()) {
            JS_SetPropertyStr(ctx, el, "_node_id", JS_NewString(ctx, node->id.c_str()));
        }

        std::string id = node->id.empty() ? ("__anon_" + std::to_string(node->uid)) : node->id;
        JS_SetPropertyStr(ctx, el, "addEventListener", JS_NewCFunction(ctx, js_dom_add_event_listener, "addEventListener", 2));
        JS_SetPropertyStr(ctx, el, "appendChild", JS_NewCFunction(ctx, js_dom_append_child, "appendChild", 1));
        JS_SetPropertyStr(ctx, el, "remove", JS_NewCFunction(ctx, js_dom_remove, "remove", 0));
        JS_SetPropertyStr(ctx, el, "setAttribute", JS_NewCFunction(ctx, js_dom_set_attribute, "setAttribute", 2));
        JS_SetPropertyStr(ctx, el, "getAttribute", JS_NewCFunction(ctx, js_dom_get_attribute, "getAttribute", 1));
        JS_SetPropertyStr(ctx, el, "removeAttribute", JS_NewCFunction(ctx, js_dom_remove_attribute, "removeAttribute", 1));
        JS_SetPropertyStr(ctx, el, "hasAttribute", JS_NewCFunction(ctx, js_dom_has_attribute, "hasAttribute", 1));
        JS_SetPropertyStr(ctx, el, "getContext", JS_NewCFunction(ctx, js_canvas_get_context, "getContext", 1));

        // 🌟 W3C element.innerHTML (Getter/Setter)
        JSAtom innerHtmlAtom = JS_NewAtom(ctx, "innerHTML");
        JSValue ihGetter = JS_NewCFunction2(ctx, (JSCFunction*)js_dom_get_inner_html, "get_innerHTML", 0, JS_CFUNC_getter, 0);
        JSValue ihSetter = JS_NewCFunction2(ctx, (JSCFunction*)js_dom_set_inner_html, "set_innerHTML", 1, JS_CFUNC_setter, 0);
        JS_DefinePropertyGetSet(ctx, el, innerHtmlAtom, ihGetter, ihSetter, JS_PROP_C_W_E);
        JS_FreeAtom(ctx, innerHtmlAtom);

        // innerText getter/setter
        JSAtom innerTextAtom = JS_NewAtom(ctx, "innerText");
        JSValue getter = JS_NewCFunction2(ctx, (JSCFunction*)js_dom_get_inner_text, "get_innerText", 0, JS_CFUNC_getter, 0);
        JSValue setter = JS_NewCFunction2(ctx, (JSCFunction*)js_dom_set_inner_text, "set_innerText", 1, JS_CFUNC_setter, 0);
        JS_DefinePropertyGetSet(ctx, el, innerTextAtom, getter, setter, JS_PROP_C_W_E);
        JS_FreeAtom(ctx, innerTextAtom);

        // value getter/setter
        JSAtom valueAtom = JS_NewAtom(ctx, "value");
        JSValue vGetter = JS_NewCFunction2(ctx, (JSCFunction*)js_dom_get_value, "get_value", 0, JS_CFUNC_getter, 0);
        JSValue vSetter = JS_NewCFunction2(ctx, (JSCFunction*)js_dom_set_value, "set_value", 1, JS_CFUNC_setter, 0);
        JS_DefinePropertyGetSet(ctx, el, valueAtom, vGetter, vSetter, JS_PROP_C_W_E);
        JS_FreeAtom(ctx, valueAtom);

        // className getter/setter
        JSAtom classAtom = JS_NewAtom(ctx, "className");
        JSValue cGetter = JS_NewCFunction2(ctx, (JSCFunction*)js_dom_get_class_name, "get_className", 0, JS_CFUNC_getter, 0);
        JSValue cSetter = JS_NewCFunction2(ctx, (JSCFunction*)js_dom_set_class_name, "set_className", 1, JS_CFUNC_setter, 0);
        JS_DefinePropertyGetSet(ctx, el, classAtom, cGetter, cSetter, JS_PROP_C_W_E);
        JS_FreeAtom(ctx, classAtom);

        // 🌟 W3C element.classList Object Proxy
        JSValue classListObj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, classListObj, "_node_id", JS_NewString(ctx, id.c_str()));
        JS_SetPropertyStr(ctx, classListObj, "add", JS_NewCFunction(ctx, js_classlist_add, "add", 1));
        JS_SetPropertyStr(ctx, classListObj, "remove", JS_NewCFunction(ctx, js_classlist_remove, "remove", 1));
        JS_SetPropertyStr(ctx, classListObj, "toggle", JS_NewCFunction(ctx, js_classlist_toggle, "toggle", 1));
        JS_SetPropertyStr(ctx, classListObj, "contains", JS_NewCFunction(ctx, js_classlist_contains, "contains", 1));
        JS_SetPropertyStr(ctx, el, "classList", classListObj);

        // 🌟 W3C element.dataset Object Proxy
        JSValue datasetObj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, datasetObj, "_node_id", JS_NewString(ctx, id.c_str()));
        JS_SetPropertyStr(ctx, el, "dataset", datasetObj);

        // style Object Proxy
        JSValue styleObj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, styleObj, "_node_id", JS_NewString(ctx, id.c_str()));
        JSAtom dispAtom = JS_NewAtom(ctx, "display");
        JSValue dispG = JS_NewCFunction2(ctx, (JSCFunction*)js_style_get_display, "get_display", 0, JS_CFUNC_getter, 0);
        JSValue dispS = JS_NewCFunction2(ctx, (JSCFunction*)js_style_set_display, "set_display", 1, JS_CFUNC_setter, 0);
        JS_DefinePropertyGetSet(ctx, styleObj, dispAtom, dispG, dispS, JS_PROP_C_W_E);
        JS_FreeAtom(ctx, dispAtom);
        JS_SetPropertyStr(ctx, el, "style", styleObj);

        return el;
    }

    JSValue wrapDOMNode(const std::string &id) {
        if (nodeMap && nodeMap->count(id)) {
            return wrapDOMNode((*nodeMap)[id]);
        }
        return JS_NULL;
    }

private:
    static JSValue js_console_log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        for (int i = 0; i < argc; i++) {
            const char *str = JS_ToCString(ctx, argv[i]);
            if (str) {
                printf("%s%s", i > 0 ? " " : "", str);
                JS_FreeCString(ctx, str);
            }
        }
        printf("\n");
        return JS_UNDEFINED;
    }

    static JSValue js_console_warn(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        printf("[WARN] ");
        for (int i = 0; i < argc; i++) {
            const char *str = JS_ToCString(ctx, argv[i]);
            if (str) {
                printf("%s%s", i > 0 ? " " : "", str);
                JS_FreeCString(ctx, str);
            }
        }
        printf("\n");
        return JS_UNDEFINED;
    }

    static JSValue js_console_error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        printf("[ERROR] ");
        for (int i = 0; i < argc; i++) {
            const char *str = JS_ToCString(ctx, argv[i]);
            if (str) {
                printf("%s%s", i > 0 ? " " : "", str);
                JS_FreeCString(ctx, str);
            }
        }
        printf("\n");
        return JS_UNDEFINED;
    }

    static JSValue js_set_timeout(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_UNDEFINED;
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        if (!self) return JS_UNDEFINED;

        double ms = 0.0;
        if (argc >= 2) JS_ToFloat64(ctx, &ms, argv[1]);

        JSTimer t;
        t.id = self->nextTimerId++;
        t.callback = JS_DupValue(ctx, argv[0]);
        t.interval = (float)(ms / 1000.0);
        t.elapsed = 0.0f;
        t.repeat = false; // 🌟 Single-shot Timeout
        self->timers.push_back(t);
        return JS_NewInt32(ctx, t.id);
    }

    static JSValue js_clear_timer(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_UNDEFINED;
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        if (!self) return JS_UNDEFINED;

        int id = 0;
        JS_ToInt32(ctx, &id, argv[0]);
        for (size_t i = 0; i < self->timers.size(); i++) {
            if (self->timers[i].id == id) {
                JS_FreeValue(ctx, self->timers[i].callback);
                self->timers.erase(self->timers.begin() + i);
                break;
            }
        }
        return JS_UNDEFINED;
    }

    static JSValue js_set_interval(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 2) return JS_UNDEFINED;
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        if (!self) return JS_UNDEFINED;

        double ms = 1000.0;
        JS_ToFloat64(ctx, &ms, argv[1]);
        
        JSTimer t;
        t.id = self->nextTimerId++;
        t.callback = JS_DupValue(ctx, argv[0]);
        t.interval = (float)(ms / 1000.0);
        t.elapsed = 0.0f;
        t.repeat = true;
        self->timers.push_back(t);
        return JS_NewInt32(ctx, t.id);
    }

    static JSValue js_dom_get_inner_html(JSContext *ctx, JSValueConst this_val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        std::string res = "";
        if (idStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            res = (*self->nodeMap)[idStr]->innerText;
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return JS_NewString(ctx, res.c_str());
    }

    static JSValue js_dom_set_inner_html(JSContext *ctx, JSValueConst this_val, JSValueConst val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *htmlStr = JS_ToCString(ctx, val);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (idStr && htmlStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            
            // 🌟 1. Completely clear all old children and placeholder text!
            node->children.clear();
            node->innerText.clear();
            node->wrappedLines.clear();

            std::string fragment(htmlStr);
            if (fragment.find('<') != std::string::npos && fragment.find('>') != std::string::npos) {
                // 🌟 2. Parse real HTML fragment with Lexbor
                auto newChildren = HTMLDOMBuilder::parseFragment(fragment, *self->cssParserPtr);
                for (auto &c : newChildren) {
                    node->addChild(c);
                    if (c->id.empty()) {
                        c->id = "__dyn_" + std::to_string(self->nextAnonNodeId++);
                    }
                    (*self->nodeMap)[c->id] = c;
                    self->cssParserPtr->applyToNode(c);
                }
            } else {
                node->innerText = fragment;
            }
        }

        if (idStr) JS_FreeCString(ctx, idStr);
        if (htmlStr) JS_FreeCString(ctx, htmlStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_dom_get_inner_text(JSContext *ctx, JSValueConst this_val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        std::string res = "";
        if (idStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            res = (*self->nodeMap)[idStr]->innerText;
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return JS_NewString(ctx, res.c_str());
    }

    static JSValue js_dom_set_inner_text(JSContext *ctx, JSValueConst this_val, JSValueConst val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *textStr = JS_ToCString(ctx, val);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        if (idStr && textStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            node->innerText = textStr;
            node->wrappedLines.clear();
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (textStr) JS_FreeCString(ctx, textStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_dom_get_value(JSContext *ctx, JSValueConst this_val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        std::string res = "";
        if (idStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            res = (*self->nodeMap)[idStr]->inputValue;
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return JS_NewString(ctx, res.c_str());
    }

    static JSValue js_dom_set_value(JSContext *ctx, JSValueConst this_val, JSValueConst val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *textStr = JS_ToCString(ctx, val);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        if (idStr && textStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            (*self->nodeMap)[idStr]->inputValue = textStr;
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (textStr) JS_FreeCString(ctx, textStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_dom_get_class_name(JSContext *ctx, JSValueConst this_val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        std::string res = "";
        if (idStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            for (const auto &c : (*self->nodeMap)[idStr]->classes) {
                res += (res.empty() ? "" : " ") + c;
            }
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return JS_NewString(ctx, res.c_str());
    }

    static JSValue js_dom_set_class_name(JSContext *ctx, JSValueConst this_val, JSValueConst val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *clsStr = JS_ToCString(ctx, val);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        if (idStr && clsStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            node->classes.clear();
            std::stringstream ss(clsStr);
            std::string item;
            while (ss >> item) node->classes.push_back(item);
            if (self->cssParserPtr) self->cssParserPtr->applyToNode(node);
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (clsStr) JS_FreeCString(ctx, clsStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_style_set_display(JSContext *ctx, JSValueConst this_val, JSValueConst val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *dispStr = JS_ToCString(ctx, val);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        if (idStr && dispStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            if (std::string(dispStr) == "none") node->style.display = DisplayMode::None;
            else if (std::string(dispStr) == "flex") node->style.display = DisplayMode::Flex;
            else node->style.display = DisplayMode::Block;
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (dispStr) JS_FreeCString(ctx, dispStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_style_get_display(JSContext *ctx, JSValueConst this_val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        std::string res = "block";
        if (idStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            if (node->style.display == DisplayMode::None) res = "none";
            else if (node->style.display == DisplayMode::Flex) res = "flex";
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return JS_NewString(ctx, res.c_str());
    }

    // 🌟 classList APIs (.add, .remove, .toggle, .contains)
    static JSValue js_classlist_add(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_UNDEFINED;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *cls = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (idStr && cls && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            if (!node->hasClass(cls)) {
                node->classes.push_back(cls);
                if (self->cssParserPtr) self->cssParserPtr->applyToNode(node);
            }
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (cls) JS_FreeCString(ctx, cls);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_classlist_remove(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_UNDEFINED;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *cls = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (idStr && cls && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            node->classes.erase(std::remove(node->classes.begin(), node->classes.end(), cls), node->classes.end());
            if (self->cssParserPtr) self->cssParserPtr->applyToNode(node);
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (cls) JS_FreeCString(ctx, cls);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_classlist_toggle(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_UNDEFINED;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *cls = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        bool result = false;
        if (idStr && cls && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            if (node->hasClass(cls)) {
                node->classes.erase(std::remove(node->classes.begin(), node->classes.end(), cls), node->classes.end());
                result = false;
            } else {
                node->classes.push_back(cls);
                result = true;
            }
            if (self->cssParserPtr) self->cssParserPtr->applyToNode(node);
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (cls) JS_FreeCString(ctx, cls);
        JS_FreeValue(ctx, idVal);
        return JS_NewBool(ctx, result);
    }

    static JSValue js_classlist_contains(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_FALSE;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *cls = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        bool result = false;
        if (idStr && cls && self && self->nodeMap && self->nodeMap->count(idStr)) {
            result = (*self->nodeMap)[idStr]->hasClass(cls);
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (cls) JS_FreeCString(ctx, cls);
        JS_FreeValue(ctx, idVal);
        return JS_NewBool(ctx, result);
    }

    // 🌟 setAttribute, getAttribute, removeAttribute, hasAttribute
    static JSValue js_dom_set_attribute(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 2) return JS_UNDEFINED;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *name = JS_ToCString(ctx, argv[0]);
        const char *val = JS_ToCString(ctx, argv[1]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (idStr && name && val && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            node->attributes[name] = val;
            if (self->cssParserPtr) self->cssParserPtr->applyToNode(node);
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (name) JS_FreeCString(ctx, name);
        if (val) JS_FreeCString(ctx, val);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_dom_get_attribute(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_NULL;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *name = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        std::string res = "";
        bool found = false;
        if (idStr && name && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            if (node->attributes.count(name)) {
                res = node->attributes[name];
                found = true;
            }
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (name) JS_FreeCString(ctx, name);
        JS_FreeValue(ctx, idVal);
        return found ? JS_NewString(ctx, res.c_str()) : JS_NULL;
    }

    static JSValue js_dom_remove_attribute(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_UNDEFINED;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *name = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (idStr && name && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            node->attributes.erase(name);
            if (self->cssParserPtr) self->cssParserPtr->applyToNode(node);
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (name) JS_FreeCString(ctx, name);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_dom_has_attribute(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_FALSE;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *name = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        bool res = false;
        if (idStr && name && self && self->nodeMap && self->nodeMap->count(idStr)) {
            res = ((*self->nodeMap)[idStr]->attributes.count(name) > 0);
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (name) JS_FreeCString(ctx, name);
        JS_FreeValue(ctx, idVal);
        return JS_NewBool(ctx, res);
    }

    static JSValue js_dom_remove(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (idStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            auto p = node->parent.lock();
            if (p) {
                p->children.erase(std::remove(p->children.begin(), p->children.end(), node), p->children.end());
            }
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_dom_add_event_listener(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 2) return JS_UNDEFINED;
        const char *eventStr = JS_ToCString(ctx, argv[0]);
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (eventStr && idStr && self) {
            std::string sId(idStr);
            if (std::string(eventStr) == "click") {
                if (self->clickCallbacks.count(sId)) JS_FreeValue(ctx, self->clickCallbacks[sId]);
                self->clickCallbacks[sId] = JS_DupValue(ctx, argv[1]);
            } else if (std::string(eventStr) == "input") {
                if (self->inputCallbacks.count(sId)) JS_FreeValue(ctx, self->inputCallbacks[sId]);
                self->inputCallbacks[sId] = JS_DupValue(ctx, argv[1]);
            }
        }
        if (eventStr) JS_FreeCString(ctx, eventStr);
        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_canvas_get_context(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_NULL;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        const char *idStr = JS_ToCString(ctx, idVal);

        JSValue ctx2d = JS_NewObject(ctx);
        if (idStr) JS_SetPropertyStr(ctx, ctx2d, "_canvas_id", JS_NewString(ctx, idStr));
        
        JS_SetPropertyStr(ctx, ctx2d, "fillRect", JS_NewCFunction(ctx, js_ctx_fill_rect, "fillRect", 4));
        JS_SetPropertyStr(ctx, ctx2d, "clearRect", JS_NewCFunction(ctx, js_ctx_clear_rect, "clearRect", 4));

        JSAtom fillStyleAtom = JS_NewAtom(ctx, "fillStyle");
        JSValue fsSetter = JS_NewCFunction2(ctx, (JSCFunction*)js_canvas_set_fill_style, "set_fillStyle", 1, JS_CFUNC_setter, 0);
        JS_DefinePropertyGetSet(ctx, ctx2d, fillStyleAtom, JS_UNDEFINED, fsSetter, JS_PROP_C_W_E);
        JS_FreeAtom(ctx, fillStyleAtom);

        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return ctx2d;
    }

    static JSValue js_canvas_set_fill_style(JSContext *ctx, JSValueConst this_val, JSValueConst val) {
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_canvas_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        const char *colStr = JS_ToCString(ctx, val);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (idStr && colStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            (*self->nodeMap)[idStr]->ctxFillColor = CSSParser::parseColor(colStr);
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        if (colStr) JS_FreeCString(ctx, colStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_ctx_fill_rect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 4) return JS_UNDEFINED;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_canvas_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (idStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            if (!node->canvasPixels.empty()) {
                double rx, ry, rw, rh;
                JS_ToFloat64(ctx, &rx, argv[0]);
                JS_ToFloat64(ctx, &ry, argv[1]);
                JS_ToFloat64(ctx, &rw, argv[2]);
                JS_ToFloat64(ctx, &rh, argv[3]);

                int ix0 = std::max(0, (int)rx), iy0 = std::max(0, (int)ry);
                int ix1 = std::min((int)node->canvasW, (int)(rx + rw));
                int iy1 = std::min((int)node->canvasH, (int)(ry + rh));

                for (int y = iy0; y < iy1; y++) {
                    for (int x = ix0; x < ix1; x++) {
                        node->canvasPixels[y * node->canvasW + x] = node->ctxFillColor;
                    }
                }
            }
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_ctx_clear_rect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 4) return JS_UNDEFINED;
        JSValue idVal = JS_GetPropertyStr(ctx, this_val, "_canvas_id");
        const char *idStr = JS_ToCString(ctx, idVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (idStr && self && self->nodeMap && self->nodeMap->count(idStr)) {
            auto node = (*self->nodeMap)[idStr];
            if (!node->canvasPixels.empty()) {
                double rx, ry, rw, rh;
                JS_ToFloat64(ctx, &rx, argv[0]);
                JS_ToFloat64(ctx, &ry, argv[1]);
                JS_ToFloat64(ctx, &rw, argv[2]);
                JS_ToFloat64(ctx, &rh, argv[3]);

                int ix0 = std::max(0, (int)rx), iy0 = std::max(0, (int)ry);
                int ix1 = std::min((int)node->canvasW, (int)(rx + rw));
                int iy1 = std::min((int)node->canvasH, (int)(ry + rh));

                for (int y = iy0; y < iy1; y++) {
                    for (int x = ix0; x < ix1; x++) {
                        node->canvasPixels[y * node->canvasW + x] = 0xFF030712;
                    }
                }
            }
        }
        if (idStr) JS_FreeCString(ctx, idStr);
        JS_FreeValue(ctx, idVal);
        return JS_UNDEFINED;
    }

    static JSValue js_document_create_element(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_NULL;
        const char *tag = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);
        if (!tag || !self) {
            if (tag) JS_FreeCString(ctx, tag);
            return JS_NULL;
        }

        std::string tagStr(tag);
        JS_FreeCString(ctx, tag);

        auto newNode = std::make_shared<DOMNode>(tagStr);
        std::string newId = "__dyn_node_" + std::to_string(self->nextAnonNodeId++);
        newNode->id = newId;

        if (tagStr == "div" || tagStr == "p") newNode->style.display = DisplayMode::Block;
        else if (tagStr == "button" || tagStr == "span") newNode->style.display = DisplayMode::InlineBlock;

        if (self->cssParserPtr) self->cssParserPtr->applyToNode(newNode);
        (*self->nodeMap)[newId] = newNode;

        return self->wrapDOMNode(newId);
    }

    static JSValue js_dom_append_child(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_UNDEFINED;
        JSValue parentIdVal = JS_GetPropertyStr(ctx, this_val, "_node_id");
        JSValue childIdVal = JS_GetPropertyStr(ctx, argv[0], "_node_id");
        const char *pId = JS_ToCString(ctx, parentIdVal);
        const char *cId = JS_ToCString(ctx, childIdVal);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (pId && cId && self && self->nodeMap && self->nodeMap->count(pId) && self->nodeMap->count(cId)) {
            auto parentNode = (*self->nodeMap)[pId];
            auto childNode = (*self->nodeMap)[cId];
            parentNode->addChild(childNode);
            if (self->cssParserPtr) self->cssParserPtr->applyToNode(childNode);
        }

        if (pId) JS_FreeCString(ctx, pId);
        if (cId) JS_FreeCString(ctx, cId);
        JS_FreeValue(ctx, parentIdVal);
        JS_FreeValue(ctx, childIdVal);
        return JS_UNDEFINED;
    }

    static JSValue js_document_get_element_by_id(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_NULL;
        const char *id = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (!id || !self || !self->nodeMap || !self->nodeMap->count(id)) {
            if (id) JS_FreeCString(ctx, id);
            return JS_NULL;
        }

        std::string idStr(id);
        JS_FreeCString(ctx, id);
        return self->wrapDOMNode(idStr);
    }

    static JSValue js_document_query_selector(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_NULL;
        const char *sel = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        if (!sel || !self || !self->cssParserPtr) {
            if (sel) JS_FreeCString(ctx, sel);
            return JS_NULL;
        }

        std::string selStr(sel);
        JS_FreeCString(ctx, sel);

        if (self->rootNodePtr) {
            auto found = self->querySelectorDFS(self->rootNodePtr, selStr);
            if (found) return self->wrapDOMNode(found);
        } else if (self->nodeMap) {
            for (const auto &pair : *self->nodeMap) {
                if (self->cssParserPtr->matchesSelector(pair.second, selStr)) {
                    return self->wrapDOMNode(pair.second);
                }
            }
        }
        return JS_NULL;
    }

    // 🌟 W3C document.querySelectorAll(selector)
    static JSValue js_document_query_selector_all(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
        if (argc < 1) return JS_NewArray(ctx);
        const char *sel = JS_ToCString(ctx, argv[0]);
        JSDOMRuntime *self = (JSDOMRuntime*)JS_GetContextOpaque(ctx);

        JSValue arr = JS_NewArray(ctx);
        if (!sel || !self || !self->cssParserPtr) {
            if (sel) JS_FreeCString(ctx, sel);
            return arr;
        }

        std::string selStr(sel);
        JS_FreeCString(ctx, sel);

        std::vector<std::shared_ptr<DOMNode>> matches;
        if (self->rootNodePtr) {
            self->querySelectorAllDFS(self->rootNodePtr, selStr, matches);
        } else if (self->nodeMap) {
            for (const auto &pair : *self->nodeMap) {
                if (self->cssParserPtr->matchesSelector(pair.second, selStr)) {
                    matches.push_back(pair.second);
                }
            }
        }

        for (uint32_t i = 0; i < (uint32_t)matches.size(); i++) {
            JS_SetPropertyUint32(ctx, arr, i, self->wrapDOMNode(matches[i]));
        }
        return arr;
    }

    void bindGlobalAPIs() {
        JSValue global = JS_GetGlobalObject(ctx);

        JSValue console = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, js_console_log, "log", 1));
        JS_SetPropertyStr(ctx, console, "warn", JS_NewCFunction(ctx, js_console_warn, "warn", 1));
        JS_SetPropertyStr(ctx, console, "error", JS_NewCFunction(ctx, js_console_error, "error", 1));
        JS_SetPropertyStr(ctx, console, "info", JS_NewCFunction(ctx, js_console_log, "info", 1));
        JS_SetPropertyStr(ctx, console, "debug", JS_NewCFunction(ctx, js_console_log, "debug", 1));
        JS_SetPropertyStr(ctx, global, "console", console);

        JS_SetPropertyStr(ctx, global, "setInterval", JS_NewCFunction(ctx, js_set_interval, "setInterval", 2));
        JS_SetPropertyStr(ctx, global, "setTimeout", JS_NewCFunction(ctx, js_set_timeout, "setTimeout", 2));
        JS_SetPropertyStr(ctx, global, "clearInterval", JS_NewCFunction(ctx, js_clear_timer, "clearInterval", 1));
        JS_SetPropertyStr(ctx, global, "clearTimeout", JS_NewCFunction(ctx, js_clear_timer, "clearTimeout", 1));

        // 🌟 W3C Standard window === globalThis Object
        JS_SetPropertyStr(ctx, global, "window", JS_DupValue(ctx, global));
        JS_SetPropertyStr(ctx, global, "innerWidth", JS_NewInt32(ctx, 1920));
        JS_SetPropertyStr(ctx, global, "innerHeight", JS_NewInt32(ctx, 1080));

        JSValue document = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, document, "getElementById", JS_NewCFunction(ctx, js_document_get_element_by_id, "getElementById", 1));
        JS_SetPropertyStr(ctx, document, "querySelector", JS_NewCFunction(ctx, js_document_query_selector, "querySelector", 1));
        JS_SetPropertyStr(ctx, document, "querySelectorAll", JS_NewCFunction(ctx, js_document_query_selector_all, "querySelectorAll", 1));
        JS_SetPropertyStr(ctx, document, "createElement", JS_NewCFunction(ctx, js_document_create_element, "createElement", 1));
        JS_SetPropertyStr(ctx, global, "document", document);

        JS_FreeValue(ctx, global);
    }
};

} // namespace UIEngine

#endif // ROOPM_JS_DOM_RUNTIME_H
