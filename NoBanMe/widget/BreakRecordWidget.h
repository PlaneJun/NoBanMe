#pragma once
#include <map>
#include <stdint.h>
#include "../module/debugger.h"

class BreakRecordWidget
{
public:
    void OnPaint();

    Debugger::DbgCaptureInfo GetCurtSelect();

    void SetDataSource(std::map<uint64_t, Debugger::DbgCaptureInfo> data);

private:
    int selected_ = -1;
    std::map<uint64_t, Debugger::DbgCaptureInfo> DataSource_;
    Debugger::DbgCaptureInfo ChooseRecord_{};
};