#ifndef ROOPM_WIDGET_STATE_H
#define ROOPM_WIDGET_STATE_H

#include <stdint.h>

namespace UIEngine {

enum class WidgetState : uint32_t {
    Normal       = 1 << 0,  // Idle default
    Hovered      = 1 << 1,  // Mouse hover or stylus proximity
    Pressed      = 1 << 2,  // Active mouse down or touch contact
    Focused      = 1 << 3,  // Keyboard focus active
    FocusVisible = 1 << 4,  // Keyboard focus indicator visible
    Disabled     = 1 << 5,  // Non-interactive, events swallowed
    Checked      = 1 << 6,  // For toggles & checkboxes
    Unchecked    = 1 << 7,
    Indeterminate= 1 << 8,
    Dragging     = 1 << 9,  // Actively being dragged
    DragOver     = 1 << 10, // Active payload hovering over node
    Selected     = 1 << 11, // For list items, tabs, rows
    Loading      = 1 << 12, // Busy / Spinner active
    Invalid      = 1 << 13  // Validation error state
};

inline WidgetState operator|(WidgetState a, WidgetState b) {
    return static_cast<WidgetState>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline bool has_state(WidgetState mask, WidgetState query) {
    return (static_cast<uint32_t>(mask) & static_cast<uint32_t>(query)) != 0;
}

} // namespace UIEngine

#endif // ROOPM_WIDGET_STATE_H
