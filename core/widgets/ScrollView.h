#ifndef ROOPM_SCROLL_VIEW_H
#define ROOPM_SCROLL_VIEW_H

#include "core/base/UIElement.h"
#include "core/style/ThemeEngine.h"
#include "core/roopm.h"
#include <cmath>
#include <algorithm>

namespace UIEngine {

class ScrollView : public UIElement {
public:
    float scrollY = 0.0f;
    float scrollVelocity = 0.0f;
    float contentHeight = 600.0f;
    
    bool isDragging = false;
    float dragStartPointerY = 0.0f;
    float dragStartScrollY = 0.0f;
    float lastPointerY = 0.0f;
    Vec2 pointerDownPos{0.0f, 0.0f};
    bool isGestureDragging = false;

    ScrollView(const std::string &id = "") : UIElement(id) {}

    float getMaxScroll() const {
        return std::max(0.0f, contentHeight - m_bounds.height);
    }

    void onUpdate(float dt) override {
        UIElement::onUpdate(dt);
        float maxScroll = getMaxScroll();

        if (!isDragging) {
            scrollY += scrollVelocity * dt;
            scrollVelocity *= expf(-6.0f * dt);

            if (scrollY > 0.0f) {
                scrollY += (0.0f - scrollY) * (1.0f - expf(-18.0f * dt));
                scrollVelocity *= 0.5f;
            } else if (scrollY < -maxScroll) {
                scrollY += (-maxScroll - scrollY) * (1.0f - expf(-18.0f * dt));
                scrollVelocity *= 0.5f;
            }
        }
    }

    void onRender(void *ctx) override {
        if (!m_isVisible) return;
        roopm_push_clip(m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height);

        for (auto &child : m_children) {
            if (child->isVisible()) child->onRender(ctx);
        }

        float maxScroll = getMaxScroll();
        if (maxScroll > 0.0f) {
            bool isDark = ThemeEngine::instance().isDark;
            float scrollPct = std::clamp(-scrollY / maxScroll, 0.0f, 1.0f);
            float pillTrackH = m_bounds.height - 24.0f;
            float pillH = std::max(30.0f, pillTrackH * (m_bounds.height / contentHeight));
            float pillY = m_bounds.y + 12.0f + scrollPct * (pillTrackH - pillH);
            roopm_draw_rect_rounded(m_bounds.x + m_bounds.width - 7.0f, pillY, 4.0f, pillH, 2.0f, ROOPM_RGBA(255, 255, 255, isDark ? 80 : 120));
        }

        roopm_pop_clip();
    }

    bool onEvent(UIEvent &event) override {
        if (!m_isEnabled || !m_isVisible) return false;

        UIEvent transformedEv = event;
        transformedEv.y = event.y - scrollY;

        if (event.type == EventType::PointerDown && m_bounds.contains(event.x, event.y)) {
            pointerDownPos = {event.x, event.y};
            isGestureDragging = false;
            lastPointerY = event.y;
            dragStartPointerY = event.y;
            dragStartScrollY = scrollY;
            scrollVelocity = 0.0f;
            isDragging = true;
        }

        if (event.type == EventType::PointerMove && isDragging) {
            float dist = std::fabs(event.y - pointerDownPos.y);
            if (dist > 8.0f) isGestureDragging = true;

            if (isGestureDragging) {
                float dy = event.y - lastPointerY;
                scrollVelocity = dy / 0.016f;
                lastPointerY = event.y;
                scrollY = dragStartScrollY + (event.y - dragStartPointerY);
            }
        }

        if (event.type == EventType::PointerUp && isDragging) {
            isDragging = false;
        }

        if (event.type == EventType::Scroll && m_bounds.contains(event.x, event.y)) {
            scrollVelocity += event.deltaY * 350.0f;
            event.handled = true;
            return true;
        }

        if (!isGestureDragging) {
            return UIElement::onEvent(transformedEv);
        }
        return true;
    }
};

} // namespace UIEngine

#endif // ROOPM_SCROLL_VIEW_H
