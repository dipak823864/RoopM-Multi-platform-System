#ifndef ROOPM_UI_ELEMENT_H
#define ROOPM_UI_ELEMENT_H

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <cmath>
#include <algorithm>
#include <stdint.h>

#include "core/widgets/WidgetState.h"
#include "core/base/DisplayMetrics.h"

namespace UIEngine {

struct Vec2 {
    float x = 0.0f, y = 0.0f;
    Vec2() = default;
    Vec2(float _x, float _y) : x(_x), y(_y) {}
    Vec2 operator+(const Vec2 &o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2 &o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    float length() const { return std::sqrt(x * x + y * y); }
};

struct Rect {
    float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f;
    Rect() = default;
    Rect(float _x, float _y, float _w, float _h) : x(_x), y(_y), width(_w), height(_h) {}
    bool contains(float px, float py) const {
        return px >= x && px <= (x + width) && py >= y && py <= (y + height);
    }
    bool contains(const Vec2 &pt) const { return contains(pt.x, pt.y); }
};

struct BoxConstraints {
    float minWidth = 0.0f;
    float maxWidth = 100000.0f;
    float minHeight = 0.0f;
    float maxHeight = 100000.0f;

    static BoxConstraints loose(float w, float h) {
        return {0.0f, w, 0.0f, h};
    }
    static BoxConstraints tight(float w, float h) {
        return {w, w, h, h};
    }
    Vec2 constrain(float w, float h) const {
        float targetW = (w > 0.0f) ? w : minWidth;
        float targetH = (h > 0.0f) ? h : minHeight;
        return {
            std::clamp(targetW, minWidth, maxWidth),
            std::clamp(targetH, minHeight, maxHeight)
        };
    }
};

enum class EventType {
    PointerDown,
    PointerMove,
    PointerUp,
    PointerCancel,
    PointerEnter,
    PointerLeave,
    Scroll,
    KeyDown,
    KeyUp
};

struct UIEvent {
    EventType type = EventType::PointerMove;
    float x = 0.0f, y = 0.0f;
    float deltaX = 0.0f, deltaY = 0.0f;
    uint32_t keyCode = 0;
    bool handled = false;
};

class UIElement : public std::enable_shared_from_this<UIElement> {
protected:
    std::string m_id;
    Rect m_bounds{0, 0, 0, 0};
    WidgetState m_state = WidgetState::Normal;
    bool m_isLayoutDirty = true;
    bool m_isRenderDirty = true;
    bool m_isVisible = true;
    bool m_isEnabled = true;
    
    std::weak_ptr<UIElement> m_parent;
    std::vector<std::shared_ptr<UIElement>> m_children;

public:
    UIElement(const std::string &id = "") : m_id(id) {}
    virtual ~UIElement() = default;

    // Visibility & Enabled Getters
    bool isVisible() const { return m_isVisible; }
    void setVisible(bool v) { m_isVisible = v; markRenderDirty(); }
    bool isEnabled() const { return m_isEnabled; }
    void setEnabled(bool e) { m_isEnabled = e; markRenderDirty(); }

    // Lifecycle Hooks
    virtual void onInit() {}
    virtual void onUpdate(float dt) {
        for (auto &child : m_children) {
            if (child->isVisible()) child->onUpdate(dt);
        }
    }
    virtual Vec2 onLayout(const BoxConstraints &constraints) {
        Vec2 sz = constraints.constrain(m_bounds.width, m_bounds.height);
        m_bounds.width = sz.x;
        m_bounds.height = sz.y;
        m_isLayoutDirty = false;
        return {m_bounds.width, m_bounds.height};
    }
    virtual void onRender(void *renderContext) {
        m_isRenderDirty = false;
        for (auto &child : m_children) {
            if (child->isVisible()) child->onRender(renderContext);
        }
    }
    virtual bool onEvent(UIEvent &event) {
        if (!m_isEnabled || !m_isVisible) return false;
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->m_bounds.contains(event.x, event.y)) {
                if ((*it)->onEvent(event)) return true;
            }
        }
        return false;
    }
    virtual void onDestroy() {
        for (auto &child : m_children) child->onDestroy();
        m_children.clear();
    }

    // Hierarchy Management
    void addChild(std::shared_ptr<UIElement> child) {
        child->m_parent = weak_from_this();
        m_children.push_back(child);
        markLayoutDirty();
    }
    void removeChild(std::shared_ptr<UIElement> child) {
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end()) {
            (*it)->onDestroy();
            m_children.erase(it);
            markLayoutDirty();
        }
    }
    const std::vector<std::shared_ptr<UIElement>> &getChildren() const { return m_children; }
    
    // Bounds & Geometry
    void setPosition(float x, float y) { m_bounds.x = x; m_bounds.y = y; markRenderDirty(); }
    void setSize(float w, float h) { m_bounds.width = w; m_bounds.height = h; markLayoutDirty(); }
    const Rect &getBounds() const { return m_bounds; }
    
    // State
    void setState(WidgetState state) { m_state = state; markRenderDirty(); }
    WidgetState getState() const { return m_state; }
    bool hasState(WidgetState query) const { return UIEngine::has_state(m_state, query); }

    // Dirty propagation
    void markLayoutDirty() {
        m_isLayoutDirty = true;
        m_isRenderDirty = true;
        if (auto p = m_parent.lock()) p->markLayoutDirty();
    }
    void markRenderDirty() {
        m_isRenderDirty = true;
        if (auto p = m_parent.lock()) p->markRenderDirty();
    }

    const std::string &getId() const { return m_id; }
};

} // namespace UIEngine

#endif // ROOPM_UI_ELEMENT_H
