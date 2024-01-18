#pragma once
#include <vector>
#include <stdint.h>
#include "../module/thread.h"

class ThreadWidget
{
public:
    void OnPaint();

    void SetDataSource(uint32_t pid);

    void Load();

private:
    std::vector<ThreadItem> DataSource_{};
    int selected_ = -1;
    bool show_;

};