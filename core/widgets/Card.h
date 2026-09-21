#ifndef ROOPM_CARD_H
#define ROOPM_CARD_H

#include "core/base/UIElement.h"
#include "core/style/ThemeEngine.h"
#include "core/roopm.h"

namespace UIEngine {

class Card : public UIElement {
public:
    std::string title = "";
    std::string subtitle = "";
    float cornerRadius = 24.0f;

    Card(const std::string &id = "", const std::string &t = "", const std::string &st = "")
        : UIElement(id), title(t), subtitle(st) {
        m_bounds.width = 300.0f;
        m_bounds.height = 370.0f;
    }

    void onRender(void *ctx) override {
        if (!m_isVisible) return;
        bool isDark = ThemeEngine::instance().isDark;
        uint32_t textPri = isDark ? ROOPM_RGB(248, 250, 252) : ROOPM_RGB(15, 23, 42);
        uint32_t textSec = isDark ? ROOPM_RGBA(255, 255, 255, 170) : ROOPM_RGBA(30, 41, 59, 180);

        roopm_draw_vision_glass(m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height, cornerRadius);
        if (!title.empty()) roopm_draw_text(m_bounds.x + 20.0f, m_bounds.y + 18.0f, title.c_str(), 15.0f, textPri);
        if (!subtitle.empty()) roopm_draw_text(m_bounds.x + 20.0f, m_bounds.y + 36.0f, subtitle.c_str(), 11.0f, textSec);
        
        for (auto &child : m_children) {
            if (child->isVisible()) child->onRender(ctx);
        }
    }
};

} // namespace UIEngine

#endif // ROOPM_CARD_H
