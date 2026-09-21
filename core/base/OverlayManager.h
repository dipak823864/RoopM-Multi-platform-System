#ifndef ROOPM_OVERLAY_MANAGER_H
#define ROOPM_OVERLAY_MANAGER_H

#include <vector>
#include <functional>
#include "core/base/UIElement.h"

namespace UIEngine {

struct OverlayEntry {
    std::function<void()> renderCallback;
    std::function<bool(UIEvent &)> eventCallback;
};

class OverlayManager {
private:
    std::vector<OverlayEntry> m_overlays;

public:
    static OverlayManager &instance() {
        static OverlayManager s_inst;
        return s_inst;
    }

    void pushOverlay(std::function<void()> renderCb, std::function<bool(UIEvent &)> eventCb) {
        m_overlays.push_back({renderCb, eventCb});
    }

    bool dispatchEvent(UIEvent &event) {
        for (auto it = m_overlays.rbegin(); it != m_overlays.rend(); ++it) {
            if (it->eventCallback && it->eventCallback(event)) {
                event.handled = true;
                return true;
            }
        }
        return false;
    }

    void renderOverlays() {
        for (auto &entry : m_overlays) {
            if (entry.renderCallback) entry.renderCallback();
        }
        m_overlays.clear(); // Cleared every frame after top-pass rendering
    }
};

} // namespace UIEngine

#endif // ROOPM_OVERLAY_MANAGER_H
