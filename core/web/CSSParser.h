#ifndef ROOPM_CSS_PARSER_H
#define ROOPM_CSS_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "core/web/DOMNode.h"

namespace UIEngine {

struct CSSDeclaration {
    std::string property;
    std::string value;
    bool isImportant = false;
};

struct CSSRule {
    std::string selector;
    bool isHover = false;
    int specificity = 0;
    float mediaMaxWidth = 100000.0f;
    float mediaMinWidth = 0.0f;
    std::vector<CSSDeclaration> declarations;
};

class CSSParser {
public:
    std::vector<CSSRule> rules;

    static inline std::string trim(const std::string &s) {
        auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c){ return std::isspace(c); });
        auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c){ return std::isspace(c); }).base();
        return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
    }

    static uint32_t parseColor(const std::string &col) {
        std::string s = trim(col);
        if (s.empty()) return 0x00000000;
        if (s[0] == '#') {
            unsigned int r = 255, g = 255, b = 255, a = 255;
            if (s.length() == 7) sscanf(s.c_str() + 1, "%02x%02x%02x", &r, &g, &b);
            else if (s.length() == 9) sscanf(s.c_str() + 1, "%02x%02x%02x%02x", &r, &g, &b, &a);
            else if (s.length() == 4) {
                unsigned int hr, hg, hb; sscanf(s.c_str() + 1, "%1x%1x%1x", &hr, &hg, &hb);
                r = hr * 17; g = hg * 17; b = hb * 17;
            }
            return (a << 24) | (r << 16) | (g << 8) | b;
        }
        if (s.find("rgba(") != std::string::npos || s.find("rgb(") != std::string::npos) {
            unsigned int r = 255, g = 255, b = 255; float a = 1.0f;
            if (s.find("rgba") != std::string::npos) sscanf(s.c_str(), "rgba(%u,%u,%u,%f)", &r, &g, &b, &a);
            else sscanf(s.c_str(), "rgb(%u,%u,%u)", &r, &g, &b);
            return ((uint8_t)(a * 255.0f) << 24) | (r << 16) | (g << 8) | b;
        }
        if (s == "white") return 0xFFFFFFFF;
        if (s == "black") return 0xFF000000;
        if (s == "transparent") return 0x00000000;
        return 0xFF3B82F6;
    }

    static float evaluateCalc(const std::string &expr, float parentSize = 1000.0f, float rootFont = 16.0f) {
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

    static float resolveUnit(const std::string &val, float defaultVal = 0.0f, float currentFontSize = 14.0f, float rootFontSize = 16.0f, float vpW = 1920.0f, float vpH = 1080.0f) {
        std::string s = trim(val);
        if (s.empty()) return defaultVal;
        
        if (s.find("calc(") != std::string::npos) {
            return evaluateCalc(s, vpW, rootFontSize);
        }

        char *end = nullptr;
        float num = strtof(s.c_str(), &end);
        if (end == s.c_str()) return defaultVal;

        std::string unit = trim(std::string(end));
        if (unit == "rem") return num * rootFontSize;
        if (unit == "em") return num * currentFontSize;
        if (unit == "vw") return (num / 100.0f) * vpW;
        if (unit == "vh") return (num / 100.0f) * vpH;
        if (unit == "pt") return num * 1.3333f;
        return num;
    }

    static float parsePx(const std::string &val, float defaultVal = 0.0f) {
        return resolveUnit(val, defaultVal);
    }

    static std::string stripComments(const std::string &input) {
        std::string clean = "";
        size_t pos = 0;
        while (pos < input.length()) {
            size_t start = input.find("/*", pos);
            if (start == std::string::npos) {
                clean += input.substr(pos);
                break;
            }
            clean += input.substr(pos, start - pos);
            size_t end = input.find("*/", start + 2);
            if (end == std::string::npos) break;
            pos = end + 2;
        }
        return clean;
    }

    void parse(const std::string &rawCss) {
        rules.clear();
        std::string css = stripComments(rawCss);

        size_t pos = 0;
        while (pos < css.length()) {
            size_t atMediaPos = css.find("@media", pos);
            size_t nextBrace = css.find('{', pos);

            if (atMediaPos != std::string::npos && atMediaPos < nextBrace) {
                size_t mediaOpen = css.find('{', atMediaPos);
                if (mediaOpen == std::string::npos) break;
                
                std::string mediaHeader = trim(css.substr(atMediaPos, mediaOpen - atMediaPos));
                float maxW = 100000.0f, minW = 0.0f;
                if (mediaHeader.find("max-width") != std::string::npos) {
                    size_t colon = mediaHeader.find(':', mediaHeader.find("max-width"));
                    if (colon != std::string::npos) maxW = parsePx(mediaHeader.substr(colon + 1), 100000.0f);
                }
                if (mediaHeader.find("min-width") != std::string::npos) {
                    size_t colon = mediaHeader.find(':', mediaHeader.find("min-width"));
                    if (colon != std::string::npos) minW = parsePx(mediaHeader.substr(colon + 1), 0.0f);
                }

                int braceDepth = 1;
                size_t mediaEnd = mediaOpen + 1;
                while (mediaEnd < css.length() && braceDepth > 0) {
                    if (css[mediaEnd] == '{') braceDepth++;
                    else if (css[mediaEnd] == '}') braceDepth--;
                    mediaEnd++;
                }

                std::string mediaBody = css.substr(mediaOpen + 1, mediaEnd - mediaOpen - 2);
                parseRulesBlock(mediaBody, maxW, minW);
                pos = mediaEnd;
                continue;
            }

            if (nextBrace == std::string::npos) break;
            size_t closeBrace = css.find('}', nextBrace);
            if (closeBrace == std::string::npos) break;

            std::string selectorGroup = trim(css.substr(pos, nextBrace - pos));
            std::string body = css.substr(nextBrace + 1, closeBrace - nextBrace - 1);
            pos = closeBrace + 1;

            if (!selectorGroup.empty()) {
                parseSingleRule(selectorGroup, body, 100000.0f, 0.0f);
            }
        }

        std::stable_sort(rules.begin(), rules.end(), [](const CSSRule &a, const CSSRule &b){
            return a.specificity < b.specificity;
        });
    }

    void parseRulesBlock(const std::string &blockCss, float maxW, float minW) {
        size_t pos = 0;
        while (pos < blockCss.length()) {
            size_t open = blockCss.find('{', pos);
            if (open == std::string::npos) break;
            size_t close = blockCss.find('}', open);
            if (close == std::string::npos) break;

            std::string sel = trim(blockCss.substr(pos, open - pos));
            std::string body = blockCss.substr(open + 1, close - open - 1);
            pos = close + 1;

            if (!sel.empty()) {
                parseSingleRule(sel, body, maxW, minW);
            }
        }
    }

    void parseSingleRule(const std::string &selectorGroup, const std::string &body, float maxW, float minW) {
        std::stringstream ssSel(selectorGroup);
        std::string singleSel;
        while (std::getline(ssSel, singleSel, ',')) {
            singleSel = trim(singleSel);
            if (singleSel.empty()) continue;

            CSSRule rule;
            if (singleSel.find(":hover") != std::string::npos) {
                rule.isHover = true;
                singleSel = trim(singleSel.substr(0, singleSel.find(":hover")));
            }
            rule.selector = singleSel;
            rule.specificity = calculateSpecificity(singleSel) + (rule.isHover ? 10 : 0) + (maxW < 99999.0f ? 2 : 0);
            rule.mediaMaxWidth = maxW;
            rule.mediaMinWidth = minW;

            std::stringstream ss(body);
            std::string line;
            while (std::getline(ss, line, ';')) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string prop = trim(line.substr(0, colon));
                    std::string val = trim(line.substr(colon + 1));
                    if (!prop.empty() && !val.empty()) {
                        CSSDeclaration decl;
                        decl.property = prop;
                        
                        if (val.find("!important") != std::string::npos) {
                            decl.isImportant = true;
                            val = trim(val.substr(0, val.find("!important")));
                        }
                        decl.value = val;
                        rule.declarations.push_back(decl);
                    }
                }
            }
            rules.push_back(rule);
        }
    }

    // 🌟 W3C CSS 2.1 & Selectors Level 3 Specificity Scoreboard (ID: 10000, Class: 100, Type: 1, Universal: 0)
    int calculateSpecificity(const std::string &rawSel) const {
        std::string s = trim(rawSel);
        if (s.empty() || s == "*") return 0; // Universal selector has specificity 0!

        int idCount = 0;
        int classCount = 0;
        int typeCount = 0;

        size_t i = 0;
        bool hasType = false;

        while (i < s.length()) {
            char c = s[i];
            if (c == ' ' || c == '>' || c == '+' || c == '~') {
                hasType = false;
                i++;
                continue;
            }

            if (c == '#') {
                idCount++;
                i++;
                while (i < s.length() && (isalnum((unsigned char)s[i]) || s[i] == '-' || s[i] == '_')) i++;
            } else if (c == '.') {
                classCount++;
                i++;
                while (i < s.length() && (isalnum((unsigned char)s[i]) || s[i] == '-' || s[i] == '_')) i++;
            } else if (c == '[') {
                classCount++;
                size_t close = s.find(']', i);
                if (close != std::string::npos) i = close + 1;
                else i++;
            } else if (c == ':') {
                classCount++;
                i++;
                while (i < s.length() && (isalnum((unsigned char)s[i]) || s[i] == '-' || s[i] == '_')) i++;
            } else if (isalpha((unsigned char)c)) {
                if (!hasType) {
                    typeCount++;
                    hasType = true;
                }
                while (i < s.length() && (isalnum((unsigned char)s[i]) || s[i] == '-' || s[i] == '_')) i++;
            } else {
                i++;
            }
        }

        return (idCount * 10000) + (classCount * 100) + typeCount;
    }

    static bool matchSimpleSelector(const std::shared_ptr<DOMNode> &node, const std::string &rawSel) {
        std::string sel = rawSel;
        if (sel == "*") return true;

        size_t attrOpen = sel.find('[');
        if (attrOpen != std::string::npos) {
            size_t attrClose = sel.find(']', attrOpen);
            if (attrClose != std::string::npos) {
                std::string prefixTag = sel.substr(0, attrOpen);
                std::string attrExpr = sel.substr(attrOpen + 1, attrClose - attrOpen - 1);
                
                if (!prefixTag.empty() && !matchSimpleSelector(node, prefixTag)) return false;

                size_t eqPos = attrExpr.find('=');
                if (eqPos != std::string::npos) {
                    std::string aName = trim(attrExpr.substr(0, eqPos));
                    std::string aVal = trim(attrExpr.substr(eqPos + 1));
                    if (!aVal.empty() && (aVal.front() == '"' || aVal.front() == '\'')) aVal = aVal.substr(1, aVal.length() - 2);
                    
                    if (aName == "type") {
                        if (node->isCheckbox && aVal == "checkbox") return true;
                        if (node->isInput && aVal == "text") return true;
                    }
                    return (node->attributes.count(aName) && node->attributes[aName] == aVal);
                } else {
                    return node->attributes.count(trim(attrExpr)) > 0;
                }
            }
        }

        size_t pseudoPos = sel.find(':');
        if (pseudoPos != std::string::npos) {
            std::string baseSel = sel.substr(0, pseudoPos);
            std::string pseudo = sel.substr(pseudoPos);

            if (!baseSel.empty() && !matchSimpleSelector(node, baseSel)) return false;

            auto p = node->parent.lock();
            if (p) {
                auto it = std::find(p->children.begin(), p->children.end(), node);
                if (it != p->children.end()) {
                    int index = (int)std::distance(p->children.begin(), it);
                    if (pseudo == ":first-child") return (index == 0);
                    if (pseudo == ":last-child") return (index == (int)p->children.size() - 1);
                    if (pseudo == ":nth-child(even)") return (index % 2 == 1);
                    if (pseudo == ":nth-child(odd)") return (index % 2 == 0);
                }
            }
            return true;
        }

        if (sel.rfind("#", 0) == 0) return node->id == sel.substr(1);
        if (sel.rfind(".", 0) == 0) {
            std::stringstream ss(sel.substr(1));
            std::string cls;
            while (std::getline(ss, cls, '.')) {
                if (!cls.empty() && !node->hasClass(cls)) return false;
            }
            return true;
        }
        return node->tag == sel;
    }

    bool matchesSelector(const std::shared_ptr<DOMNode> &node, const std::string &sel) const {
        size_t plusIdx = sel.rfind('+');
        if (plusIdx != std::string::npos) {
            std::string prevSel = trim(sel.substr(0, plusIdx));
            std::string targetSel = trim(sel.substr(plusIdx + 1));
            if (!matchSimpleSelector(node, targetSel)) return false;
            
            auto p = node->parent.lock();
            if (p) {
                auto it = std::find(p->children.begin(), p->children.end(), node);
                if (it != p->children.begin() && it != p->children.end()) {
                    auto prevSibling = *(it - 1);
                    return matchesSelector(prevSibling, prevSel);
                }
            }
            return false;
        }

        size_t gtIdx = sel.rfind('>');
        if (gtIdx != std::string::npos) {
            std::string parentSel = trim(sel.substr(0, gtIdx));
            std::string targetSel = trim(sel.substr(gtIdx + 1));
            if (!matchSimpleSelector(node, targetSel)) return false;
            auto p = node->parent.lock();
            return p ? matchesSelector(p, parentSel) : false;
        }

        size_t spaceIdx = sel.rfind(' ');
        if (spaceIdx != std::string::npos) {
            std::string parentSel = trim(sel.substr(0, spaceIdx));
            std::string targetSel = trim(sel.substr(spaceIdx + 1));
            if (!matchSimpleSelector(node, targetSel)) return false;
            auto curr = node->parent.lock();
            while (curr) {
                if (matchesSelector(curr, parentSel)) return true;
                curr = curr->parent.lock();
            }
            return false;
        }

        return matchSimpleSelector(node, sel);
    }

    void applyToNode(const std::shared_ptr<DOMNode> &node, float currentViewportW = 1920.0f) {
        // 🌟 Apply external stylesheet rules first, then re-apply node's style="..." attribute at highest priority!
        node->importantProps.clear(); // 🌟 Fresh cascade calculation: allows Like button to toggle freely!
        for (const auto &rule : rules) {
            if (currentViewportW > rule.mediaMaxWidth || currentViewportW < rule.mediaMinWidth) {
                continue;
            }

            if (matchesSelector(node, rule.selector)) {
                for (const auto &decl : rule.declarations) {
                    const std::string &p = decl.property;
                    const std::string &v = decl.value;

                    if (!decl.isImportant && node->importantProps[p]) {
                        continue;
                    }
                    if (decl.isImportant) {
                        node->importantProps[p] = true;
                    }

                    if (rule.isHover) {
                        if (p == "background-color" || p == "background") {
                            node->style.hasHoverBg = true;
                            node->style.hoverBackgroundColor = parseColor(v);
                        } else if (p == "color") {
                            node->style.hasHoverColor = true;
                            node->style.hoverColor = parseColor(v);
                        } else if (p == "border-color" || p == "border") {
                            node->style.hasHoverBorder = true;
                            node->style.hoverBorderColor = parseColor(v);
                        } else if (p == "transform") {
                            if (v.find("translateY") != std::string::npos) {
                                size_t o = v.find('(');
                                size_t c = v.rfind(')');
                                if (o != std::string::npos && c != std::string::npos && c > o) {
                                    node->style.hasHoverTransform = true;
                                    node->style.hoverTranslateY = parsePx(v.substr(o + 1, c - o - 1));
                                }
                            }
                        }
                        continue;
                    }

                    if (p == "position") {
                        if (v == "absolute") node->style.positionType = YGPositionTypeAbsolute;
                        else if (v == "fixed") { node->style.positionType = YGPositionTypeAbsolute; node->style.isFixed = true; }
                        else node->style.positionType = YGPositionTypeRelative;
                    } else if (p == "top") node->style.top = resolveUnit(v);
                    else if (p == "right") node->style.right = resolveUnit(v);
                    else if (p == "bottom") node->style.bottom = resolveUnit(v);
                    else if (p == "left") node->style.left = resolveUnit(v);
                    else if (p == "display") {
                        if (v == "flex") {
                            node->style.display = DisplayMode::Flex;
                            if (node->tag != "table" && node->tag != "thead" && node->tag != "tbody") {
                                node->style.flexDirection = YGFlexDirectionRow;
                            }
                        }
                        else if (v == "inline-block") node->style.display = DisplayMode::InlineBlock;
                        else if (v == "none") node->style.display = DisplayMode::None;
                        else node->style.display = DisplayMode::Block;
                    } else if (p == "flex-direction") {
                        node->style.flexDirection = (v == "row") ? YGFlexDirectionRow : YGFlexDirectionColumn;
                    } else if (p == "justify-content") {
                        if (v == "center") node->style.justifyContent = YGJustifyCenter;
                        else if (v == "space-between") node->style.justifyContent = YGJustifySpaceBetween;
                        else if (v == "space-around") node->style.justifyContent = YGJustifySpaceAround;
                        else if (v == "space-evenly") node->style.justifyContent = YGJustifySpaceEvenly;
                        else if (v == "flex-end") node->style.justifyContent = YGJustifyFlexEnd;
                        else node->style.justifyContent = YGJustifyFlexStart;
                    } else if (p == "align-items") {
                        if (v == "center") node->style.alignItems = YGAlignCenter;
                        else if (v == "flex-start") node->style.alignItems = YGAlignFlexStart;
                        else if (v == "flex-end") node->style.alignItems = YGAlignFlexEnd;
                        else node->style.alignItems = YGAlignStretch;
                    } else if (p == "align-self") {
                        if (v == "center") node->style.alignSelf = YGAlignCenter;
                        else if (v == "flex-start") node->style.alignSelf = YGAlignFlexStart;
                        else if (v == "flex-end") node->style.alignSelf = YGAlignFlexEnd;
                        else node->style.alignSelf = YGAlignAuto;
                    } else if (p == "flex") {
                        if (v == "1" || v.find("1 ") == 0 || v == "auto") {
                            node->style.flexGrow = 1.0f;
                            node->style.flexShrink = 1.0f;
                        } else if (v == "0" || v == "none") {
                            node->style.flexGrow = 0.0f;
                            node->style.flexShrink = 0.0f;
                        }
                    } else if (p == "flex-grow") {
                        node->style.flexGrow = resolveUnit(v, 0.0f);
                    } else if (p == "flex-shrink") {
                        node->style.flexShrink = resolveUnit(v, 1.0f);
                    } else if (p == "flex-wrap") {
                        if (v == "wrap") node->style.flexWrap = YGWrapWrap;
                        else if (v == "wrap-reverse") node->style.flexWrap = YGWrapWrapReverse;
                        else node->style.flexWrap = YGWrapNoWrap;
                    } else if (p == "min-width") { node->style.minWidth = resolveUnit(v); }
                    else if (p == "max-width") { node->style.maxWidth = resolveUnit(v); }
                    else if (p == "min-height") { node->style.minHeight = resolveUnit(v); }
                    else if (p == "max-height") { node->style.maxHeight = resolveUnit(v); }
                    else if (p == "z-index") { node->style.zIndex = atoi(v.c_str()); }
                    else if (p == "opacity") { node->style.opacity = strtof(v.c_str(), nullptr); }
                    else if (p == "width") {
                        if (v.find('%') != std::string::npos) { node->style.isWidthPercent = true; node->style.widthPercent = resolveUnit(v, 100.0f); }
                        else { node->style.isWidthPercent = false; node->style.width = resolveUnit(v, -1.0f); }
                    } else if (p == "height") {
                        if (v.find('%') != std::string::npos) { node->style.isHeightPercent = true; node->style.heightPercent = resolveUnit(v, 100.0f); }
                        else { node->style.isHeightPercent = false; node->style.height = resolveUnit(v, -1.0f); }
                    } else if (p == "padding") {
                        std::stringstream ss(v); std::string valStr; std::vector<float> vals;
                        while (ss >> valStr) vals.push_back(resolveUnit(valStr));
                        if (vals.size() == 1) node->style.padTop = node->style.padRight = node->style.padBottom = node->style.padLeft = vals[0];
                        else if (vals.size() == 2) { node->style.padTop = node->style.padBottom = vals[0]; node->style.padRight = node->style.padLeft = vals[1]; }
                        else if (vals.size() == 3) { node->style.padTop = vals[0]; node->style.padRight = node->style.padLeft = vals[1]; node->style.padBottom = vals[2]; }
                        else if (vals.size() >= 4) { node->style.padTop = vals[0]; node->style.padRight = vals[1]; node->style.padBottom = vals[2]; node->style.padLeft = vals[3]; }
                    } else if (p == "padding-left") node->style.padLeft = resolveUnit(v);
                    else if (p == "padding-right") node->style.padRight = resolveUnit(v);
                    else if (p == "padding-top") node->style.padTop = resolveUnit(v);
                    else if (p == "padding-bottom") node->style.padBottom = resolveUnit(v);
                    else if (p == "margin") {
                        if (v.find("auto") != std::string::npos) {
                            node->style.isMarginLeftAuto = true;
                            node->style.isMarginRightAuto = true;
                        }
                        std::stringstream ss(v); std::string valStr; std::vector<float> vals;
                        while (ss >> valStr) {
                            if (valStr == "auto") vals.push_back(0.0f);
                            else vals.push_back(resolveUnit(valStr));
                        }
                        if (vals.size() == 1) node->style.marginTop = node->style.marginRight = node->style.marginBottom = node->style.marginLeft = vals[0];
                        else if (vals.size() == 2) { node->style.marginTop = node->style.marginBottom = vals[0]; node->style.marginRight = node->style.marginLeft = vals[1]; }
                        else if (vals.size() >= 4) { node->style.marginTop = vals[0]; node->style.marginRight = vals[1]; node->style.marginBottom = vals[2]; node->style.marginLeft = vals[3]; }
                    } else if (p == "margin-top") node->style.marginTop = resolveUnit(v);
                    else if (p == "margin-bottom") node->style.marginBottom = resolveUnit(v);
                    else if (p == "margin-left") {
                        if (v == "auto") node->style.isMarginLeftAuto = true;
                        else node->style.marginLeft = resolveUnit(v);
                    } else if (p == "margin-right") {
                        if (v == "auto") node->style.isMarginRightAuto = true;
                        else node->style.marginRight = resolveUnit(v);
                    } else if (p == "gap") node->style.gap = resolveUnit(v, 0.0f);
                    else if (p == "background-color" || p == "background") {
                        if (v.find("linear-gradient") != std::string::npos) {
                            node->style.isGradient = true;
                            node->style.isGradientHorizontal = (v.find("to right") != std::string::npos || v.find("90deg") != std::string::npos);
                            size_t pOpen = v.find('(');
                            size_t pClose = v.rfind(')');
                            if (pOpen != std::string::npos && pClose != std::string::npos && pClose > pOpen) {
                                std::string params = v.substr(pOpen + 1, pClose - pOpen - 1);
                                std::stringstream ssParams(params);
                                std::string token;
                                std::vector<std::string> colorStops;
                                while (std::getline(ssParams, token, ',')) {
                                    token = trim(token);
                                    if (token.find("to ") == std::string::npos && token.find("deg") == std::string::npos && !token.empty()) {
                                        colorStops.push_back(token);
                                    }
                                }
                                if (colorStops.size() >= 2) {
                                    node->style.gradientStart = parseColor(colorStops[0]);
                                    node->style.gradientEnd = parseColor(colorStops[1]);
                                } else if (colorStops.size() == 1) {
                                    node->style.gradientStart = node->style.gradientEnd = parseColor(colorStops[0]);
                                }
                            }
                        } else {
                            node->style.isGradient = false;
                            node->style.backgroundColor = parseColor(v);
                        }
                    } else if (p == "color") node->style.color = parseColor(v);
                    else if (p == "border-radius") node->style.borderRadius = resolveUnit(v, 0.0f);
                    else if (p == "border-right") {
                        node->style.borderRightWidth = 1.0f;
                        std::stringstream ss(v); std::string t;
                        while (ss >> t) {
                            if (t[0] == '#' || t.find("rgb") != std::string::npos) node->style.borderRightColor = parseColor(t);
                            else if (t.find("px") != std::string::npos || t.find("rem") != std::string::npos) node->style.borderRightWidth = resolveUnit(t, 1.0f);
                        }
                    } else if (p == "border-bottom") {
                        node->style.borderBottomWidth = 1.0f;
                        std::stringstream ss(v); std::string t;
                        while (ss >> t) {
                            if (t[0] == '#' || t.find("rgb") != std::string::npos) node->style.borderBottomColor = parseColor(t);
                            else if (t.find("px") != std::string::npos || t.find("rem") != std::string::npos) node->style.borderBottomWidth = resolveUnit(t, 1.0f);
                        }
                    } else if (p == "border-top") {
                        node->style.borderTopWidth = 1.0f;
                        std::stringstream ss(v); std::string t;
                        while (ss >> t) {
                            if (t[0] == '#' || t.find("rgb") != std::string::npos) node->style.borderTopColor = parseColor(t);
                            else if (t.find("px") != std::string::npos || t.find("rem") != std::string::npos) node->style.borderTopWidth = resolveUnit(t, 1.0f);
                        }
                    } else if (p == "border-left") {
                        node->style.borderLeftWidth = 1.0f;
                        std::stringstream ss(v); std::string t;
                        while (ss >> t) {
                            if (t[0] == '#' || t.find("rgb") != std::string::npos) node->style.borderLeftColor = parseColor(t);
                            else if (t.find("px") != std::string::npos || t.find("rem") != std::string::npos) node->style.borderLeftWidth = resolveUnit(t, 1.0f);
                        }
                    } else if (p == "border") {
                        node->style.borderWidth = 1.0f;
                        std::stringstream ss(v); std::string t;
                        while (ss >> t) {
                            if (t[0] == '#' || t.find("rgb") != std::string::npos) node->style.borderColor = parseColor(t);
                            else if (t.find("px") != std::string::npos || t.find("rem") != std::string::npos) node->style.borderWidth = resolveUnit(t, 1.0f);
                        }
                    } else if (p == "box-shadow") { node->style.shadow.hasShadow = true; node->style.shadow.blurRadius = 16.0f; node->style.shadow.alpha = 50; }
                    else if (p == "font-size") {
                        node->style.fontSize = resolveUnit(v, 14.0f);
                        node->style.lineHeight = node->style.fontSize * 1.35f;
                    } else if (p == "line-height") {
                        node->style.lineHeight = resolveUnit(v, node->style.fontSize * 1.35f);
                    } else if (p == "font-weight") { node->style.isBold = (v == "bold" || parsePx(v) >= 600); }
                    else if (p == "fill") { node->style.color = parseColor(v); }
                    else if (p == "stroke") { node->style.borderColor = parseColor(v); }
                    else if (p == "stroke-width") { node->style.borderWidth = resolveUnit(v, 1.5f); }
                    else if (p == "text-align") {
                        if (v == "center") node->style.textAlign = TextAlign::Center;
                        else if (v == "right") node->style.textAlign = TextAlign::Right;
                        else node->style.textAlign = TextAlign::Left;
                    }
                }
            }
        }
    }
};

} // namespace UIEngine

#endif // ROOPM_CSS_PARSER_H
