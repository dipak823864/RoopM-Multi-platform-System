#ifndef ROOPM_TEXT_FIELD_H
#define ROOPM_TEXT_FIELD_H

#include "core/base/UIElement.h"
#include <functional>

namespace UIEngine {

class TextField : public UIElement {
public:
    std::string text = "";
    std::string placeholder = "Enter text...";
    size_t caretPosition = 0;
    bool isFocused = false;
    float blinkTimer = 0.0f;
    bool showCaret = true;

    std::function<void(const std::string &)> onTextChanged;

    TextField(const std::string &id = "", const std::string &placeholderText = "Enter text...")
        : UIElement(id), placeholder(placeholderText) {
        m_bounds.width = 220.0f;
        m_bounds.height = 36.0f;
    }

    void insertText(const std::string &str) {
        if (caretPosition > text.length()) caretPosition = text.length();
        text.insert(caretPosition, str);
        caretPosition += str.length();
        markRenderDirty();
        if (onTextChanged) onTextChanged(text);
    }

    void backspace() {
        if (caretPosition > 0 && !text.empty()) {
            caretPosition--;
            text.erase(caretPosition, 1);
            markRenderDirty();
            if (onTextChanged) onTextChanged(text);
        }
    }

    void onUpdate(float dt) override {
        UIElement::onUpdate(dt);
        if (isFocused) {
            blinkTimer += dt;
            if (blinkTimer >= 0.5f) {
                blinkTimer = 0.0f;
                showCaret = !showCaret;
                markRenderDirty();
            }
        }
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;

        if (event.type == EventType::PointerDown) {
            isFocused = m_bounds.contains(event.x, event.y);
            if (isFocused) {
                setState(WidgetState::Focused | WidgetState::FocusVisible);
                caretPosition = text.length(); // Move to end on simple click
                showCaret = true;
                blinkTimer = 0.0f;
            } else {
                setState(WidgetState::Normal);
            }
            markRenderDirty();
            return isFocused;
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_TEXT_FIELD_H
