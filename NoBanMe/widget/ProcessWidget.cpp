#include <filesystem>
#include "ProcessWidget.h"
#include <imgui.h>
#include "../../common/utils/utils.h"
#include "../render/render.h"
#include "../../common/mem/memstub.h"
#include "../global.h"

void ProcessWidget::OnPaint()
{
    //���û�л�ȡ�����ȡһ��
    if (DataSource_.size() == 0)
    {
        ProcessItem::EnumCurtAllProcess(DataSource_);
    }
    //����
    if (ImGui::BeginTable("#processlist", 10, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV))
    {
        ImGui::TableSetupColumn("##icon", ImGuiTableColumnFlags_NoSort, 0.0f);
        ImGui::TableSetupColumn(u8"����ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::PID);
        ImGui::TableSetupColumn(u8"������", ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::NAME);
        ImGui::TableSetupColumn(u8"������ID", ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::PPID);
        ImGui::TableSetupColumn(u8"����ʱ��", ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::STARTUPTIME);
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, 0.0f);
        ImGui::TableSetupColumn(u8"�ļ�����", ImGuiTableColumnFlags_WidthFixed, 0.0f);
        ImGui::TableSetupColumn(u8"�ļ��汾", ImGuiTableColumnFlags_WidthFixed, 0.0f);
        ImGui::TableSetupColumn(u8"����·��", ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::FULLPATH);
        ImGui::TableSetupColumn(u8"������", ImGuiTableColumnFlags_WidthFixed, 0.0f);
        ImGui::TableHeadersRow();
        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
        {
            if (sorts_specs->SpecsDirty)
            {
                ProcessItem::SetSortSpecs(sorts_specs); // Store in variable accessible by the sort function.
                if (DataSource_.size() > 1)
                    qsort(&DataSource_[0], (size_t)DataSource_.size(), sizeof(DataSource_[0]), ProcessItem::CompareWithSortSpecs);
                ProcessItem::SetSortSpecs(NULL);
                sorts_specs->SpecsDirty = false;
            }
        }
        ImGuiListClipper clipper;
        clipper.Begin(DataSource_.size());
        while (clipper.Step())
        {
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
            {
                ProcessItem* item = &DataSource_[row_n];
                ImGui::PushID(item->GetPid());
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (item->GetIcon())
                    ImGui::Image(item->GetIcon(), ImVec2(16, 16));
                ImGui::TableNextColumn();
                if (ImGui::Selectable(std::to_string(item->GetPid()).c_str(), selected_ == row_n, ImGuiSelectableFlags_SpanAllColumns))
                {
                    selected_ = row_n;
                }
                ImGui::TableNextColumn();
                ImGui::Text(utils::conver::string_To_UTF8(item->GetName().append(item->IsWow64() ? "*32" : "")).c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%d", item->GetPPid());
                ImGui::TableNextColumn();
                ImGui::Text(utils::conver::string_To_UTF8(item->GetStartUpTime()).c_str());
                ImGui::TableNextColumn();
                ImGui::Text(utils::conver::string_To_UTF8(item->GetDecription()).c_str());
                ImGui::TableNextColumn();
                ImGui::Text(utils::conver::string_To_UTF8(item->GetCompanyName()).c_str());
                ImGui::TableNextColumn();
                ImGui::Text(utils::conver::string_To_UTF8(item->GetFileVersion()).c_str());
                ImGui::TableNextColumn();
                ImGui::Text(utils::conver::string_To_UTF8(item->GetFullPath()).c_str());
                ImGui::TableNextColumn();
                ImGui::Text(utils::conver::string_To_UTF8(item->GetCmdLine()).c_str());
                ImGui::PopID();
            }
        }

        if (selected_ != -1 && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
            ImGui::OpenPopup("process_option");

        if (ImGui::BeginPopup("process_option"))
        {
            switch (int s = render::get_instasnce()->DrawItemBlock({ u8"ˢ�½���" }))
            {
                case 0:
                {
                    DataSource_.clear();
                    break;
                }
            }
            ImGui::Separator();
            switch (int s = render::get_instasnce()->DrawItemBlock({ u8"�ڴ��б�",u8"�߳��б�",u8"���̾��",u8"�쳣�ص�" }))
            {
                case 0:
                {
                    memoryWidget_.SetDataSource(DataSource_[selected_].GetPid());
                    memoryWidget_.Load();
                    break;
                }
                case 1:
                {
                    threadWidget_.SetDataSource(DataSource_[selected_].GetPid());
                    threadWidget_.Load();
                    break;
                }
                case 2:
                {
                    
                    break;
                }
                case 3:
                {
                    exceptionWidget_.SetDataSource(DataSource_[selected_].GetPid());
                    exceptionWidget_.Load();
                    break;
                }
            }
            ImGui::Separator();
            switch (int s = render::get_instasnce()->DrawItemBlock({ u8"��������",u8"�����߳�",u8"�ָ��߳�",u8"���ؽ���",u8"��������" }))
            {
                case 0:
                {
                    HANDLE handLe = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DataSource_[selected_].GetPid());
                    if (handLe)
                    {
                        reinterpret_cast<DWORD(WINAPI*)(HANDLE hProcess, DWORD DwExitCode)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtTerminateProcess"))(handLe,0);
                        CloseHandle(handLe);
                    }
                    break;
                }
                case 1:
                {
                       
                    HANDLE handLe = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DataSource_[selected_].GetPid());
                    if (handLe)
                    {
                        reinterpret_cast<DWORD(WINAPI*)(HANDLE ProcessHandle)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSuspendProcess"))(handLe);
                        CloseHandle(handLe);
                    }
                    break;
                }
                case 2:
                {
                    HANDLE handLe = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DataSource_[selected_].GetPid());
                    if (handLe)
                    {
                        reinterpret_cast<DWORD(WINAPI*)(HANDLE ProcessHandle)>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtResumeProcess"))(handLe);
                        CloseHandle(handLe);
                    }
                    break;
                }
                case 3:
                {
                    break;
                }
                case 4:
                {
                    utils::file::OpenFilePropertyDlg(DataSource_[selected_].GetFullPath().c_str());
                    break;
                }
            }
            ImGui::Separator();
            switch (int s = render::get_instasnce()->DrawItemBlock({ u8"ת���ڴ�",u8"�ļ���λ" }))
            {
                case 0:
                {
                    std::filesystem::path tmp(DataSource_[selected_].GetFullPath());
                    std::string dump_name = tmp.filename().string();
                    if (utils::process::DumpMemory(DataSource_[selected_].GetPid(), utils::conver::string_to_wstirng(dump_name).c_str(), ("dump_"+dump_name).c_str()))
                    {
                        MessageBoxA(NULL,"dump ok!","NoBanMe",NULL);
                    }
                    else
                    {
                        MessageBoxA(NULL, "dump failed!", "NoBanMe", NULL);
                    }
                    break;
                }
                case 1:
                {
                    utils::file::OpenFolderAndSelectFile(DataSource_[selected_].GetFullPath().c_str());
                    break;
                }
            }
            ImGui::Separator();
            //u8"ע��Dll",
            if (ImGui::BeginMenu(u8"ע��Dll"))
            {

                int s = render::get_instasnce()->DrawItemBlock({ u8"Զ�߳�ע��",u8"�ڴ�ע��",u8"APCע��" });
                if (s >= 0)
                {
                    std::string dllPath{};
                    dllPath = utils::file::openFileDlg(render::get_instasnce()->GetHwnd());
                    if (dllPath.empty())
                    {
                        MessageBoxA(NULL, "Dll·������ע��ʧ��", "NoBanMe", NULL);
                    }
                    else
                    {
                        switch (s)
                        {
                            case 0:
                            {

                                if (MemStub::RemoteInjectDLL(DataSource_[selected_].GetPid(), dllPath.c_str()))
                                {
                                    MessageBoxA(NULL, "ע��ɹ�!", "NoBanMe", NULL);
                                }
                                else
                                {
                                    MessageBoxA(NULL, "ע��ʧ��!", "NoBanMe", NULL);
                                }
                                break;
                            }
                        }
                    }
                }
                ImGui::EndMenu();
            }

            switch (int s = render::get_instasnce()->DrawItemBlock({ u8"ɨ�����",u8"���Բ��" }))
            {
				case 0:
				{
                    scanHooksWidget_.Load();
					break;
				}
                case 1:
                {
                    const char* plugin_name = DataSource_[selected_].IsWow64() ? "plugin_32.dll" : "plugin_64.dll";
                    static uint64_t  Offset_Dispatch = 0;
                    static char plugin_path[MAX_PATH]{};
                    bool can = true;
                    HMODULE plugin_base = NULL;
                    //��ȡ����ص�ƫ��
                    if (Offset_Dispatch <= 0)
                    {
                        HMODULE plugin_base = GetModuleHandleA(plugin_name);
                        if (plugin_base == NULL)
                        {
                            plugin_base = LoadLibraryA(plugin_name);
                            if (!plugin_base)
                            {
                                MessageBoxA(NULL, "����ļ�����ʧ��,�����ļ����Ƿ����!", "NoBanMe", NULL);
                                break;//��ǰ��¼������ע��Ľ���һ�£���ֹע��
                            }
                        }

                        Offset_Dispatch = (uint64_t)(reinterpret_cast<uint8_t*>(GetProcAddress(plugin_base, "Dispatch")) - (uint8_t*)plugin_base);
                        //printf("[DBG]plugin_base=%p\n", plugin_base);
                        //printf("[DBG]Offset_Dispatch=%p\n", Offset_Dispatch);
                        GetModuleFileNameA(plugin_base, plugin_path, MAX_PATH);
                        FreeLibrary(plugin_base);
                    }

                    //���Ŀ������Ƿ��й�ע����
                    plugin_base = MemStub::GetProcessModuleHandle(DataSource_[selected_].GetPid(), utils::conver::string_to_wstirng(plugin_name).c_str());
                    if (plugin_base)
                    {
                        //���»ص���ַ
                        global::plugin::lpPluginDispatch = reinterpret_cast<uint64_t>(plugin_base) + Offset_Dispatch;
                        if (pluginProcess_.GetPid() == DataSource_[selected_].GetPid())
                        {
                            MessageBoxA(NULL, "Ŀ������Ѿ����ò��,�����ظ�����!", "NoBanMe", NULL);
                            break;//��ǰ��¼������ע��Ľ���һ�£���ֹע��
                        }
                        else
                        {
                            if (pluginProcess_.GetPid() <= 0)
                                pluginProcess_ = DataSource_[selected_];
                            //������һ������
                            do
                            {
                                //1)�����ȫ���ϵ�
                                for (int i = 0; i < 4; i++)
                                    global::dbg::Dr->statue = 0; //��������0��OnUpdate���Զ�ɾ���ϵ�
                                Sleep(500);
                                //2)�����veh
                                if (!utils::mem::InvokeRemoteFunction(pluginProcess_.GetPid(), global::plugin::lpPluginDispatch, { ECMD::veh_uninstall }))
                                {
                                    MessageBoxA(NULL, "���ò��ʧ��[1]!", NULL, NULL);
                                    can = false;
                                    break;
                                }
                                //3)�����syscall
                                if (!utils::mem::InvokeRemoteFunction(pluginProcess_.GetPid(), global::plugin::lpPluginDispatch, { ECMD::syscallmonitor_uninstall }))
                                {
                                    MessageBoxA(NULL, "���ò��ʧ��[2]!", NULL, NULL);
                                    can = false;
                                    break;
                                }
                                //3)�����pipe
                                if (!utils::mem::InvokeRemoteFunction(pluginProcess_.GetPid(), global::plugin::lpPluginDispatch, { ECMD::pipe_client_close }))
                                {
                                    MessageBoxA(NULL, "���ò��ʧ��[3]!", NULL, NULL);
                                    can = false;
                                    break;
                                }
                                //3)�����plugin
                                if (!utils::mem::InvokeRemoteFunction(pluginProcess_.GetPid(), global::plugin::lpPluginDispatch, { ECMD::plugin_uninstall }))
                                {
                                    MessageBoxA(NULL, "���ò��ʧ��[4]!", NULL, NULL);
                                    can = false;
                                    break;
                                }
                                //����ɹ�
                                pluginProcess_ = ProcessItem();
                            } while (false);
                        }
                    }
                    if (can)
                    {
                        char caption[256]{};
                        sprintf_s(caption, "[%d]%s\n�Ƿ���Ҫ���ò��?", DataSource_[selected_].GetPid(), DataSource_[selected_].GetName().c_str());
                        if (MessageBoxA(NULL, caption, NULL, MB_OKCANCEL) == IDOK)
                        {
                            do
                            {
                                MemStub::RemoteInjectDLL(DataSource_[selected_].GetPid(), plugin_path);
                                plugin_base = MemStub::GetProcessModuleHandle(DataSource_[selected_].GetPid(), utils::conver::string_to_wstirng(plugin_name).c_str());
                                if (!plugin_base)
                                {
                                    MessageBoxA(NULL, "ע����ʧ��!", "NoBanMe", NULL);
                                    break;
                                }
                                if (!global::plugin::plugin_.create())
                                {
                                    MessageBoxA(NULL, "�����ܵ�ʧ��!", "NoBanMe", NULL);
                                    break;
                                }
                                global::plugin::lpPluginDispatch = reinterpret_cast<uint64_t>(plugin_base) + Offset_Dispatch;
                                if (!utils::mem::InvokeRemoteFunction(DataSource_[selected_].GetPid(), global::plugin::lpPluginDispatch, { ECMD::pipe_client_connect }))
                                {
                                    MessageBoxA(NULL, "���ӹܵ�ʧ��!", NULL, NULL);
                                    break;
                                }
                                if (!utils::mem::InvokeRemoteFunction(DataSource_[selected_].GetPid(), global::plugin::lpPluginDispatch, { ECMD::veh_init }))
                                {
                                    MessageBoxA(NULL, "��ʼ�����ʧ��!", NULL, NULL);
                                    break;
                                }
                                MessageBoxA(NULL, "�����ʼ���ɹ�!", NULL, NULL);
                                pluginProcess_ = DataSource_[selected_];
                            } while (false);
                        }
                    }
                    else
                    {
                        MessageBoxA(NULL, "��ʼ�����ʧ��!", NULL, NULL);
                    }
                    break;
                }
            }
            ImGui::Separator();
            if (ImGui::BeginMenu(u8"����"))
            {
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"����ID",u8"������",u8"������ID",u8"����ʱ��"  ,u8"����" ,u8"�ļ�����" ,u8"�ļ��汾",u8"����·��",u8"������" }))
                {
                    case 0:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), std::to_string(DataSource_[selected_].GetPid()).c_str());
                        break;
                    }
                    case 1:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), DataSource_[selected_].GetName().c_str());
                        break;
                    }
                    case 2:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), std::to_string(DataSource_[selected_].GetPPid()).c_str());
                        break;
                    }
                    case 3:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), DataSource_[selected_].GetStartUpTime().c_str());
                        break;
                    }
                    case 4:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), DataSource_[selected_].GetDecription().c_str());
                        break;
                    }
                    case 5:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), DataSource_[selected_].GetCompanyName().c_str());
                        break;
                    }
                    case 6:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), DataSource_[selected_].GetFileVersion().c_str());
                        break;
                    }
                    case 7:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), DataSource_[selected_].GetFullPath().c_str());
                        break;
                    }
                    case 8:
                    {
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), DataSource_[selected_].GetCmdLine().c_str());
                        break;
                    }
                }
                ImGui::Separator();
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"����" ,u8"������" }))
                {
                    case 0:
                    {
                        char buff[8192]{};
                        sprintf_s(buff, "%d | %s | %d | %s | %s | %s | %s | %s | %s", DataSource_[selected_].GetPid(),
                            DataSource_[selected_].GetName().c_str(), DataSource_[selected_].GetPPid(),
                            DataSource_[selected_].GetStartUpTime().c_str(), DataSource_[selected_].GetDecription().c_str(),
                            DataSource_[selected_].GetCompanyName().c_str(), DataSource_[selected_].GetFileVersion().c_str(),
                            DataSource_[selected_].GetFullPath().c_str(), DataSource_[selected_].GetCmdLine().c_str());
                        utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), buff);
                        break;
                    }
                    case 1:
                    {
                        std::string ret{};
                        for (auto p : DataSource_)
                        {
                            char buff[8192]{};
                            sprintf_s(buff, "%d | %s | %d | %s | %s | %s | %s | %s | %s\n", p.GetPid(),
                                p.GetName().c_str(), p.GetPPid(),
                                p.GetStartUpTime().c_str(), p.GetDecription().c_str(),
                                p.GetCompanyName().c_str(), p.GetFileVersion().c_str(),
                                p.GetFullPath().c_str(), p.GetCmdLine().c_str());
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

        threadWidget_.OnPaint();
        exceptionWidget_.OnPaint();
        memoryWidget_.OnPaint();
        scanHooksWidget_.OnPaint();
        ImGui::EndTable();
    }
}

ProcessItem ProcessWidget::GetCurtProcessItem()
{
    if(DataSource_.size()>0 && selected_>=0)
        return DataSource_[selected_];
    return ProcessItem();
}

ProcessItem ProcessWidget::GetPluginProcessItem()
{
    return pluginProcess_;
}
