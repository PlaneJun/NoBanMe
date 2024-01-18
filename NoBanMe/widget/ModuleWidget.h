#pragma once
#include <vector>
#include <stdint.h>
#include "../module/module.h"

class ModuleWidget
{
public:
    void OnPaint();

    void SetDataSource(uint32_t pid);

private:
    uint32_t pid_=0;
    int selected_ = -1;
    std::vector<ModuleItem> DataSource_;
};
