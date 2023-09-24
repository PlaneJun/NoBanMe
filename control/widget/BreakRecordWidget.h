#pragma once
class BreakRecordWidget
{
public:
	void OnPaint()
	{
        ImGui::SeparatorText(u8"触发记录");
        static const std::vector<std::string> headers = { u8"计数" ,u8"地址" ,u8"指令" };
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
                        ChooseRecord_ = ii.second; //设置要跳转的地址
                    }
                    break;
                }
                i++;
            }

            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
                ImGui::OpenPopup("access_option");

            if (ImGui::BeginPopup("access_option"))
            {
                if (ImGui::BeginMenu(u8"复制"))
                {
                    switch (int s = render::get_instasnce()->DrawItemBlock({ u8"计数",u8"地址",u8"指令" }))
                    {
                        case 0:
                        {
                            utils::normal::CopyStringToClipboard(std::to_string(ChooseRecord_.count).c_str());
                            break;
                        }
                        case 1:
                        {
                            utils::normal::CopyStringToClipboard(ChooseRecord_.text.c_str());
                            break;
                        }
                        case 2:
                        {
                            utils::normal::CopyStringToClipboard(ChooseRecord_.dbginfo.disassembly);
                            break;
                        }
                    }
                    ImGui::Separator();
                    switch (int s = render::get_instasnce()->DrawItemBlock({ u8"整行",u8"整个表" }))
                    {
                        case 0:
                        {
                            char buff[8192]{};
                            sprintf_s(buff, "%d | %s | %s", ChooseRecord_.count, ChooseRecord_.text.c_str(), ChooseRecord_.dbginfo.disassembly);
                            utils::normal::CopyStringToClipboard(buff);
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
                            utils::normal::CopyStringToClipboard(ret.c_str());
                            break;
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
        }
	}

    auto GetCurtSelect()
    {
        return ChooseRecord_;
    }

    auto SetDataSource(std::map<uint64_t, Debugger::DbgCaptureInfo> data)
    {
        DataSource_ = data;
    }
private:
    int selected_ = -1;
    std::map<uint64_t, Debugger::DbgCaptureInfo> DataSource_;
    Debugger::DbgCaptureInfo ChooseRecord_{};
};