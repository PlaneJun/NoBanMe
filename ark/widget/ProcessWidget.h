#pragma once
#include <vector>
#include "../module/process.h"
#include "../widget/ThreadWidget.h"
#include "../widget/ExceptionWidget.h"
#include "../widget/MemoryWidget.h"

class ProcessWidget
{
public:
    void OnPaint();

    ProcessItem GetCurtProcessItem();

    ProcessItem GetPluginProcessItem();

private:
    int selected_ = -1;
    ProcessItem pluginProcess_;
	std::vector<ProcessItem> DataSource_;
    ThreadWidget threadWidget;
    ExceptionWidget exceptionWidget;
    MemoryWidget memoryWidget;
};
