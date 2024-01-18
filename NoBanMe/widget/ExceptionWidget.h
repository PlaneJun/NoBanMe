#pragma once
#include <vector>
#include <stdint.h>
#include "../module/vehhandler.h"

class ExceptionWidget
{
public:
    void OnPaint();

    void SetDataSource(uint32_t pid);

    void Load();

private:
    std::vector<VehHandlerItem> DataSource_{};
    int selected_ = -1;
    bool show_;
};
