#ifndef ROOPM_PROGRESS_BAR_H
#define ROOPM_PROGRESS_BAR_H

#include "core/base/UIElement.h"
#include <algorithm>

namespace UIEngine {

class ProgressBar : public UIElement {
public:
    float progress = 0.0f; // 0.0 to 1.0
    bool isIndeterminate = false;
    float animTime = 0.0f;

    ProgressBar(const std::string &id = "", float initialProgress = 0.0f)
        : UIElement(id), progress(initialProgress) {
        m_bounds.width = 200.0f;
        m_bounds.height = 6.0f;
    }

    void setProgress(float p) {
        progress = std::clamp(p, 0.0f, 1.0f);
        markRenderDirty();
    }

    void onUpdate(float dt) override {
        UIElement::onUpdate(dt);
        if (isIndeterminate) {
            animTime += dt;
            markRenderDirty();
        }
    }
};

} // namespace UIEngine

#endif // ROOPM_PROGRESS_BAR_H
