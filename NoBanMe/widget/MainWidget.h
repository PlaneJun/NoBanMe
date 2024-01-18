#pragma once
#include <imgui.h>
#include "../widget/ProcessWidget.h"
#include "../widget/ModuleWidget.h"
#include "../widget/SyscallMonitorWidget.h"
#include "../widget/VehDebuggerWidget.h"
#include "../widget/WindowsWidget.h"

class MainWidget
{
public:
    void OnPaint();

    void OnUpdate();

    void OnIPC();
    
    void SetWindowSize(float w, float h);

private:

    ImVec2 size_;
    ProcessWidget processWidget_;
    ModuleWidget moduleWidget_;
    SysCallMonitorWidget syscallMonitorWidget_;
    VehDebuggerWidget vehDebuggerWidget_;
    WindowsWidget windowsWidget_;
};