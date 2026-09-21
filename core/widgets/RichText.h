#ifndef ROOPM_RICH_TEXT_H
#define ROOPM_RICH_TEXT_H

#include "core/base/UIElement.h"
#include <vector>
#include <string>
#include <functional>

namespace UIEngine {

struct TextSpan {
    std::string text;
    uint32_t color = 0xFFFFFFFF;
    float fontSize = 14.0f;
    bool isBold = false;
    bool isUnderlined = false;
    std::function<void()> onClick;
};

class RichText : public UIElement {
public:
    std::vector<TextSpan> spans;
    float lineHeight = 20.0f;

    RichText(const std::string &id = "") : UIElement(id) {
        m_bounds.width = 300.0f;
        m_bounds.height = 40.0f;
    }

    void addSpan(const std::string &txt, uint32_t col, float sz = 14.0f, bool bold = false, bool underline = false, std::function<void()> clickCb = nullptr) {
        spans.push_back({txt, col, sz, bold, underline, clickCb});
        markLayoutDirty();
    }

    size_t getTotalCharacterCount() const {
        size_t count = 0;
        for (const auto &span : spans) count += span.text.length();
        return count;
    }
};

} // namespace UIEngine

#endif // ROOPM_RICH_TEXT_H
