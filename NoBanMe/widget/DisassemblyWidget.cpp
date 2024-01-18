#include "DisassemblyWidget.h"
#include <imgui.h>
#include "../render/render.h"
#include "../utils/utils.h"
#include "../mem/memstub.h"
#include "../module/debugger.h"

int DisassemblyWidget::GetSelected()
{
    return selected_;
}

void DisassemblyWidget::SetData(ProcessItem proc, uint64_t jmpto)
{
    DataSource_ = proc;
    if (DataSource_.GetPid() > 0)
    {
        //��ѯ��ַ��Ӧ��ģ�����ַ
        MEMORY_BASIC_INFORMATION meminfo{};
        MemStub::QueryMem(DataSource_.GetPid(),(uintptr_t)(jmpto), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
        std::vector<uint64_t> bases{};
        bases.push_back((uint64_t)meminfo.BaseAddress);
        bases.push_back((uint64_t)meminfo.AllocationBase);
        for (auto& i : bases)
        {
            if (i <= 0)
            {
                continue;
            }
            curtModule = MemStub::GetModuleFullName(DataSource_.GetPid(), i);
            if (!curtModule.empty())
            {
                curtModule = curtModule.substr(curtModule.rfind("\\") + 1);
                if (dissambly_base_ != i)
                    dissambly_base_ = i;
                break;//�ҵ��ͷ���
            }
        }

        //����shellcode����
        if (curtModule.empty())
            dissambly_base_ = (uint64_t)meminfo.BaseAddress > 0 ? (uint64_t)meminfo.BaseAddress : (uint64_t)meminfo.AllocationBase;

        dissambly_jmp_ = jmpto;
    }
}


void DisassemblyWidget::OnPaint()
{
    char title[256]{};
    sprintf_s(title, u8"�����\t ģ��:%s", curtModule.c_str());
	ImGui::SeparatorText(title);
    if (ImGui::BeginTable("#disassembly", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
    {
        ImGui::TableSetupColumn(u8"��ַ", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(u8"�ֽ���", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(u8"ָ��", ImGuiTableColumnFlags_WidthFixed);
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
                MemStub::ReadMemory(DataSource_.GetPid(), disam_start,reinterpret_cast<uintptr_t>(&region[0]), Debugger::DISAM_BLOCK);
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

                //�������ָ���о�ֹͣ����
                if (dissambly_jmp_ > 0 && utils::conver::IntegerTohex(dissambly_jmp_) == dissambly_block_[i][0])
                {
                    selected_ = i; //ֻ����ת��ʱ��ѡ��һ��
                    dissambly_jmp_ = 0;
                }
                if (dissambly_jmp_ > 0)
                {
                    ImGui::SetScrollHereY(0);
                }

            }
            dissambly_jmp_ = 0;//������û���ҵ�������Ϊ0
        }

        if (selected_ != -1)
        {
            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
                ImGui::OpenPopup("disassembly_option");

            bool openJmpToInputAddress = false;
            if (ImGui::BeginPopup("disassembly_option"))
            {
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"��ת��" }))
                {
                    case 0:
                    {
                        openJmpToInputAddress = true;
                        break;
                    }
                }
                if (ImGui::BeginMenu(u8"����"))
                {
                    switch (int s = render::get_instasnce()->DrawItemBlock({ u8"��ַ",u8"�ֽ���",u8"ָ��" }))
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
                    switch (int s = render::get_instasnce()->DrawItemBlock({ u8"����" }))
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
                ImGui::Text(u8"����Ҫ��ת�ĵ�ַ");
                ImGui::Separator();
                static char buf[100]{};
                ImGui::InputText("##JmpToInputAddress", buf, 100, ImGuiInputTextFlags_CharsHexadecimal);
                if (ImGui::Button(u8"ȷ��", ImVec2(120, 0)))
                {
                    dissambly_jmp_ = utils::conver::hexToInteger(buf);
                    SetData(DataSource_, dissambly_jmp_);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button(u8"ȡ��", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }
        }
        ImGui::EndTable();
    }
}
