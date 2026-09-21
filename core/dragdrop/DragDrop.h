#ifndef ROOPM_DRAG_DROP_H
#define ROOPM_DRAG_DROP_H

#include <string>
#include <vector>

namespace UIEngine {

enum class DragPayloadType {
    PlainText,
    Unicode,
    CustomBinary,
    FilePaths,
    ImageData
};

struct DragPayload {
    DragPayloadType type = DragPayloadType::PlainText;
    std::string textData;
    std::vector<std::string> filePaths;
    std::vector<uint8_t> binaryData;
    void *customPointer = nullptr;
};

struct DragVisual {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float width = 100.0f;
    float height = 40.0f;
    float opacity = 0.85f;
    bool isVisible = false;
};

class DropTarget {
public:
    virtual void onDragEnter(const DragPayload &payload, float x, float y) {}
    virtual void onDragOver(const DragPayload &payload, float x, float y) {}
    virtual void onDragLeave() {}
    virtual bool onDrop(const DragPayload &payload, float x, float y) { return false; }
    virtual void onDragCancel() {}
    virtual ~DropTarget() = default;
};

} // namespace UIEngine

#endif // ROOPM_DRAG_DROP_H
