#include "SyscallMonitorWidget.h"
#include <imgui.h>

void SysCallMonitorWidget::OnPaint()
{
    ImGui::SeparatorText(u8"选项");
    ImGui::Checkbox(u8"启用",&active_); ImGui::SameLine();
    ImGui::Checkbox(u8"保存日志", &save_); ImGui::SameLine();
    ImGui::Button(u8"清除");
    ImGui::SeparatorText(u8"输出");
    if (ImGui::TreeNode("root"))
    {
        for (auto n : monitor_)
        {
            if (ImGui::TreeNode(n.first.c_str()))
            {
                for (auto m : n.second)
                {
                    ImGui::Text("%d : %s", m.second, m.first.c_str());
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

bool SysCallMonitorWidget::GetActive()
{
    return active_;
}

void  SysCallMonitorWidget::SetActive(bool b)
{
    active_ = b;
}

bool SysCallMonitorWidget::GetSave()
{
    return save_;
}

bool SysCallMonitorWidget::IsExsist(std::string key)
{
    return monitor_.count(key) > 0;
}

void SysCallMonitorWidget::SetValue(std::string k1,std::string k2,uint64_t v,bool inc)
{
    //inc = ture，增加值;=false，等于值
    if (inc)
        monitor_[k1][k2] += v;
    else
        monitor_[k1][k2] = v;
}
