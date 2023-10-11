#pragma once
#include <vector>
#include <stdint.h>
#include "../module/memory.h"

class MemoryWidget
{
public:
    void OnPaint();

    void SetDataSource(uint32_t pid);

    void Load();
private:
    std::vector<MemoryItem> DataSource_{};
    int selected_ = -1;
    bool show_;
};