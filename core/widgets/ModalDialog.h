#ifndef ROOPM_MODAL_DIALOG_H
#define ROOPM_MODAL_DIALOG_H

#include "core/base/UIElement.h"
#include <functional>

namespace UIEngine {

class ModalDialog : public UIElement {
public:
    bool isOpen = false;
    std::string title = "Dialog";
    std::string message = "Message content";
    
    std::function<void()> onConfirm;
    std::function<void()> onCancel;

    ModalDialog(const std::string &id = "", const std::string &dialogTitle = "Dialog", const std::string &dialogMsg = "")
        : UIElement(id), title(dialogTitle), message(dialogMsg) {
        m_bounds.width = 380.0f;
        m_bounds.height = 180.0f;
    }

    void show() { isOpen = true; markRenderDirty(); }
    void dismiss() { isOpen = false; markRenderDirty(); if (onCancel) onCancel(); }

    bool onEvent(UIEvent &event) override {
        if (!isOpen) return false;
        // Trap all pointer events to prevent clicking background when modal is open
        if (event.type == EventType::PointerDown || event.type == EventType::PointerUp) {
            event.handled = true;
            return true;
        }
        return false;
    }
};

} // namespace UIEngine

#endif // ROOPM_MODAL_DIALOG_H
