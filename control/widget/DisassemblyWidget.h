#pragma once

class DisassemblerWidget
{
public:
	void OnPaint()
	{
        char title[256]{};
        sprintf_s(title, u8"反汇编\t 模块:%s", curtModule.c_str());
		ImGui::SeparatorText(title);
        if (ImGui::BeginTable("#disassembly", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
        {
            ImGui::TableSetupColumn(u8"地址", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(u8"字节码", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn(u8"指令", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();
            //dissambly_base_ = (uint64_t)Mem::GetProcessModuleHandle(config::global::injectProcess.GetPid(), utils::conver::string_to_wstirng(config::global::injectProcess.GetName()).c_str());
            if (dissambly_base_ > 0)
            {
                if (dissambly_jmp_ > 0)
                {
                    curt_page_index_ = (dissambly_jmp_ - dissambly_base_) / Debugger::DISAM_BLOCK;
                    dissambly_block_.clear();
                }

            getdisam:
                uint64_t disam_start = dissambly_base_ + curt_page_index_ * Debugger::DISAM_BLOCK;
                if (dissambly_block_.size() <= 0)
                {
                    std::vector<uint8_t> region{};
                    region.resize(Debugger::DISAM_BLOCK);
                    //read mem
                    Mem::ReadMemory(DataSource_.GetPid(), disam_start, &region[0], Debugger::DISAM_BLOCK);
                    dissambly_block_ = Debugger::Disassembly(!DataSource_.IsWow64(), disam_start, region.data(), Debugger::DISAM_BLOCK);
                }
                else
                {
                    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() && ImGui::GetIO().MouseWheel < 0) //down
                    {
                        if (wheel_count_ > 3)
                        {
                            curt_page_index_++;
                            dissambly_block_.clear();
                            wheel_count_ = 0;
                            goto getdisam;
                        }
                        else
                        {
                            wheel_count_++;
                        }

                    }
                    else if (ImGui::GetScrollY() <= 0 && curt_page_index_ > 0 && ImGui::GetIO().MouseWheel > 0) //up
                    {
                        if (wheel_count_ > 3)
                        {
                            curt_page_index_--;
                            dissambly_block_.clear();
                            wheel_count_ = 0;
                            goto getdisam;
                        }
                        else
                        {
                            wheel_count_++;
                        }
                    }
                }

                for (int i = 0; i < dissambly_block_.size(); i++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    if (ImGui::Selectable(dissambly_block_[i][0].c_str(), selected_ == i, ImGuiSelectableFlags_SpanAllColumns))
                        selected_ = i;
                    for (int j = 1; j < dissambly_block_[i].size(); j++)
                    {
                        ImGui::TableSetColumnIndex(j);
                        ImGui::Text(dissambly_block_[i][j].c_str()); ImGui::NextColumn();
                    }

                    //如果到了指定行就停止滚动
                    if (dissambly_jmp_ > 0 && utils::conver::IntegerTohex(dissambly_jmp_) == dissambly_block_[i][0])
                    {
                        selected_ = i; //只在跳转的时候选择一次
                        dissambly_jmp_ = 0;
                    }
                    if (dissambly_jmp_ > 0)
                    {
                        ImGui::SetScrollHereY(0);
                    }

                }
                dissambly_jmp_ = 0;//无论有没有找到都设置为0
            }

            if (selected_ != -1)
            {
                if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
                    ImGui::OpenPopup("disassembly_option");

                bool openJmpToInputAddress = false;
                if (ImGui::BeginPopup("disassembly_option"))
                {
                    switch (int s = render::get_instasnce()->DrawItemBlock({ u8"跳转到" }))
                    {
                        case 0:
                        {
                            openJmpToInputAddress = true;
                            break;
                        }
                    }
                    if (ImGui::BeginMenu(u8"复制"))
                    {
                        switch (int s = render::get_instasnce()->DrawItemBlock({ u8"地址",u8"字节码",u8"指令" }))
                        {
                            case 0:
                            {
                                utils::normal::CopyStringToClipboard(dissambly_block_[selected_][0].c_str());
                                break;
                            }
                            case 1:
                            {
                                utils::normal::CopyStringToClipboard(dissambly_block_[selected_][1].c_str());
                                break;
                            }
                            case 2:
                            {
                                utils::normal::CopyStringToClipboard(dissambly_block_[selected_][2].c_str());
                                break;
                            }
                        }
                        ImGui::Separator();
                        switch (int s = render::get_instasnce()->DrawItemBlock({ u8"整行" }))
                        {
                            case 0:
                            {
                                char buff[8192]{};
                                sprintf_s(buff, "%d | %s | %s", dissambly_block_[selected_][0].c_str(), dissambly_block_[selected_][1].c_str(), dissambly_block_[selected_][2].c_str());
                                utils::normal::CopyStringToClipboard(buff);
                                break;
                            }
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }
                if (openJmpToInputAddress)
                {
                    openJmpToInputAddress = false;
                    ImGui::OpenPopup("JmpToInputAddress");
                    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                }

                if (ImGui::BeginPopupModal("JmpToInputAddress", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text(u8"输入要跳转的地址");
                    ImGui::Separator();
                    static char buf[100]{};
                    ImGui::InputText("##JmpToInputAddress", buf, 100, ImGuiInputTextFlags_CharsHexadecimal);
                    if (ImGui::Button(u8"确认", ImVec2(120, 0)))
                    {
                        dissambly_jmp_ = utils::conver::hexToInteger(buf);
                        SetData(DataSource_, dissambly_jmp_);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button(u8"取消", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }
            }
            ImGui::EndTable();
        }
	}

    auto GetSelected() 
    {
        return selected_;
    }

    auto SetData(ProcessItem proc,uint64_t jmpto)
    {
        DataSource_ = proc;
        if (DataSource_.GetPid() > 0)
        {
            //查询地址对应的模块基地址
            MEMORY_BASIC_INFORMATION meminfo{};
            Mem::QueryMem(DataSource_.GetPid(), (PVOID)jmpto, &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
            std::vector<uint64_t> bases{};
            bases.push_back((uint64_t)meminfo.BaseAddress);
            bases.push_back((uint64_t)meminfo.AllocationBase);
            for (auto& i : bases)
            {
                if (i <= 0)
                    continue;
                curtModule = Mem::GetModuleFullName(DataSource_.GetPid(), i);
                if (!curtModule.empty())
                {
                    curtModule = curtModule.substr(curtModule.rfind("\\")+1);
                    if (dissambly_base_ != i)
                        dissambly_base_ = i;
                    break;//找到就返回
                }
            }
            
            //碰到shellcode类似
            if (curtModule.empty())
                dissambly_base_ = (uint64_t)meminfo.BaseAddress > 0 ? (uint64_t)meminfo.BaseAddress : (uint64_t)meminfo.AllocationBase;

            dissambly_jmp_ = jmpto;
        }
    }

private:
    ProcessItem DataSource_;
    int selected_ = -1;
    std::vector<std::vector<std::string>> dissambly_block_{};
    uint64_t dissambly_base_;
    uint64_t dissambly_jmp_;
    uint32_t curt_page_index_;
    uint32_t max_page_index_;
    uint8_t wheel_count_; //轮动记录5次翻页
    std::string curtModule;
};