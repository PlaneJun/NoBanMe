#include "BreakRecordWidget.h"
#include <vector>
#include <string>
#include <imgui.h>
#include "../render/render.h"
#include "../../common/utils/utils.h"

void BreakRecordWidget::OnPaint()
{
    ImGui::SeparatorText(u8"������¼");
    static const std::vector<std::string> headers = { u8"����" ,u8"��ַ" ,u8"ָ��" };
    std::vector<std::vector<std::string>> text{};
    for (auto i : DataSource_)
    {
        std::vector<std::string> tmp{};
        tmp.push_back(std::to_string(i.second.count));
        tmp.push_back(i.second.text);
        tmp.push_back(i.second.dbginfo.disassembly);
        text.push_back(tmp);
    }
    render::get_instasnce()->AddListBox("##bp_access", selected_, 1, headers, text);
    if (selected_ != -1)
    {
        int i = 0;
        for (auto ii : DataSource_)
        {
            if (i == selected_)
            {
                if (ChooseRecord_.text != ii.second.text)
                {
                    ChooseRecord_ = ii.second; //����Ҫ��ת�ĵ�ַ
                }
                break;
            }
            i++;
        }

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
            ImGui::OpenPopup("access_option");

        if (ImGui::BeginPopup("access_option"))
        {
            if (ImGui::BeginMenu(u8"����"))
            {
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"����",u8"��ַ",u8"ָ��" }))
                {
                    case 0:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), std::to_string(ChooseRecord_.count).c_str());
                        break;
                    }
                    case 1:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), ChooseRecord_.text.c_str());
                        break;
                    }
                    case 2:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), ChooseRecord_.dbginfo.disassembly);
                        break;
                    }
                }
                ImGui::Separator();
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"����",u8"������" }))
                {
                    case 0:
                    {
                        char buff[8192]{};
                        sprintf_s(buff, "%d | %s | %s", ChooseRecord_.count, ChooseRecord_.text.c_str(), ChooseRecord_.dbginfo.disassembly);
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), buff);
                        break;
                    }
                    case 1:
                    {
                        std::string ret{};
                        for (auto i : DataSource_)
                        {
                            char buff[8192]{};
                            sprintf_s(buff, "%d | %s | %s", i.second.count, i.second.text.c_str(), i.second.dbginfo.disassembly);
                            ret += buff;
                        }
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), ret.c_str());
                        break;
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
    }
}

Debugger::DbgCaptureInfo BreakRecordWidget::GetCurtSelect()
{
    return ChooseRecord_;
}

void BreakRecordWidget::SetDataSource(std::map<uint64_t, Debugger::DbgCaptureInfo> data)
{
    DataSource_ = data;
}
