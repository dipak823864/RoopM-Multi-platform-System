#ifndef ROOPM_DRAG_CONTROLLER_H
#define ROOPM_DRAG_CONTROLLER_H

#include "core/dragdrop/DragDrop.h"
#include "core/base/UIElement.h"
#include <memory>

namespace UIEngine {

class DragController {
public:
    static DragController &instance() {
        static DragController s_inst;
        return s_inst;
    }

    bool isDragging = false;
    DragPayload currentPayload;
    DragVisual currentVisual;
    Vec2 cursorPosition;
    std::shared_ptr<DropTarget> currentHoverTarget;

    void startDrag(const DragPayload &payload, const DragVisual &visual, const Vec2 &startPos) {
        isDragging = true;
        currentPayload = payload;
        currentVisual = visual;
        cursorPosition = startPos;
    }

    void updateDrag(const Vec2 &pos, std::shared_ptr<DropTarget> targetUnderCursor) {
        if (!isDragging) return;
        cursorPosition = pos;

        if (targetUnderCursor != currentHoverTarget) {
            if (currentHoverTarget) currentHoverTarget->onDragLeave();
            currentHoverTarget = targetUnderCursor;
            if (currentHoverTarget) currentHoverTarget->onDragEnter(currentPayload, pos.x, pos.y);
        } else if (currentHoverTarget) {
            currentHoverTarget->onDragOver(currentPayload, pos.x, pos.y);
        }
    }

    bool drop(const Vec2 &pos) {
        if (!isDragging) return false;
        bool handled = false;
        if (currentHoverTarget) {
            handled = currentHoverTarget->onDrop(currentPayload, pos.x, pos.y);
        }
        cancelDrag();
        return handled;
    }

    void cancelDrag() {
        if (isDragging && currentHoverTarget) {
            currentHoverTarget->onDragCancel();
        }
        isDragging = false;
        currentHoverTarget = nullptr;
    }
};

} // namespace UIEngine

#endif // ROOPM_DRAG_CONTROLLER_H
