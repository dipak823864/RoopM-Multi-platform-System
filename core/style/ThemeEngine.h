#ifndef ROOPM_THEME_ENGINE_H
#define ROOPM_THEME_ENGINE_H

#include <stdint.h>

namespace UIEngine {

struct DesignTokens {
    uint32_t bgPrimary;
    uint32_t bgSecondary;
    uint32_t bgGlass;
    uint32_t textPrimary;
    uint32_t textSecondary;
    uint32_t accent;
    uint32_t accentHover;
    uint32_t border;
    uint32_t borderGlass;

    float radiusSmall = 6.0f;
    float radiusMedium = 14.0f;
    float radiusLarge = 26.0f;
    float radiusFull = 9999.0f;

    int glassBlurRadius = 12;
    float shadowBlur = 16.0f;
    uint8_t shadowAlpha = 45;
};

class ThemeEngine {
public:
    static ThemeEngine &instance() {
        static ThemeEngine s_inst;
        return s_inst;
    }

    bool isDark = true;
    DesignTokens tokens;

    ThemeEngine() { setDarkTheme(); }

    void setDarkTheme() {
        isDark = true;
        tokens.bgPrimary = 0xFF0B1120;
        tokens.bgSecondary = 0xFF1E293B;
        tokens.bgGlass = 0x22FFFFFF;
        tokens.textPrimary = 0xFFF8FAFC;
        tokens.textSecondary = 0xFF94A3B8;
        tokens.accent = 0xFF3B82F6;
        tokens.accentHover = 0xFF60A5FA;
        tokens.border = 0xFF334155;
        tokens.borderGlass = 0x66FFFFFF;
    }

    void setLightTheme() {
        isDark = false;
        tokens.bgPrimary = 0xFFF8FAFC;
        tokens.bgSecondary = 0xFFFFFFFF;
        tokens.bgGlass = 0x44FFFFFF;
        tokens.textPrimary = 0xFF0F172A;
        tokens.textSecondary = 0xFF64748B;
        tokens.accent = 0xFF2563EB;
        tokens.accentHover = 0xFF1D4ED8;
        tokens.border = 0xFFE2E8F0;
        tokens.borderGlass = 0xAAFFFFFF;
    }
};

} // namespace UIEngine

#endif // ROOPM_THEME_ENGINE_H
