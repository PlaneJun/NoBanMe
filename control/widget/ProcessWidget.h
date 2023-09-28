#pragma once

class ProcessWidget
{
public:
	void OnPaint()
	{
        //如果没有获取过则获取一次
        if (DataSource_.size() == 0)
        {
            ProcessItem::EnumCurtAllProcess(DataSource_);
        }
        //绘制
        if (ImGui::BeginTable("#processlist", 9, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV))
        {
            ImGui::TableSetupColumn("##icon", ImGuiTableColumnFlags_NoSort, 0.0f);
            ImGui::TableSetupColumn(u8"进程ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::PID);
            ImGui::TableSetupColumn(u8"进程名", ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::NAME);
            ImGui::TableSetupColumn(u8"父进程ID", ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::PPID);
            ImGui::TableSetupColumn(u8"创建时间", ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::STARTUPTIME);
            ImGui::TableSetupColumn(u8"描述", ImGuiTableColumnFlags_WidthFixed, 0.0f);
            ImGui::TableSetupColumn(u8"文件厂商", ImGuiTableColumnFlags_WidthFixed, 0.0f);
            ImGui::TableSetupColumn(u8"文件版本", ImGuiTableColumnFlags_WidthFixed, 0.0f);
            ImGui::TableSetupColumn(u8"进程路径", ImGuiTableColumnFlags_WidthFixed, 0.0f, ProcessItem::EInfo::FULLPATH);
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
                    ImGui::PopID();
                }
            }

            if (selected_ != -1 && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(1))
                ImGui::OpenPopup("process_option");

            if (ImGui::BeginPopup("process_option"))
            {
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"刷新进程" }))
                {
                    case 0:
                    {
                        DataSource_.clear();
                        break;
                    }
                }
                ImGui::Separator();
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"枚举窗口",u8"内存列表",u8"线程列表",u8"进程句柄",u8"异常回调" }))
                {
                    case 1:
                    {
                        memoryWidget.SetDataSource(DataSource_[selected_].GetPid());
                        memoryWidget.Load();
                        break;
                    }
                    case 2:
                    {
                        threadWidget.SetDataSource(DataSource_[selected_].GetPid());
                        threadWidget.Load();
                        break;
                    }
                    case 4:
                    {
                        exceptionWidget.SetDataSource(DataSource_[selected_].GetPid());
                        exceptionWidget.Load();
                        break;
                    }
                }
                ImGui::Separator();
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"结束进程",u8"隐藏进程",u8"进程属性" }))
                {
                    case 0:
                    {
                        HANDLE handLe = OpenProcess(PROCESS_ALL_ACCESS, FALSE, DataSource_[selected_].GetPid());
                        if (handLe)
                        {
                            TerminateProcess(handLe, 0);
                            CloseHandle(handLe);
                        }
                        break;
                    }
                    case 2:
                    {
                        utils::file::OpenFilePropertyDlg(DataSource_[selected_].GetFullPath().c_str());
                        break;
                    }
                }
                ImGui::Separator();
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"转储内存",u8"拷贝文件",u8"文件定位" }))
                {
                    case 0:
                    {

                        break;
                    }
                    case 2:
                    {
                        utils::file::OpenFolderAndSelectFile(DataSource_[selected_].GetFullPath().c_str());
                        break;
                    }
                }
                ImGui::Separator();
                //u8"注入Dll",
                if (ImGui::BeginMenu(u8"注入Dll"))
                {

                    int s = render::get_instasnce()->DrawItemBlock({ u8"远线程注入",u8"内存注入",u8"APC注入" });
                    if (s >= 0)
                    {
                        std::string dllPath{};
                        dllPath = utils::file::openFileDlg();
                        if (dllPath.empty())
                        {
                            MessageBoxA(NULL, "Dll路径有误，注入失败", "pjark", NULL);
                        }
                        else
                        {
                            switch (s)
                            {
                                case 0:
                                {

                                    if (Mem::RemoteInjectDLL(DataSource_[selected_].GetPid(), dllPath.c_str()))
                                    {
                                        MessageBoxA(NULL, "注入成功!", "pjark", NULL);
                                    }
                                    else
                                    {
                                        MessageBoxA(NULL, "注入失败!", "pjark", NULL);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    ImGui::EndMenu();
                }
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"调试插件" }))
                {
                    case 0:
                    {
                        static const char* plugin_name = DataSource_[selected_].IsWow64() ? "pjveh_32.dll" : "pjveh_64.dll";
                        static uint64_t  Offset_Dispatch = 0;
                        static char plugin_path[MAX_PATH]{};

                        bool can = true;
                        HMODULE plugin_base = NULL;
                        //获取插件回调偏移
                        if (Offset_Dispatch <= 0)
                        {
                            HMODULE plugin_base = GetModuleHandleA(plugin_name);
                            if (plugin_base == NULL)
                            {
                                plugin_base = LoadLibraryA(plugin_name);
                                if (!plugin_base)
                                {
                                    MessageBoxA(NULL, "插件文件加载失败,请检查文件下是否存在!", "pjark", NULL);
                                    break;//先前记录过，且注入的进程一致，禁止注入
                                }
                            }

                            Offset_Dispatch = (uint64_t)(reinterpret_cast<uint8_t*>(GetProcAddress(plugin_base, "Dispatch")) - (uint8_t*)plugin_base);
                            GetModuleFileNameA(plugin_base, plugin_path, MAX_PATH);
                            FreeLibrary(plugin_base);
                        }

                        //检查目标进程是否有过注入插件
                        plugin_base = Mem::GetProcessModuleHandle(DataSource_[selected_].GetPid(), utils::conver::string_to_wstirng(plugin_name).c_str());
                        if (plugin_base)
                        {
                            //更新回调地址
                            config::global::lpPluginDispatch = reinterpret_cast<uint64_t>(plugin_base) + Offset_Dispatch;
                            if (pluginProcess_.GetPid() == DataSource_[selected_].GetPid())
                            {
                                MessageBoxA(NULL, "目标进程已经启用插件,无需重复启用!", "pjark", NULL);
                                break;//先前记录过，且注入的进程一致，禁止注入
                            }
                            else
                            {
                                if (pluginProcess_.GetPid() <= 0)
                                    pluginProcess_ = DataSource_[selected_];
                                //清理上一个环境
                                do
                                {
                                    //1)、清空全部断点
                                    for (int i = 0; i < 4; i++)
                                        config::dbg::Dr->statue = 0; //这里设置0，OnUpdate会自动删除断点
                                    Sleep(500);
                                    //2)、清除veh
                                    if (!utils::mem::InvokePluginFunction(pluginProcess_.GetPid(), { ECMD::veh_uninstall }))
                                    {
                                        MessageBoxA(NULL, "启用插件失败[1]!", NULL, NULL);
                                        can = false;
                                        break;
                                    }
                                    //3)、清除syscall
                                    if (!utils::mem::InvokePluginFunction(pluginProcess_.GetPid(), { ECMD::syscallmonitor_uninstall }))
                                    {
                                        MessageBoxA(NULL, "启用插件失败[2]!", NULL, NULL);
                                        can = false;
                                        break;
                                    }
                                    //3)、清除pipe
                                    if (!utils::mem::InvokePluginFunction(pluginProcess_.GetPid(), { ECMD::pipe_client_close }))
                                    {
                                        MessageBoxA(NULL, "启用插件失败[3]!", NULL, NULL);
                                        can = false;
                                        break;
                                    }
                                    //3)、清除plugin
                                    if (!utils::mem::InvokePluginFunction(pluginProcess_.GetPid(), { ECMD::plugin_uninstall }))
                                    {
                                        MessageBoxA(NULL, "启用插件失败[4]!", NULL, NULL);
                                        can = false;
                                        break;
                                    }
                                    //清理成功
                                    pluginProcess_ = ProcessItem();
                                } while (false);
                            }
                        }
                        if (can)
                        {
                            char caption[256]{};
                            sprintf_s(caption, "[%d]%s\n是否需要启用插件?", DataSource_[selected_].GetPid(), DataSource_[selected_].GetName().c_str());
                            if (MessageBoxA(NULL, caption, NULL, MB_OKCANCEL) == IDOK)
                            {
                                do
                                {
                                    Mem::RemoteInjectDLL(DataSource_[selected_].GetPid(), plugin_path);
                                    plugin_base = Mem::GetProcessModuleHandle(DataSource_[selected_].GetPid(), utils::conver::string_to_wstirng(plugin_name).c_str());
                                    if (!plugin_base)
                                    {
                                        MessageBoxA(NULL, "注入插件失败!", "pjark", NULL);
                                        break;
                                    }
                                    if (!config::global::plugin_.init_pipe())
                                    {
                                        MessageBoxA(NULL, "创建管道失败!", "pjark", NULL);
                                        break;
                                    }
                                    config::global::lpPluginDispatch = reinterpret_cast<uint64_t>(plugin_base) + Offset_Dispatch;
                                    if (!utils::mem::InvokePluginFunction(DataSource_[selected_].GetPid(), { ECMD::pipe_client_connect }))
                                    {
                                        MessageBoxA(NULL, "连接管道失败!", NULL, NULL);
                                        break;
                                    }
                                    if (!utils::mem::InvokePluginFunction(DataSource_[selected_].GetPid(), { ECMD::veh_init }))
                                    {
                                        MessageBoxA(NULL, "初始化插件失败!", NULL, NULL);
                                        break;
                                    }
                                    MessageBoxA(NULL, "插件初始化成功!", NULL, NULL);
                                    pluginProcess_ = DataSource_[selected_];
                                } while (false);
                            }
                        }
                        else
                        {
                            MessageBoxA(NULL, "初始化插件失败!", NULL, NULL);
                        }
                        break;
                    }
                }
                ImGui::Separator();
                if (ImGui::BeginMenu(u8"复制"))
                {
                    switch (int s = render::get_instasnce()->DrawItemBlock({ u8"进程ID",u8"进程名",u8"父进程ID",u8"创建时间"  ,u8"描述" ,u8"文件厂商" ,u8"文件版本",u8"进程路径" }))
                    {
                        case 0:
                        {
                            utils::normal::CopyStringToClipboard(std::to_string(DataSource_[selected_].GetPid()).c_str());
                            break;
                        }
                        case 1:
                        {
                            utils::normal::CopyStringToClipboard(DataSource_[selected_].GetName().c_str());
                            break;
                        }
                        case 2:
                        {
                            utils::normal::CopyStringToClipboard(std::to_string(DataSource_[selected_].GetPPid()).c_str());
                            break;
                        }
                        case 3:
                        {
                            utils::normal::CopyStringToClipboard(DataSource_[selected_].GetStartUpTime().c_str());
                            break;
                        }
                        case 4:
                        {
                            utils::normal::CopyStringToClipboard(DataSource_[selected_].GetDecription().c_str());
                            break;
                        }
                        case 5:
                        {
                            utils::normal::CopyStringToClipboard(DataSource_[selected_].GetCompanyName().c_str());
                            break;
                        }
                        case 6:
                        {
                            utils::normal::CopyStringToClipboard(DataSource_[selected_].GetFileVersion().c_str());
                            break;
                        }
                        case 7:
                        {
                            utils::normal::CopyStringToClipboard(DataSource_[selected_].GetFullPath().c_str());
                            break;
                        }
                    }
                    ImGui::Separator();
                    switch (int s = render::get_instasnce()->DrawItemBlock({ u8"整行" ,u8"整个表" }))
                    {
                        case 0:
                        {
                            char buff[8192]{};
                            sprintf_s(buff, "%d | %s | %d | %s | %s | %s | %s | %s", DataSource_[selected_].GetPid(),
                                DataSource_[selected_].GetName().c_str(), DataSource_[selected_].GetPPid(),
                                DataSource_[selected_].GetStartUpTime().c_str(), DataSource_[selected_].GetDecription().c_str(),
                                DataSource_[selected_].GetCompanyName().c_str(), DataSource_[selected_].GetFileVersion().c_str(),
                                DataSource_[selected_].GetFullPath().c_str());
                            utils::normal::CopyStringToClipboard(buff);
                            break;
                        }
                        case 1:
                        {
                            std::string ret{};
                            for (auto p : DataSource_)
                            {
                                char buff[8192]{};
                                sprintf_s(buff, "%d | %s | %d | %s | %s | %s | %s | %s\n", p.GetPid(),
                                    p.GetName().c_str(), p.GetPPid(),
                                    p.GetStartUpTime().c_str(), p.GetDecription().c_str(),
                                    p.GetCompanyName().c_str(), p.GetFileVersion().c_str(),
                                    p.GetFullPath().c_str());
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

            threadWidget.OnPaint();
            exceptionWidget.OnPaint();
            memoryWidget.OnPaint();
            ImGui::EndTable();
        }
	}

    auto GetCurtProcessItem()
    {
        if(DataSource_.size()>0 && selected_>=0)
            return DataSource_[selected_];
        return ProcessItem();
    }

    auto GetPluginProcessItem()
    {
        return pluginProcess_;
    }
private:
    int selected_ = -1;
    ProcessItem pluginProcess_;
	std::vector<ProcessItem> DataSource_;

    ThreadWidget threadWidget;
    ExceptionWidget exceptionWidget;
    MemoryWidget memoryWidget;
};
