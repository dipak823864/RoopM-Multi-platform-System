#ifndef ROOPM_HTML_DOM_BUILDER_H
#define ROOPM_HTML_DOM_BUILDER_H

#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include "vendor/lexbor/html/parser.h"
#include "vendor/lexbor/html/interfaces/element.h"
#include "vendor/lexbor/dom/interfaces/element.h"
#include "core/web/DOMNode.h"
#include "core/web/CSSParser.h"

namespace UIEngine {

class HTMLDOMBuilder {
public:
    static std::vector<std::shared_ptr<DOMNode>> parseFragment(const std::string &htmlFragment, CSSParser &cssParser) {
        std::vector<std::shared_ptr<DOMNode>> nodes;
        std::string wrappedHtml = "<body>" + htmlFragment + "</body>";
        auto root = build(wrappedHtml, cssParser);
        if (root) {
            for (const auto &child : root->children) {
                nodes.push_back(child);
            }
        }
        return nodes;
    }

    static std::shared_ptr<DOMNode> build(const std::string &htmlContent, CSSParser &cssParser) {
        lxb_html_parser_t *parser = lxb_html_parser_create();
        lxb_html_parser_init(parser);
        lxb_html_document_t *doc = lxb_html_parse(parser, (const lxb_char_t*)htmlContent.c_str(), htmlContent.length());
        if (!doc) {
            lxb_html_parser_destroy(parser);
            return nullptr;
        }

        if (doc->head) {
            lxb_dom_node_t *headNode = lxb_dom_interface_node(doc->head);
            lxb_dom_node_t *hChild = lxb_dom_node_first_child(headNode);
            while (hChild) {
                if (hChild->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                    size_t cLen = 0;
                    const lxb_char_t *cName = lxb_dom_element_qualified_name(lxb_dom_interface_element(hChild), &cLen);
                    if (cName && std::string((const char*)cName, cLen) == "style") {
                        size_t sLen = 0;
                        const lxb_char_t *sText = lxb_dom_node_text_content(hChild, &sLen);
                        if (sText && sLen > 0) {
                            cssParser.parse(std::string((const char*)sText, sLen));
                        }
                    }
                }
                hChild = hChild->next;
            }
        }

        lxb_dom_node_t *body = lxb_dom_interface_node(doc->body);
        std::shared_ptr<DOMNode> rootNode = convertLexborNode(body, cssParser, 0xFFF8FAFC, 14.0f);

        lxb_html_document_destroy(doc);
        lxb_html_parser_destroy(parser);
        return rootNode;
    }

private:
    static std::string normalizeWhitespace(const std::string &input) {
        std::string result = "";
        bool inSpace = false;
        for (char c : input) {
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                if (!inSpace && !result.empty()) {
                    result += ' ';
                    inSpace = true;
                }
            } else {
                result += c;
                inSpace = false;
            }
        }
        return CSSParser::trim(result);
    }

    static bool isInlineTextTag(const std::string &rawTag) {
        std::string tag = rawTag;
        for (char &c : tag) c = tolower(c);
        return (tag == "b" || tag == "strong" || tag == "i" || tag == "em" || 
                tag == "small" || tag == "u" || tag == "code" || tag == "span");
    }

    static std::string getSubtreeText(lxb_dom_node_t *lxbNode) {
        if (!lxbNode) return "";
        std::string result = "";

        if (lxbNode->type == LXB_DOM_NODE_TYPE_TEXT) {
            size_t txtLen = 0;
            const lxb_char_t *txt = lxb_dom_node_text_content(lxbNode, &txtLen);
            if (txt && txtLen > 0) result += std::string((const char*)txt, txtLen);
        }

        lxb_dom_node_t *child = lxb_dom_node_first_child(lxbNode);
        while (child) {
            result += getSubtreeText(child);
            child = child->next;
        }
        return normalizeWhitespace(result);
    }

    static std::shared_ptr<DOMNode> convertLexborNode(lxb_dom_node_t *lxbNode, CSSParser &cssParser, uint32_t inheritedColor, float inheritedFontSize) {
        if (!lxbNode) return nullptr;

        std::string tagName = "div";
        if (lxbNode->type == LXB_DOM_NODE_TYPE_ELEMENT) {
            size_t len = 0;
            const lxb_char_t *name = lxb_dom_element_qualified_name(lxb_dom_interface_element(lxbNode), &len);
            if (name) tagName = std::string((const char*)name, len);
        }

        auto node = std::make_shared<DOMNode>(tagName);
        node->style.color = inheritedColor;
        node->style.fontSize = inheritedFontSize;
        node->style.lineHeight = inheritedFontSize * 1.35f;

        // W3C Tag Defaults
        if (tagName == "div" || tagName == "header" || tagName == "main" || tagName == "section" || tagName == "footer" || tagName == "nav" || tagName == "aside") {
            node->style.display = DisplayMode::Block;
        } else if (tagName == "h1") {
            node->style.display = DisplayMode::Block;
            node->style.fontSize = 24.0f;
            node->style.lineHeight = 32.0f;
            node->style.isBold = true;
        } else if (tagName == "h2") {
            node->style.display = DisplayMode::Block;
            node->style.fontSize = 18.0f;
            node->style.lineHeight = 26.0f;
            node->style.isBold = true;
        } else if (tagName == "h3") {
            node->style.display = DisplayMode::Block;
            node->style.fontSize = 16.0f;
            node->style.lineHeight = 22.0f;
            node->style.isBold = true;
        } else if (tagName == "h4") {
            node->style.display = DisplayMode::Block;
            node->style.fontSize = 14.0f;
            node->style.lineHeight = 20.0f;
            node->style.isBold = true;
        } else if (tagName == "p") {
            node->style.display = DisplayMode::Block;
            node->style.fontSize = 13.5f;
            node->style.lineHeight = 20.0f;
            node->style.color = 0xFF94A3B8;
        } else if (tagName == "button") {
            node->style.display = DisplayMode::InlineBlock;
            node->style.alignSelf = YGAlignFlexStart;
            node->style.flexDirection = YGFlexDirectionRow;
            node->style.alignItems = YGAlignCenter;
            node->style.justifyContent = YGJustifyCenter;
            node->style.gap = 6.0f;
            node->style.textAlign = TextAlign::Center;
            node->style.isClickable = true;
        } else if (tagName == "img") {
            node->isImage = true;
            node->style.display = DisplayMode::InlineBlock;
        } else if (tagName == "span" || tagName == "label" || tagName == "a") {
            node->style.display = DisplayMode::InlineBlock;
            node->style.isClickable = (tagName == "a");
        } else if (tagName == "select") {
            node->isSelect = true;
            node->style.display = DisplayMode::InlineBlock;
        } else if (tagName == "input") {
            node->isInput = true;
            node->style.display = DisplayMode::InlineBlock;
        } 
        // 🌟 1. RESTORED: HTML5 <canvas> Buffer Allocator!
        else if (tagName == "canvas") {
            node->isCanvas = true;
            node->style.display = DisplayMode::InlineBlock;
        }
        // 🌟 2. RESTORED: W3C <svg> Vector Graphics Container!
        else if (tagName == "svg") {
            node->isSvg = true;
            node->style.display = DisplayMode::InlineBlock;
            node->style.width = 24.0f;
            node->style.height = 24.0f;
        } else if (tagName == "table") {
            node->style.display = DisplayMode::Flex;
            node->style.flexDirection = YGFlexDirectionColumn;
            node->style.isWidthPercent = true;
            node->style.widthPercent = 100.0f;
        } else if (tagName == "thead" || tagName == "tbody") {
            node->style.display = DisplayMode::Flex;
            node->style.flexDirection = YGFlexDirectionColumn;
            node->style.isWidthPercent = true;
            node->style.widthPercent = 100.0f;
        } else if (tagName == "tr") {
            node->style.display = DisplayMode::Flex;
            node->style.flexDirection = YGFlexDirectionRow;
            node->style.isWidthPercent = true;
            node->style.widthPercent = 100.0f;
            node->style.alignItems = YGAlignStretch;
        } else if (tagName == "th") {
            node->style.display = DisplayMode::Flex;
            node->style.flexDirection = YGFlexDirectionColumn;
            node->style.justifyContent = YGJustifyCenter;
            node->style.isWidthPercent = true;
            node->style.widthPercent = 25.0f;
            node->style.fontSize = 13.0f;
            node->style.isBold = true;
            node->style.textAlign = TextAlign::Center;
        } else if (tagName == "td") {
            node->style.display = DisplayMode::Flex;
            node->style.flexDirection = YGFlexDirectionColumn;
            node->style.justifyContent = YGJustifyCenter;
            node->style.isWidthPercent = true;
            node->style.widthPercent = 25.0f;
            node->style.fontSize = 13.0f;
        }

        // Attribute Extraction
        if (lxbNode->type == LXB_DOM_NODE_TYPE_ELEMENT) {
            lxb_dom_element_t *el = lxb_dom_interface_element(lxbNode);

            lxb_dom_attr_t *attr = lxb_dom_element_first_attribute(el);
            while (attr) {
                size_t kLen = 0, vLen = 0;
                const lxb_char_t *k = lxb_dom_attr_qualified_name(attr, &kLen);
                const lxb_char_t *v = lxb_dom_attr_value(attr, &vLen);
                if (k && kLen > 0) {
                    std::string key((const char*)k, kLen);
                    std::string val = (v && vLen > 0) ? std::string((const char*)v, vLen) : "";
                    node->attributes[key] = val;
                }
                attr = lxb_dom_element_next_attribute(attr);
            }

            size_t idLen = 0;
            const lxb_char_t *idVal = lxb_dom_element_id(el, &idLen);
            if (idVal && idLen > 0) node->id = std::string((const char*)idVal, idLen);

            size_t classLen = 0;
            const lxb_char_t *classVal = lxb_dom_element_class(el, &classLen);
            if (classVal && classLen > 0) {
                std::string classStr((const char*)classVal, classLen);
                std::stringstream ss(classStr);
                std::string item;
                while (ss >> item) node->classes.push_back(item);
            }

            size_t srcLen = 0;
            const lxb_char_t *srcVal = lxb_dom_element_get_attribute(el, (const lxb_char_t*)"src", 3, &srcLen);
            if (srcVal && srcLen > 0) node->imageSrc = std::string((const char*)srcVal, srcLen);

            size_t hrefLen = 0;
            const lxb_char_t *hrefVal = lxb_dom_element_get_attribute(el, (const lxb_char_t*)"href", 4, &hrefLen);
            if (hrefVal && hrefLen > 0) node->href = std::string((const char*)hrefVal, hrefLen);

            size_t typeLen = 0;
            const lxb_char_t *typeVal = lxb_dom_element_get_attribute(el, (const lxb_char_t*)"type", 4, &typeLen);
            if (typeVal && typeLen > 0) {
                std::string tStr((const char*)typeVal, typeLen);
                if (tStr == "checkbox") {
                    node->isCheckbox = true;
                    node->isInput = false;
                    node->style.width = 16.0f;
                    node->style.height = 16.0f;
                    node->style.borderRadius = 3.0f;
                    node->style.backgroundColor = 0xFFFFFFFF;
                    node->style.borderColor = 0xFF94A3B8;
                    node->style.borderWidth = 1.5f;
                } else if (tStr == "radio") {
                    node->isRadio = true;
                    node->isInput = false;
                    node->style.width = 16.0f;
                    node->style.height = 16.0f;
                    node->style.borderRadius = 8.0f;
                    node->style.backgroundColor = 0xFFFFFFFF;
                    node->style.borderColor = 0xFF94A3B8;
                    node->style.borderWidth = 1.5f;
                    size_t nameLen = 0;
                    const lxb_char_t *nameVal = lxb_dom_element_get_attribute(el, (const lxb_char_t*)"name", 4, &nameLen);
                    if (nameVal && nameLen > 0) node->radioGroup = std::string((const char*)nameVal, nameLen);
                }
            }

            size_t phLen = 0;
            const lxb_char_t *phVal = lxb_dom_element_get_attribute(el, (const lxb_char_t*)"placeholder", 11, &phLen);
            if (phVal && phLen > 0) node->placeholder = std::string((const char*)phVal, phLen);

            // 🌟 Allocate Canvas Memory Buffer
            if (tagName == "canvas") {
                node->isCanvas = true;
                size_t wLen = 0, hLen = 0;
                const lxb_char_t *wVal = lxb_dom_element_get_attribute(el, (const lxb_char_t*)"width", 5, &wLen);
                const lxb_char_t *hVal = lxb_dom_element_get_attribute(el, (const lxb_char_t*)"height", 6, &hLen);
                int cw = (wVal && wLen > 0) ? atoi((const char*)wVal) : 480;
                int ch = (hVal && hLen > 0) ? atoi((const char*)hVal) : 90;
                node->canvasW = cw;
                node->canvasH = ch;
                node->canvasPixels.assign(cw * ch, 0xFF030712);
            }

            size_t vbLen = 0;
            const lxb_char_t *vbVal = lxb_dom_element_get_attribute(el, (const lxb_char_t*)"viewBox", 7, &vbLen);
            if (vbVal && vbLen > 0) {
                float vx, vy, vw, vh;
                if (sscanf((const char*)vbVal, "%f %f %f %f", &vx, &vy, &vw, &vh) == 4 && vw > 0 && vh > 0) {
                    node->svgViewBoxW = vw;
                    node->svgViewBoxH = vh;
                }
            }

            size_t styleLen = 0;
            const lxb_char_t *styleVal = lxb_dom_element_get_attribute(el, (const lxb_char_t*)"style", 5, &styleLen);
            if (styleVal && styleLen > 0) {
                std::string inlineCss = "* { " + std::string((const char*)styleVal, styleLen) + " }";
                CSSParser inlineParser;
                inlineParser.parse(inlineCss);
                inlineParser.applyToNode(node);
            }
        }

        // 🌟 Direct Child Processing & SVG Path Gathering
        lxb_dom_node_t *child = lxb_dom_node_first_child(lxbNode);
        while (child) {
            if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                lxb_dom_element_t *cEl = lxb_dom_interface_element(child);

                // If this is <svg>, directly collect all child <path> items into node->svgPaths!
                if (node->isSvg) {
                    size_t cLen = 0;
                    const lxb_char_t *cName = lxb_dom_element_qualified_name(cEl, &cLen);
                    if (cName && std::string((const char*)cName, cLen) == "path") {
                        size_t dLen = 0;
                        const lxb_char_t *dVal = lxb_dom_element_get_attribute(cEl, (const lxb_char_t*)"d", 1, &dLen);
                        if (dVal && dLen > 0) {
                            SVGPathItem item;
                            item.d = std::string((const char*)dVal, dLen);
                            
                            size_t fLen = 0;
                            const lxb_char_t *fVal = lxb_dom_element_get_attribute(cEl, (const lxb_char_t*)"fill", 4, &fLen);
                            if (fVal && fLen > 0) item.fill = CSSParser::parseColor(std::string((const char*)fVal, fLen));
                            else item.fill = node->style.color;

                            size_t sLen = 0;
                            const lxb_char_t *sVal = lxb_dom_element_get_attribute(cEl, (const lxb_char_t*)"stroke", 6, &sLen);
                            if (sVal && sLen > 0) item.stroke = CSSParser::parseColor(std::string((const char*)sVal, sLen));

                            node->svgPaths.push_back(item);
                        }
                    }
                }

                size_t cLen = 0;
                const lxb_char_t *cName = lxb_dom_element_qualified_name(cEl, &cLen);
                std::string cTagName = cName ? std::string((const char*)cName, cLen) : "";
                std::transform(cTagName.begin(), cTagName.end(), cTagName.begin(), ::tolower);

                bool isPhrasing = (cTagName == "b" || cTagName == "strong" || cTagName == "small" ||
                                  cTagName == "i" || cTagName == "em" || cTagName == "u" || cTagName == "s");

                if (isPhrasing && node->tag == "p") {
                    std::string raw = getSubtreeText(child);
                    TextRun r;
                    r.text = raw;
                    r.isBold = (cTagName == "b" || cTagName == "strong");
                    r.isItalic = (cTagName == "i" || cTagName == "em");
                    r.isUnderlined = (cTagName == "u");
                    r.color = 0;
                    node->textRuns.push_back(r);
                    node->innerText += raw;
                } else {
                    auto childNode = convertLexborNode(child, cssParser, node->style.color, node->style.fontSize);
                    if (childNode) {
                        if (node->isSelect && childNode->tag == "option") {
                            childNode->style.display = DisplayMode::None;
                            if (!childNode->innerText.empty()) {
                                node->selectOptions.push_back(childNode->innerText);
                            }
                        }
                        node->addChild(childNode);
                    }
                }
            } else if (child->type == LXB_DOM_NODE_TYPE_TEXT) {
                size_t txtLen = 0;
                const lxb_char_t *txt = lxb_dom_node_text_content(child, &txtLen);
                if (txt && txtLen > 0) {
                    std::string raw((const char*)txt, txtLen);
                    std::string normalized = normalizeWhitespace(raw);
                    if (!normalized.empty()) {
                        // Plain nodes (buttons, nav items) stay clean with textRuns.empty() so they center perfectly!
                        if (node->tag == "p") {
                            TextRun r;
                            r.text = normalized;
                            r.isBold = node->style.isBold;
                            r.isItalic = false;
                            r.isUnderlined = false;
                            r.color = 0;
                            node->textRuns.push_back(r);
                        }

                        if (!node->innerText.empty() && node->innerText.back() != ' ') node->innerText += " ";
                        node->innerText += normalized;
                    }
                }
            }
            child = child->next;
        }

        // 🌟 W3C Inline Flow: Containers with only inline elements (radio + label) flow horizontally!
        if (node->style.display == DisplayMode::Block && !node->children.empty()) {
            bool hasBlock = false;
            for (const auto &c : node->children) {
                if (c->tag == "div" || c->tag == "p" || c->tag == "h1" || c->tag == "h2" || c->tag == "h3" ||
                    c->tag == "h4" || c->tag == "h5" || c->tag == "h6" || c->tag == "section" || c->tag == "header" || c->tag == "table") {
                    hasBlock = true;
                    break;
                }
            }
            if (!hasBlock) {
                node->style.flexDirection = YGFlexDirectionRow;
                node->style.alignItems = YGAlignCenter;
                node->style.gap = 6.0f;
            }
        }

        cssParser.applyToNode(node);
        return node;
    }
};

} // namespace UIEngine

#endif // ROOPM_HTML_DOM_BUILDER_H
