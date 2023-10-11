#pragma once
#include "../module/process.h"
#include "../widget/BreakRecordWidget.h"
#include "../widget/ContextWidget.h"
#include "../widget/DisassemblyWidget.h"
#include "../widget/StackWidget.h"
class VehDebuggerWidget
{
public:
    void OnPaint();

    void SetDataSource(ProcessItem d);

private:
    ProcessItem DataSource_;
    DisassemblyWidget disassemblerWidget_;
    BreakRecordWidget breakrecordWidget_;
    ContextWidget contextWidget_;
    StackWidget stackWidget_;

};