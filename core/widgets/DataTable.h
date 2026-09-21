#ifndef ROOPM_DATA_TABLE_H
#define ROOPM_DATA_TABLE_H

#include "core/base/UIElement.h"
#include <vector>
#include <string>
#include <algorithm>

namespace UIEngine {

struct DataColumn {
    std::string header = "";
    float width = 100.0f;
    bool isAscending = true;

    DataColumn() = default;
    DataColumn(const std::string &h, float w = 100.0f, bool asc = true)
        : header(h), width(w), isAscending(asc) {}
};

class DataTable : public UIElement {
public:
    std::vector<DataColumn> columns;
    std::vector<std::vector<std::string>> rows;
    int selectedRow = -1;
    int sortedColumnIndex = -1;
    bool isAscending = true;
    
    float headerHeight = 34.0f;
    float rowHeight = 30.0f;

    DataTable(const std::string &id = "") : UIElement(id) {
        m_bounds.width = 400.0f;
        m_bounds.height = 250.0f;
    }

    void sortByColumn(int colIdx) {
        if (colIdx < 0 || colIdx >= (int)columns.size()) return;

        if (sortedColumnIndex == colIdx) {
            isAscending = !isAscending;
        } else {
            sortedColumnIndex = colIdx;
            isAscending = true;
        }

        bool asc = isAscending;
        std::sort(rows.begin(), rows.end(), [colIdx, asc](const std::vector<std::string> &a, const std::vector<std::string> &b) {
            if (colIdx < (int)a.size() && colIdx < (int)b.size()) {
                return asc ? (a[colIdx] < b[colIdx]) : (a[colIdx] > b[colIdx]);
            }
            return false;
        });
        markRenderDirty();
    }
};

} // namespace UIEngine

#endif // ROOPM_DATA_TABLE_H
