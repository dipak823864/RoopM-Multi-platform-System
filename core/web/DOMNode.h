#ifndef ROOPM_DOM_NODE_H
#define ROOPM_DOM_NODE_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <stdint.h>
#include "vendor/yoga/yoga/Yoga.h"

namespace UIEngine {

struct SVGPathItem {
    std::string d;
    uint32_t fill = 0x00000000;
    uint32_t stroke = 0x00000000;
    float strokeWidth = 1.5f;
};

struct TextRun {
    std::string text;
    bool isBold = false;
    bool isItalic = false;
    bool isUnderlined = false;
    uint32_t color = 0;
};

enum class DisplayMode { Block, Flex, InlineBlock, None };
enum class TextAlign { Left, Center, Right };

struct CSSBoxShadow {
    bool hasShadow = false;
    float offsetX = 0.0f;
    float offsetY = 4.0f;
    float blurRadius = 12.0f;
    uint8_t alpha = 40;
};

struct CSSComputedStyle {
    DisplayMode display = DisplayMode::Block;
    YGFlexDirection flexDirection = YGFlexDirectionColumn;
    YGJustify justifyContent = YGJustifyFlexStart;
    YGAlign alignItems = YGAlignStretch;
    YGAlign alignSelf = YGAlignAuto;
    YGWrap flexWrap = YGWrapNoWrap;
    
    YGPositionType positionType = YGPositionTypeRelative;
    bool isFixed = false;
    float top = -100000.0f;
    float right = -100000.0f;
    float bottom = -100000.0f;
    float left = -100000.0f;

    float flexGrow = 0.0f;
    float flexShrink = 1.0f;

    float width = -1.0f;
    float height = -1.0f;
    bool isWidthPercent = false;
    bool isHeightPercent = false;
    float widthPercent = 100.0f;
    float heightPercent = 100.0f;

    float minWidth = -1.0f;
    float maxWidth = -1.0f;
    float minHeight = -1.0f;
    float maxHeight = -1.0f;
    int zIndex = 0;
    float opacity = 1.0f;

    float padTop = 0.0f, padRight = 0.0f, padBottom = 0.0f, padLeft = 0.0f;
    float marginTop = 0.0f, marginRight = 0.0f, marginBottom = 0.0f, marginLeft = 0.0f;
    bool isMarginLeftAuto = false;
    bool isMarginRightAuto = false;
    float gap = 0.0f;

    uint32_t backgroundColor = 0x00000000;
    uint32_t color = 0xFFF8FAFC;
    float borderRadius = 0.0f;
    float borderWidth = 0.0f;
    uint32_t borderColor = 0x00000000;

    // 🌟 W3C 4-Side Border Support
    float borderTopWidth = 0.0f, borderRightWidth = 0.0f, borderBottomWidth = 0.0f, borderLeftWidth = 0.0f;
    uint32_t borderTopColor = 0, borderRightColor = 0, borderBottomColor = 0, borderLeftColor = 0;

    CSSBoxShadow shadow;

    // CSS Linear Gradient
    bool isGradient = false;
    uint32_t gradientStart = 0x00000000;
    uint32_t gradientEnd = 0x00000000;
    bool isGradientHorizontal = false;

    float fontSize = 14.0f;
    float lineHeight = 19.0f;
    TextAlign textAlign = TextAlign::Left;
    bool isBold = false;
    bool isClickable = false;
    bool overflowHidden = false;

    // Hover Overrides
    bool hasHoverBg = false;
    uint32_t hoverBackgroundColor = 0x00000000;
    bool hasHoverColor = false;
    uint32_t hoverColor = 0x00000000;
    bool hasHoverBorder = false;
    uint32_t hoverBorderColor = 0x00000000;

    bool hasHoverTransform = false;
    float translateY = 0.0f;
    float hoverTranslateY = 0.0f;
};

class DOMNode : public std::enable_shared_from_this<DOMNode> {
public:
    std::string tag;
    std::string id;
    std::vector<std::string> classes;
    std::string innerText;
    std::map<std::string, std::string> attributes;
    std::map<std::string, bool> importantProps;
    std::vector<TextRun> textRuns;

    // 🌟 W3C DOM EventTarget Architecture (Pointer & UID Based)
    uint64_t uid = 0;
    struct DOMEventListener {
        std::string type;
        uint32_t listenerId = 0;
    };
    std::vector<DOMEventListener> eventListeners;

    CSSComputedStyle style;
    YGNodeRef yogaNode = nullptr;

    float layoutX = 0.0f;
    float layoutY = 0.0f;
    float layoutWidth = 0.0f;
    float layoutHeight = 0.0f;

    // Interactive & Form States
    bool isHovered = false;
    bool isFocused = false;
    bool isInput = false;
    bool isCheckbox = false;
    bool isRadio = false;
    std::string radioGroup = "";
    bool isChecked = false;
    bool isSelect = false;
    bool isDropdownOpen = false;
    std::vector<std::string> selectOptions;
    int selectedOptionIndex = 0;
    std::string href = "";
    std::string inputValue = "";
    std::string placeholder = "";
    float caretTimer = 0.0f;
    bool showCaret = true;

    // HTML5 <canvas> States
    bool isCanvas = false;
    int canvasW = 0;
    int canvasH = 0;
    std::vector<uint32_t> canvasPixels;
    uint32_t ctxFillColor = 0xFF10B981;

    // Image & SVG States
    bool isImage = false;
    std::string imageSrc = "";
    bool isSvg = false;
    float svgViewBoxW = 24.0f;
    float svgViewBoxH = 24.0f;
    std::vector<SVGPathItem> svgPaths;
    std::vector<uint32_t> svgRasterCache;
    int cachedSvgW = 0;
    int cachedSvgH = 0;
    uint32_t cachedSvgCol = 0;

    // 🌟 60 FPS Smooth CSS Color & Transform LERP State
    float animBgR = -1.0f, animBgG = -1.0f, animBgB = -1.0f, animBgA = -1.0f;
    float animBorderR = -1.0f, animBorderG = -1.0f, animBorderB = -1.0f, animBorderA = -1.0f;
    float animTranslateY = 0.0f;
    bool hasHoverBorder = false;
    uint32_t hoverBorderColor = 0x00000000;

    std::vector<std::string> wrappedLines;

    std::weak_ptr<DOMNode> parent;
    std::vector<std::shared_ptr<DOMNode>> children;

    DOMNode(const std::string &tagName = "div") : tag(tagName) {
        static uint64_t g_nextUid = 1;
        uid = g_nextUid++;}

    ~DOMNode() {
        if (yogaNode) {
            YGNodeFree(yogaNode);
            yogaNode = nullptr;
        }
    }

    bool hasClass(const std::string &cls) const {
        for (const auto &c : classes) {
            if (c == cls) return true;
        }
        return false;
    }

    void addChild(std::shared_ptr<DOMNode> child) {
        child->parent = weak_from_this();
        children.push_back(child);
    }
};

} // namespace UIEngine

#endif // ROOPM_DOM_NODE_H
