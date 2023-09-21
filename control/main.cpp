#include "pch.h"

bool InvokePluginFunction(DWORD pid, ControlCmd cmd)
{
    return Mem::RemoteCallFunction(pid,config::global::lpPluginDispatch,&cmd,sizeof(ControlCmd));
}


void Table_Process()
{
    static std::vector<ProcessItem> pitems;
    static std::vector<ModuleItem> mitems;
    if (ImGui::BeginChild("#process", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.7), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        static int selected_process = -1;
        if (pitems.size() == 0)
        {
            ProcessItem::EnumCurtAllProcess(pitems);
        }
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
                    if (pitems.size() > 1)
                        qsort(&pitems[0], (size_t)pitems.size(), sizeof(pitems[0]), ProcessItem::CompareWithSortSpecs);
                    ProcessItem::SetSortSpecs( NULL);
                    sorts_specs->SpecsDirty = false;
                }
            }
            ImGuiListClipper clipper;
            clipper.Begin(pitems.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    ProcessItem* item = &pitems[row_n];
                    ImGui::PushID(item->GetPid());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (item->GetIcon())
                        ImGui::Image(item->GetIcon(), ImVec2(16, 16));
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(std::to_string(item->GetPid()).c_str(), selected_process == row_n, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        selected_process = row_n;
                        if (config::global::targetProcess.GetPid() != item->GetPid())
                        {
                            config::global::targetProcess = *item;
                            mitems.clear();
                        }
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::conver::string_To_UTF8(item->GetName().append(item->IsWow64()?"*32":"")).c_str());
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

            if (selected_process != -1 && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(1))
                ImGui::OpenPopup("process_option");
               
            if (ImGui::BeginPopup("process_option"))
            {
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"刷新进程" }))
                {
                    case 0:
                    {
                        pitems.clear();
                        break;
                    }
                }
                ImGui::Separator();
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"枚举窗口",u8"内存列表",u8"线程列表",u8"进程句柄",u8"异常回调"}))
                {
                    case 1:
                    {
                        config::process::memory::bShow = true;
                        config::process::memory::pid = config::global::targetProcess.GetPid();
                        break;
                    }
                    case 2:
                    {
                        config::process::thread::bShow = true;
                        config::process::thread::pid = config::global::targetProcess.GetPid();
                        break;
                    }
                    case 4:
                    {
                        config::process::veh::bShow = true;
                        config::process::veh::pid = config::global::targetProcess.GetPid();;
                        break;
                    }
                }
                ImGui::Separator();
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"结束进程",u8"隐藏进程",u8"进程属性"}))
                {
                    case 0:
                    {
                        HANDLE handLe = OpenProcess(PROCESS_ALL_ACCESS, FALSE, config::global::targetProcess.GetPid());
                        if (handLe)
                        {
                            TerminateProcess(handLe, 0);
                            CloseHandle(handLe);
                        }
                        break;
                    }
                    case 2:
                    {
                        utils::file::OpenFilePropertyDlg(config::global::targetProcess.GetFullPath().c_str());
                        break;
                    }
                }
                ImGui::Separator();
                switch(int s = render::get_instasnce()->DrawItemBlock({ u8"转储内存",u8"拷贝文件",u8"文件定位" }))
                {
                    case 0:
                    {
                        
                        break;
                    }
                    case 2:
                    {
                        utils::file::OpenFolderAndSelectFile(config::global::targetProcess.GetFullPath().c_str());
                        break;
                    }
                }
                ImGui::Separator();
                //u8"注入Dll",
                if (ImGui::BeginMenu(u8"注入Dll"))
                {
                    
                    int s = render::get_instasnce()->DrawItemBlock({ u8"远线程注入",u8"内存注入",u8"APC注入" });
                    if (s >=0)
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

                                    if (Mem::RemoteInjectDLL(config::global::targetProcess.GetPid(), dllPath.c_str()))
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
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"调试插件"}))
                {
                    case 0:
                    {
                        static const char* plugin_name = config::global::targetProcess.IsWow64() ? "pjveh_32.dll" : "pjveh_64.dll";
                        static uint64_t  Offset_Dispatch = 0;
                        static char plugin_path[MAX_PATH]{};

                        bool can = true;
                        HMODULE plugin_base = NULL;
                        //获取插件回调偏移
                        if (Offset_Dispatch <= 0)
                        {
                            HMODULE plugin_base = GetModuleHandleA(plugin_name);
                            if (plugin_base == NULL)
                                plugin_base = LoadLibraryA(plugin_name);
                            Offset_Dispatch = (uint64_t)(reinterpret_cast<uint8_t*>(GetProcAddress(plugin_base, "Dispatch")) - (uint8_t*)plugin_base);
                            GetModuleFileNameA(plugin_base, plugin_path, MAX_PATH);
                            FreeLibrary(plugin_base);
                        }

                        //检查目标进程是否有过注入插件
                        plugin_base = Mem::GetProcessModuleHandle(config::global::targetProcess.GetPid(), utils::conver::string_to_wstirng(plugin_name).c_str());
                        if (plugin_base)
                        {
                            //更新回调地址
                            config::global::lpPluginDispatch = reinterpret_cast<uint64_t>(plugin_base)+ Offset_Dispatch;
                            if (config::global::injectProcess.GetPid() == config::global::targetProcess.GetPid())
                            {
                                MessageBoxA(NULL, "目标进程已经启用插件,无需重复启用!", "pjark", NULL);
                                break;//先前记录过，且注入的进程一致，禁止注入
                            }
                            else
                            {
                                if (config::global::injectProcess.GetPid() <= 0)
                                    config::global::injectProcess = config::global::targetProcess;
                                //清理上一个环境
                                do
                                {
                                    //1)、清空全部断点
                                    for (int i = 0; i < 4; i++)
                                        config::dbg::Dr->statue = 0; //这里设置0，OnUpdate会自动删除断点
                                    Sleep(500);
                                    //2)、清除veh
                                    if (!InvokePluginFunction(config::global::injectProcess.GetPid(), { ECMD::veh_uninstall }))
                                    {
                                        MessageBoxA(NULL, "启用插件失败[1]!", NULL, NULL);
                                        can = false;
                                        break;
                                    }
                                    //3)、清除syscall
                                    if (!InvokePluginFunction(config::global::injectProcess.GetPid(), { ECMD::syscallmonitor_uninstall }))
                                    {
                                        MessageBoxA(NULL, "启用插件失败[2]!", NULL, NULL);
                                        can = false;
                                        break;
                                    }
                                    //3)、清除pipe
                                    if (!InvokePluginFunction(config::global::injectProcess.GetPid(), { ECMD::pipe_client_close }))
                                    {
                                        MessageBoxA(NULL, "启用插件失败[3]!", NULL, NULL);
                                        can = false;
                                        break;
                                    }
                                    //3)、清除plugin
                                    if (!InvokePluginFunction(config::global::injectProcess.GetPid(), { ECMD::plugin_uninstall }))
                                    {
                                        MessageBoxA(NULL, "启用插件失败[4]!", NULL, NULL);
                                        can = false;
                                        break;
                                    }
                                    //清理成功
                                    config::global::injectProcess = ProcessItem();
                                } while (false);
                            }
                        }
                        if (can)
                        {
                            char caption[256]{};
                            sprintf_s(caption, "[%d]%s\n是否需要启用插件?", config::global::targetProcess.GetPid(), config::global::targetProcess.GetName().c_str());
                            if (MessageBoxA(NULL, caption, NULL, MB_OKCANCEL) == IDOK)
                            {
                                do
                                {
                                    Mem::RemoteInjectDLL(config::global::targetProcess.GetPid(), plugin_path);
                                    plugin_base = Mem::GetProcessModuleHandle(config::global::targetProcess.GetPid(), utils::conver::string_to_wstirng(plugin_name).c_str());
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
                                    if (!InvokePluginFunction(config::global::targetProcess.GetPid(), { ECMD::pipe_client_connect }))
                                    {
                                        MessageBoxA(NULL, "连接管道失败!", NULL, NULL);
                                        break;
                                    }
                                    if (!InvokePluginFunction(config::global::targetProcess.GetPid(), { ECMD::veh_init }))
                                    {
                                        MessageBoxA(NULL, "初始化插件失败!", NULL, NULL);
                                        break;
                                    }
                                    MessageBoxA(NULL, "插件初始化成功!", NULL, NULL);
                                    config::global::injectProcess = config::global::targetProcess;
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
                    switch (int s = render::get_instasnce()->DrawItemBlock({ u8"进程ID",u8"进程名",u8"父进程ID",u8"创建时间"  ,u8"描述" ,u8"文件厂商" ,u8"文件版本",u8"进程路径"  }))
                    {
                        case 0:
                        {
                            utils::normal::CopyStringToClipboard(std::to_string(config::global::targetProcess.GetPid()).c_str());
                            break;
                        }
                        case 1:
                        {
                            utils::normal::CopyStringToClipboard(config::global::targetProcess.GetName().c_str());
                            break;
                        }
                        case 2:
                        {
                            utils::normal::CopyStringToClipboard(std::to_string(config::global::targetProcess.GetPPid()).c_str());
                            break;
                        }
                        case 3:
                        {
                            utils::normal::CopyStringToClipboard(config::global::targetProcess.GetStartUpTime().c_str());
                            break;
                        }
                        case 4:
                        {
                            utils::normal::CopyStringToClipboard(config::global::targetProcess.GetDecription().c_str());
                            break;
                        }
                        case 5:
                        {
                            utils::normal::CopyStringToClipboard(config::global::targetProcess.GetCompanyName().c_str());
                            break;
                        }
                        case 6:
                        {
                            utils::normal::CopyStringToClipboard(config::global::targetProcess.GetFileVersion().c_str());
                            break;
                        }
                        case 7:
                        {
                            utils::normal::CopyStringToClipboard(config::global::targetProcess.GetFullPath().c_str());
                            break;
                        }
                    }
                    ImGui::Separator();
                    switch (int s = render::get_instasnce()->DrawItemBlock({ u8"整行" ,u8"整个表"}))
                    {
                        case 0:
                        {
                            char buff[8192]{};
                            sprintf_s(buff,"%d | %s | %d | %s | %s | %s | %s | %s", config::global::targetProcess.GetPid(),
                                config::global::targetProcess.GetName().c_str(), config::global::targetProcess.GetPPid(),
                                config::global::targetProcess.GetStartUpTime().c_str(), config::global::targetProcess.GetDecription().c_str(),
                                config::global::targetProcess.GetCompanyName().c_str(), config::global::targetProcess.GetFileVersion().c_str(),
                                config::global::targetProcess.GetFullPath().c_str());
                            utils::normal::CopyStringToClipboard(buff);
                            break;
                        }
                        case 1:
                        {
                            std::string ret{};
                            for (auto p : pitems)
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
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
    if (ImGui::BeginChild("#module", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        static int selected_module = -1;
        if (config::global::targetProcess.GetPid() != 0)
        {
            //Enum Modules
            if (mitems.size() == 0)
            {
                ModuleItem::EnumPidModules(config::global::targetProcess.GetPid(), mitems);
            }
        }
        if (ImGui::BeginTable("#modulelist", 6, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV))
        {
            ImGui::TableSetupColumn(u8"模块路径", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, ModuleItem::EInfo::IMAGEPATH);
            ImGui::TableSetupColumn(u8"基地址", ImGuiTableColumnFlags_WidthFixed, 0.0f, ModuleItem::EInfo::BASE);
            ImGui::TableSetupColumn(u8"大小", ImGuiTableColumnFlags_WidthFixed, 0.0f, ModuleItem::EInfo::SIZE);
            ImGui::TableSetupColumn(u8"描述", ImGuiTableColumnFlags_WidthFixed, 0.0f);
            ImGui::TableSetupColumn(u8"文件厂商", ImGuiTableColumnFlags_WidthFixed, 0.0f);
            ImGui::TableSetupColumn(u8"文件版本", ImGuiTableColumnFlags_WidthFixed, 0.0f);
            ImGui::TableHeadersRow();
            if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
            {
                if (sorts_specs->SpecsDirty)
                {
                    ModuleItem::SetSortSpecs(sorts_specs); // Store in variable accessible by the sort function.
                    if (mitems.size() > 1)
                        qsort(&mitems[0], (size_t)mitems.size(), sizeof(mitems[0]), ModuleItem::CompareWithSortSpecs);
                    ModuleItem::SetSortSpecs(NULL);
                    sorts_specs->SpecsDirty = false;
                }
            }
            ImGuiListClipper clipper;
            clipper.Begin(mitems.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    ModuleItem* item = &mitems[row_n];
                    ImGui::PushID(item->GetBase());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(utils::conver::string_To_UTF8(item->GetImagePath()).c_str(), selected_module == row_n, ImGuiSelectableFlags_SpanAllColumns))
                        selected_module = row_n;
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%p", item->GetBase());
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%llx", item->GetSize());
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::conver::string_To_UTF8(item->GetDecription()).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::conver::string_To_UTF8(item->GetCompanyName()).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::conver::string_To_UTF8(item->GetFileVersion()).c_str());
                    ImGui::PopID();
                }
            }

            if (selected_module != -1 && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(1))
                ImGui::OpenPopup("module_option");

            if (ImGui::BeginPopup("module_option"))
            {
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"内存转储" }))
                {
                    case 0:
                    {
                        
                        break;
                    }
                }
                ImGui::Separator();
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"文件定位",u8"文件属性" }))
                {
                    case 0:
                    {
                        utils::file::OpenFolderAndSelectFile(mitems[selected_module].GetImagePath().c_str());
                        break;
                    }
                    case 1:
                    {
                        utils::file::OpenFilePropertyDlg(mitems[selected_module].GetImagePath().c_str());
                        break;
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}
void Table_SyscallMonitor()
{
    ImGui::SeparatorText(u8"选项");
    ImGui::Checkbox(u8"启用", &config::syscall::active); ImGui::SameLine();
    ImGui::Checkbox(u8"保存日志", &config::syscall::save); ImGui::SameLine();
    ImGui::Button(u8"清除");
    ImGui::SeparatorText(u8"输出");
    if (ImGui::TreeNode("root"))
    {
        for (auto n : config::syscall::monitor)
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
void Table_VehDebuger()
{
    static std::vector<std::vector<std::string>> dissambly_block{};
    static uint64_t dissambly_base = 0;
    static uint64_t dissambly_jmp = 0;
    static uint32_t curt_page_index = 0;
    static uint32_t max_page_index = 0;
    static uint8_t wheel_count = 0; //轮动记录5次翻页

    auto curtData = Debugger::GetDbgInfo(config::dbg::curtChoose);
    static Debugger::DbgCaptureInfo JmpToAddress{}; //触发记录中的选择项

    if (ImGui::CollapsingHeader(u8"硬件断点", ImGuiTreeNodeFlags_DefaultOpen))
    {
        static const char* dr_statue[] = { u8"添加",u8"移除" };
        static const char* dr_size[] = { "1 byte", "2 byte", "4 byte", "8 byte" };
        static const char* dr_type[] = { u8"执行", u8"读/写", u8"写" };
        for (int i = 0;i < 4;i++)
        {
            char check_title[100]{};
            sprintf_s(check_title, u8"启用     Dr%d:", i);
            if (config::dbg::Dr[i].statue == 0)//未添加断点时不可启用
                ImGui::BeginDisabled();
            ImGui::Checkbox(check_title, &config::dbg::Dr[i].active); ImGui::SameLine();
            if (config::dbg::Dr[i].statue == 0)
                ImGui::EndDisabled();

            //启用时,不可修改地址、类型、大小
            if (config::dbg::Dr[i].active)
                ImGui::BeginDisabled();
            char input_title[100]{};
            sprintf_s(input_title, "##input_dr%d", i);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3);
            ImGui::InputTextWithHint(input_title, "adrress", config::dbg::Dr[i].addr, 1024); ImGui::SameLine();
            ImGui::PopItemWidth();

            ImGui::Text(u8"类型:");ImGui::SameLine();
            char combo_type_title[100]{};
            sprintf_s(combo_type_title, "##combot_type_dr%d", i);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.2);
            ImGui::Combo(combo_type_title, &config::dbg::Dr[i].type, dr_type, IM_ARRAYSIZE(dr_type));ImGui::SameLine();
            ImGui::PopItemWidth();

            ImGui::Text(u8"大小:");ImGui::SameLine();
            char combo_size_title[100]{};
            sprintf_s(combo_size_title, "##combot_size_dr%d", i);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.2);
            ImGui::Combo(combo_size_title, &config::dbg::Dr[i].size, dr_size, IM_ARRAYSIZE(dr_size));ImGui::SameLine();
            ImGui::PopItemWidth();
            if (config::dbg::Dr[i].active)
                ImGui::EndDisabled();
            char button_id[100]{};
            sprintf_s(button_id, "btn_dr%d", i);
            ImGui::PushID(button_id);
            if (ImGui::Button(dr_statue[config::dbg::Dr[i].statue]))
            {
                config::dbg::Dr[i].statue++;
                if (config::dbg::Dr[i].statue > 1)
                    config::dbg::Dr[i].statue = 0;
            }
            ImGui::PopID();
        }
    }
    if (ImGui::CollapsingHeader(u8"数据窗口", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::BeginTabBar("DrSelector_Tables", ImGuiTabBarFlags_None))
        {
            for (int i = 0;i < 4;i++)
            {
                char buf[10]{};
                sprintf_s(buf, "Dr%d", i);
                if (ImGui::BeginTabItem(buf))
                {
                    config::dbg::curtChoose = i;
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
        if (ImGui::BeginChild("ChildAccess", ImVec2(ImGui::GetContentRegionAvail().x * 0.25, ImGui::GetContentRegionAvail().y * 0.66), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImGui::SeparatorText(u8"触发记录");
            static int selected = -1;
            static const std::vector<std::string> headers = { u8"计数" ,u8"地址" ,u8"指令" };
            std::vector<std::vector<std::string>> text{};
            for (auto i : curtData.capture)
            {
                std::vector<std::string> tmp{};
                tmp.push_back(std::to_string(i.second.count));
                tmp.push_back(i.second.text);
                tmp.push_back(i.second.dbginfo.disassembly);
                text.push_back(tmp);
            }
            render::get_instasnce()->AddListBox("##bp_access", selected, 1, headers, text);
            if (selected != -1)
            {
                int i = 0;
                for (auto ii : curtData.capture)
                {
                    if (i == selected)
                    {
                        curtData.ctx = ii.second.dbginfo.ctx; //设置上下文显示的数据
                        if (JmpToAddress.text != ii.second.text)
                        {
                            dissambly_jmp = utils::conver::hexToInteger(ii.second.text);
                            JmpToAddress = ii.second; //设置要跳转的地址
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
                                utils::normal::CopyStringToClipboard(std::to_string(JmpToAddress.count).c_str());
                                break;
                            }
                            case 1:
                            {
                                utils::normal::CopyStringToClipboard(JmpToAddress.text.c_str());
                                break;
                            }
                            case 2:
                            {
                                utils::normal::CopyStringToClipboard(JmpToAddress.dbginfo.disassembly);
                                break;
                            }
                        }
                        ImGui::Separator();
                        switch (int s = render::get_instasnce()->DrawItemBlock({ u8"整行",u8"整个表" }))
                        {
                            case 0:
                            {
                                char buff[8192]{};
                                sprintf_s(buff, "%d | %s | %s", JmpToAddress.count, JmpToAddress.text.c_str(), JmpToAddress.dbginfo.disassembly);
                                utils::normal::CopyStringToClipboard(buff);
                                break;
                            }
                            case 1:
                            {
                                std::string ret{};
                                for (auto i : curtData.capture)
                                {
                                    char buff[8192]{};
                                    sprintf_s(buff, "%d | %s | %s", i.second.count,i.second.text.c_str(), i.second.dbginfo.disassembly);
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
            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (ImGui::BeginChild("ChildDisassembly", ImVec2(ImGui::GetContentRegionAvail().x * 0.66, ImGui::GetContentRegionAvail().y * 0.66)))
        {
            //按照64kb显示
            static int selected = -1;
            ImGui::SeparatorText(u8"反汇编");
            if (ImGui::BeginTable("#disassembly", 3, ImGuiTableFlags_Resizable|ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
            {
                ImGui::TableSetupColumn(u8"地址", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(u8"字节码", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(u8"指令", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();
                //默认注入的进程
                dissambly_base = (uint64_t)Mem::GetProcessModuleHandle(config::global::injectProcess.GetPid(), utils::conver::string_to_wstirng(config::global::injectProcess.GetName()).c_str());
                if (dissambly_base > 0)
                {
                    if (dissambly_jmp >0)
                    {
                        curt_page_index = (dissambly_jmp - dissambly_base) / Debugger::DISAM_BLOCK;
                        dissambly_block.clear();
                    }

                getdisam:
                    uint64_t disam_start = dissambly_base + curt_page_index * Debugger::DISAM_BLOCK;
                    if (dissambly_block.size() <= 0)
                    {
                        curtData.memoryRegion.start = disam_start;
                        curtData.memoryRegion.data.resize(Debugger::DISAM_BLOCK);
                        //read mem
                        Mem::ReadMemory(config::global::targetProcess.GetPid(), curtData.memoryRegion.start, &curtData.memoryRegion.data[0], Debugger::DISAM_BLOCK);
                        dissambly_block = Debugger::Disassembly(!config::global::targetProcess.IsWow64(), disam_start, curtData.memoryRegion.data.data(), Debugger::DISAM_BLOCK);
                    }
                    else
                    {
                        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() && ImGui::GetIO().MouseWheel < 0) //down
                        {
                            if (wheel_count > 3)
                            {
                                curt_page_index++;
                                dissambly_block.clear();
                                wheel_count = 0;
                                goto getdisam;
                            }
                            else
                            {
                                wheel_count++;
                            }

                        }
                        else if (ImGui::GetScrollY() <= 0 && curt_page_index > 0 && ImGui::GetIO().MouseWheel > 0) //up
                        {
                            if (wheel_count > 3)
                            {
                                curt_page_index--;
                                dissambly_block.clear();
                                wheel_count = 0;
                                goto getdisam;
                            }
                            else
                            {
                                wheel_count++;
                            }
                        }
                    }

                    for (int i = 0; i < dissambly_block.size(); i++)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        if (ImGui::Selectable(dissambly_block[i][0].c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns))
                            selected = i;
                        for (int j = 1; j < dissambly_block[i].size(); j++)
                        {
                            ImGui::TableSetColumnIndex(j);
                            ImGui::Text(dissambly_block[i][j].c_str()); ImGui::NextColumn();
                        }

                        //如果到了指定行就停止滚动
                        if (dissambly_jmp > 0 && utils::conver::IntegerTohex(dissambly_jmp) == dissambly_block[i][0])
                        {
                            selected = i; //只在跳转的时候选择一次
                            dissambly_jmp = 0;
                        }
                        if (dissambly_jmp > 0)
                        {
                            ImGui::SetScrollHereY(0);
                        }

                    }
                    dissambly_jmp = 0;//无论有没有找到都设置为0
                }
                ImGui::EndTable();
            }
 
            if (selected != -1)
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
                                utils::normal::CopyStringToClipboard(dissambly_block[selected][0].c_str());
                                break;
                            }
                            case 1:
                            {
                                utils::normal::CopyStringToClipboard(dissambly_block[selected][1].c_str());
                                break;
                            }
                            case 2:
                            {
                                utils::normal::CopyStringToClipboard(dissambly_block[selected][2].c_str());
                                break;
                            }
                        }
                        ImGui::Separator();
                        switch (int s = render::get_instasnce()->DrawItemBlock({ u8"整行"}))
                        {
                            case 0:
                            {
                                char buff[8192]{};
                                sprintf_s(buff, "%d | %s | %s", dissambly_block[selected][0].c_str(), dissambly_block[selected][1].c_str(), dissambly_block[selected][2].c_str());
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
                    ImGui::InputText("##JmpToInputAddress",buf,100, ImGuiInputTextFlags_CharsHexadecimal);
                    if (ImGui::Button(u8"确认", ImVec2(120, 0))) 
                    {
                        dissambly_jmp = utils::conver::hexToInteger(buf);
                        ImGui::CloseCurrentPopup(); 
                    }
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button(u8"取消", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }
            }

            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (ImGui::BeginChild("ChildContext", ImVec2(ImGui::GetContentRegionAvail().x * 0.98, ImGui::GetContentRegionAvail().y * 0.66)))
        {
            ImGui::SeparatorText(u8"寄存器");ImGui::SameLine();
            render::get_instasnce()->HelpMarker(u8"当前显示的上下文为断点执行后数据\n");
            static int selected = -1;
            std::vector<std::string> headers = { u8"Reg" ,u8"Value" };
            std::vector<std::pair<std::string, std::string>> text = {
                {"RAX",utils::conver::IntegerTohex(curtData.ctx.Rax)},
                {"RBX",utils::conver::IntegerTohex(curtData.ctx.Rbx)},
                {"RCX",utils::conver::IntegerTohex(curtData.ctx.Rcx) },
                {"RDX",utils::conver::IntegerTohex(curtData.ctx.Rdx)},
                {"RBP",utils::conver::IntegerTohex(curtData.ctx.Rbp)},
                {"RSP",utils::conver::IntegerTohex(curtData.ctx.Rsp)},
                {"RSI",utils::conver::IntegerTohex(curtData.ctx.Rsi)},
                {"RDI",utils::conver::IntegerTohex(curtData.ctx.Rdi)},
                { "R8" ,utils::conver::IntegerTohex(curtData.ctx.R8) },
                { "R9",utils::conver::IntegerTohex(curtData.ctx.R9) },
                { "R10",utils::conver::IntegerTohex(curtData.ctx.R10) },
                { "R11",utils::conver::IntegerTohex(curtData.ctx.R11) },
                { "R12",utils::conver::IntegerTohex(curtData.ctx.R12) },
                { "R13" ,utils::conver::IntegerTohex(curtData.ctx.R13)},
                { "R14",utils::conver::IntegerTohex(curtData.ctx.R14) },
                { "R15",utils::conver::IntegerTohex(curtData.ctx.R15) },
                { "RIP" ,utils::conver::IntegerTohex(curtData.ctx.Rip) },
                { "RFLAGS" ,utils::conver::IntegerTohex(curtData.ctx.EFlags) },
                {"Xmm0",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm0)},
                {"Xmm1",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm1)},
                {"Xmm2",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm2)},
                {"Xmm3",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm3)},
                {"Xmm4",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm4)},
                {"Xmm5",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm5)},
                {"Xmm6",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm6)},
                {"Xmm7",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm7)},
                {"Xmm8",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm8)},
                {"Xmm9",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm9)},
                {"Xmm10",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm10)},
                {"Xmm11",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm11)},
                {"Xmm12",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm12)},
                {"Xmm13",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm13)},
                {"Xmm14",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm14)},
                {"Xmm15",utils::conver::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm15)}
            };
            render::get_instasnce()->AddListBox("##context", selected, headers, text);
            ImGui::EndChild();
        }
        ImGui::Separator();
        if (ImGui::BeginChild("ChildMem", ImVec2(ImGui::GetContentRegionAvail().x * 0.65, ImGui::GetContentRegionAvail().y)))
        {
            ImGui::SeparatorText(u8"内存");
            static MemoryEditor mem_edit;
            mem_edit.DrawWindow("Memory Editor", curtData.memoryRegion.data.data(), curtData.memoryRegion.data.size(), curtData.memoryRegion.start);
            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (ImGui::BeginChild("ChildStack"))
        {
            //-------------------------------------------------
            //待优化，目前代码一坨屎
            ImGui::SeparatorText(u8"堆栈");
            static int selected = -1;
            static std::vector<ModuleItem> mitems{};
            static std::vector<std::string> headers = { "Rsp" ,"Value"," " };
            static std::vector<std::vector<std::string>> text{};
            if (text.size() <= 0)
            {
                //这个位置将来兼容32的话,8需要改，建议后面动态获取
                if (curtData.capture.count(JmpToAddress.dbginfo.ctx.Rip) > 0)
                {
                    if(mitems.size()<=0)
                        ModuleItem::EnumPidModules(config::global::targetProcess.GetPid(), mitems);

                    auto cap = curtData.capture[JmpToAddress.dbginfo.ctx.Rip];
                    for (int i = 0; i < sizeof(cap.stack) / 8; i++)
                    {
                        std::vector<std::string> tmp{};
                        uint64_t v = *(uint64_t*)&cap.stack[i * 8];
                        tmp.push_back(utils::conver::IntegerTohex(curtData.ctx.Rsp + i * 8));
                        tmp.push_back(utils::conver::IntegerTohex(v));
                        tmp.push_back("");
                        for (auto& item : mitems)
                        {
                            if (v >= item.GetBase() && v <= item.GetBase() + item.GetSize())
                            {
                                uint64_t detal = (item.GetBase() + item.GetSize()) - v;
                                tmp[2] = std::filesystem::path(item.GetImagePath()).filename().string().append(utils::conver::IntegerTohex(detal));
                                break;
                            }
                        }

                        text.push_back(tmp);
                    }
                }
            }
            else
            {
                if (text[0][0] != utils::conver::IntegerTohex(curtData.ctx.Rsp))
                {
                    mitems.clear();
                    selected = -1;
                }
            }

            if (selected != -1)
            {
                if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
                    ImGui::OpenPopup("stack_option");

                if (ImGui::BeginPopup("stack_option"))
                {
                    if (ImGui::BeginMenu(u8"复制"))
                    {
                        switch (int s = render::get_instasnce()->DrawItemBlock({ u8"Rsp",u8"Value",u8"注释" }))
                        {
                            case 0:
                            {
                                utils::normal::CopyStringToClipboard(text[selected][0].c_str());
                                break;
                            }
                            case 1:
                            {
                                utils::normal::CopyStringToClipboard(text[selected][1].c_str());
                                break;
                            }
                            case 2:
                            {
                                utils::normal::CopyStringToClipboard(text[selected][2].c_str());
                                break;
                            }
                        }
                        ImGui::Separator();
                        switch (int s = render::get_instasnce()->DrawItemBlock({ u8"整行" }))
                        {
                            case 0:
                            {
                                char buff[8192]{};
                                sprintf_s(buff, "%d | %s | %s", text[selected][0].c_str(), text[selected][1].c_str(), text[selected][2].c_str());
                                utils::normal::CopyStringToClipboard(buff);
                                break;
                            }
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }
            }

            render::get_instasnce()->AddListBox("##Stack", selected,0, headers, text);
            ImGui::EndChild();
        }
    }
}
void Table_Window()
{
    /*if (ImGui::BeginChild("##window_list", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::Text("11");
        ImGui::EndChild();
    }
    ImGui::SameLine();
    if (ImGui::BeginChild("##window_info", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::Text("22");
        ImGui::EndChild();
    }*/
}
void ChildWnd_ThreadWindow(bool* show,uint32_t pid)
{
    static std::vector<ThreadItem> items{};
    static int selected = -1;
    if (*show)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = { 0.f,0.f };
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::Begin(u8"线程列表", show, ImGuiWindowFlags_NoCollapse);
        if (items.size() <= 0)
            ThreadItem::EnumPidThread(pid, items);
        if (ImGui::BeginTable("#threadlist", 4, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY  | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
        {
            ImGui::TableSetupColumn(u8"线程ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, ThreadItem::EInfo::THREAID);
            ImGui::TableSetupColumn(u8"线程入口", ImGuiTableColumnFlags_WidthFixed, 0.0f, ThreadItem::EInfo::STARTADDR);
            ImGui::TableSetupColumn(u8"优先级", ImGuiTableColumnFlags_WidthFixed, 0.0f, ThreadItem::EInfo::PRIORITY);
            ImGui::TableSetupColumn(u8"所属模块", ImGuiTableColumnFlags_WidthFixed, 0.0f, ThreadItem::EInfo::MODULEPATH);
            ImGui::TableHeadersRow();

            if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
            {
                if (sorts_specs->SpecsDirty)
                {
                    ProcessItem::SetSortSpecs(sorts_specs); // Store in variable accessible by the sort function.
                    if (items.size() > 1)
                        qsort(&items[0], (size_t)items.size(), sizeof(items[0]), ProcessItem::CompareWithSortSpecs);
                    ProcessItem::SetSortSpecs(NULL);
                    sorts_specs->SpecsDirty = false;
                }
            }

            ImGuiListClipper clipper;
            clipper.Begin(items.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    ThreadItem* item = &items[row_n];
                    ImGui::PushID(item->GetThreadId());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(std::to_string(item->GetThreadId()).c_str(), selected == row_n, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        selected = row_n;
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::conver::IntegerTohex(item->GetThreadEntry()).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", item->GetPrority());
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::conver::string_To_UTF8(item->GetModulePath()).c_str());
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
    else
    {
        selected = -1;
        items.clear();
    }
}
void ChildWnd_ExceptionWindow(bool* show, uint32_t pid)
{
    static std::vector<VehHandlerItem> items{};
    static int selected = -1;
    if (*show)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = { 0.f,0.f };
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::Begin(u8"异常回调列表", show, ImGuiWindowFlags_NoCollapse);
        if (items.size() <= 0)
            VehHandlerItem::EnumVehHandler(pid, items);

        if (ImGui::BeginTable("#vehhandlerlist", 3, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
        {
            ImGui::TableSetupColumn(u8"线程入口", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, VehHandlerItem::EInfo::ENTRY);
            ImGui::TableSetupColumn(u8"类型", ImGuiTableColumnFlags_WidthFixed, 0.0f, VehHandlerItem::EInfo::TYPE);
            ImGui::TableSetupColumn(u8"模块路径", ImGuiTableColumnFlags_WidthFixed, 0.0f, VehHandlerItem::EInfo::MODULEPATH);
            ImGui::TableHeadersRow();

            if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
            {
                if (sorts_specs->SpecsDirty)
                {
                    ProcessItem::SetSortSpecs(sorts_specs); // Store in variable accessible by the sort function.
                    if (items.size() > 1)
                        qsort(&items[0], (size_t)items.size(), sizeof(items[0]), ProcessItem::CompareWithSortSpecs);
                    ProcessItem::SetSortSpecs(NULL);
                    sorts_specs->SpecsDirty = false;
                }
            }

            ImGuiListClipper clipper;
            clipper.Begin(items.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    VehHandlerItem* item = &items[row_n];
                    ImGui::PushID(item->GetEntry());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(utils::conver::IntegerTohex(item->GetEntry()).c_str(), selected == row_n, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        selected = row_n;
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text(item->IsVeh()?"VEH Exception":"VEH Continue");
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::conver::string_To_UTF8(item->GetModulePath()).c_str());
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
    else
    {
        selected = -1;
        items.clear();
    }
}
void ChildWnd_MemoryWindow(bool* show, uint32_t pid)
{
    static std::vector<MemoryItem> items{};
    static int selected = -1;
    if (*show)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = { 0.f,0.f };
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::Begin(u8"内存列表", show, ImGuiWindowFlags_NoCollapse);
        if (items.size() <= 0)
            MemoryItem::EnumPidMemoryBlocks(pid, items);

        if (ImGui::BeginTable("#memorylist", 5,ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
        {
            ImGui::TableSetupColumn(u8"地址", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, MemoryItem::EInfo::BASE);
            ImGui::TableSetupColumn(u8"大小", ImGuiTableColumnFlags_WidthFixed, 0.0f, MemoryItem::EInfo::SIZE);
            ImGui::TableSetupColumn(u8"Protect", ImGuiTableColumnFlags_WidthFixed, 0.0f, MemoryItem::EInfo::PROTECT);
            ImGui::TableSetupColumn(u8"State", ImGuiTableColumnFlags_WidthFixed, 0.0f, MemoryItem::EInfo::STATE);
            ImGui::TableSetupColumn(u8"Type", ImGuiTableColumnFlags_WidthFixed, 0.0f, MemoryItem::EInfo::TYPE);
            ImGui::TableHeadersRow();

            if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
            {
                if (sorts_specs->SpecsDirty)
                {
                    MemoryItem::SetSortSpecs(sorts_specs); // Store in variable accessible by the sort function.
                    if (items.size() > 1)
                        qsort(&items[0], (size_t)items.size(), sizeof(items[0]), MemoryItem::CompareWithSortSpecs);
                    MemoryItem::SetSortSpecs(NULL);
                    sorts_specs->SpecsDirty = false;
                }
            }

            ImGuiListClipper clipper;
            clipper.Begin(items.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    MemoryItem* item = &items[row_n];
                    ImGui::PushID(item->GetBase());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(utils::conver::IntegerTohex(item->GetBase()).c_str(), selected == row_n, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        selected = row_n;
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%X", item->GetSize());
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::mem::GetProtectByValue(item->GetProtect()).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::mem::GetStateByValue(item->GetState()).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::mem::GetTypeByValue(item->GetType()).c_str());
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
    else
    {
        selected = -1;
        items.clear();
    }
}
void OnGui(float w,float h)
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = { 2.f,2.f };
    ChildWnd_ThreadWindow(&config::process::thread::bShow, config::process::thread::pid);
    ChildWnd_ExceptionWindow(&config::process::veh::bShow, config::process::veh::pid);
    ChildWnd_MemoryWindow(&config::process::memory::bShow, config::process::memory::pid);
    if (ImGui::Begin("main", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        ImGui::SetWindowPos("main", { 0,0 });
        ImGui::SetWindowSize("main", {w,h});
        ImGuiTheme::ApplyTheme(ImGuiTheme::ImGuiTheme_MaterialFlat);
        if (ImGui::BeginTabBar("Main_Tabls", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem(u8"进程"))
            {
                Table_Process();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(u8"VEH插件"))
            {
                if (ImGui::BeginTabBar("Plugin_Tables", ImGuiTabBarFlags_None))
                {
                    static int choose = -1;
                    static std::vector<std::string> plugin_tables = { u8"系统调用" ,u8"调试器",u8"模拟器"};
                    for (int i = 0;i < plugin_tables.size();i++)
                    {
                        if (ImGui::BeginTabItem(plugin_tables[i].c_str()))
                        {
                            choose = i;
                            ImGui::EndTabItem();
                        }
                    }
                    switch (choose)
                    {
                        case 0: Table_SyscallMonitor();break;
                        case 1: Table_VehDebuger();break;
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(u8"窗口"))
            {
                Table_Window();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}
void OnUpdate(DWORD* a)
{
    while (true)
    {
        if (config::global::injectProcess.GetPid() > 0)
        {
            //Syscall Monotor
            {
                static bool signal_syscallmonitor = false;
                if (config::syscall::active)
                {
                    if (!signal_syscallmonitor)
                    {
                        ControlCmd cmd{};
                        cmd.cmd = ECMD::syscallmonitor_init;
                        cmd.syscall_state = config::syscall::active;
                        if (!InvokePluginFunction(config::global::injectProcess.GetPid(), cmd))
                        {
                            MessageBoxA(NULL, "SyscallMonitor Open Failed!", NULL, NULL);
                            config::syscall::active = false; //clear checked
                        }
                        else {
                            signal_syscallmonitor = true; // make sure do once
                        }
                    }
                }
                else if (!config::syscall::active)
                {
                    if (signal_syscallmonitor)
                    {
                        ControlCmd cmd{};
                        cmd.cmd = ECMD::syscallmonitor_init;
                        cmd.syscall_state = config::syscall::active;
                        if (!InvokePluginFunction(config::global::injectProcess.GetPid(), cmd))
                        {
                            MessageBoxA(NULL, "SyscallMonitor Open Failed!", NULL, NULL);
                            config::syscall::active = true; //clear check
                        }
                        else {
                            signal_syscallmonitor = false;// make sure do once
                        }
                    }
                    signal_syscallmonitor = false; //alway set true while syscall-monotor is no-active
                }
            }

            //Veh Debugger
            {
                //std::pair<Enable,Add>
                static std::map<uint8_t, std::pair<bool, bool>> signal_dr{};
                for (int i = 0;i < 4;i++)
                {
                    auto& tmp_dr = config::dbg::Dr[i];

                    // this is button option
                    if (tmp_dr.statue == 1) //already add
                    {
                        //is first-time add
                        if (signal_dr.count(i) <= 0)
                        {
                            //default set false,false
                            signal_dr[i] = { false,false };
                        }

                        if (!signal_dr[i].second)
                        {
                            ControlCmd cmd{};
                            cmd.cmd = ECMD::veh_set_dr;
                            cmd.dr_index = i;
                            cmd.hardbread.addr = strlen(tmp_dr.addr) > 0 ? utils::conver::hexToInteger(tmp_dr.addr) : 0;
                            cmd.hardbread.size = tmp_dr.size;
                            cmd.hardbread.type = tmp_dr.type;
                            if (cmd.hardbread.addr <= 0 || !InvokePluginFunction(config::global::injectProcess.GetPid(), cmd))
                            {
                                MessageBoxA(NULL, "Add BreakPoint Failed!", NULL, NULL);
                                tmp_dr.statue = 0; //add fault,so set button title to "Add"
                            }
                            else
                            {
                                signal_dr[i].second = true; //make sure do once
                            }
                        }
                    }
                    else
                    {
                        //remove,make sure do once!!!
                        if (signal_dr.count(i) && signal_dr[i].second)
                        {
                            ControlCmd cmd{};
                            cmd.cmd = ECMD::veh_unset_dr;
                            cmd.dr_index = i;
                            if (!InvokePluginFunction(config::global::injectProcess.GetPid(), cmd))
                            {
                                MessageBoxA(NULL, "Remove BreakPoint Failed!", NULL, NULL);
                                tmp_dr.statue = 1; //remove fault,set button title to "Remove"
                            }
                            else
                            {
                                //clear data
                                Debugger::GetDbgInfo(i).capture.clear();
                                signal_dr[i].second = false; //make sure do once
                                tmp_dr.active = false;
                            }
                        }
                    }

                    // this is checkbox option
                    if (tmp_dr.active) //enable
                    {
                        //is recored,make sure do once!!!
                        if (!signal_dr[i].first && signal_dr[i].second)
                        {
                            ControlCmd cmd{};
                            cmd.cmd = ECMD::veh_enable_dr;
                            cmd.dr_index = i;
                            if (!InvokePluginFunction(config::global::injectProcess.GetPid(), cmd))
                            {
                                MessageBoxA(NULL, "Enable BreakPoint Failed!", NULL, NULL);
                                tmp_dr.active = 0;
                            }
                            else
                            {
                                signal_dr[i].first = true;
                            }
                        }

                    }
                    else
                    {
                        //remove,make sure do once!!!
                        if (signal_dr.count(i) && signal_dr[i].first)
                        {
                            ControlCmd cmd{};
                            cmd.cmd = ECMD::veh_disable_dr;
                            cmd.dr_index = i;
                            if (!InvokePluginFunction(config::global::injectProcess.GetPid(), cmd))
                            {
                                MessageBoxA(NULL, "Disable BreakPoint Failed!", NULL, NULL);
                                tmp_dr.active = 1;
                            }
                            else
                            {
                                signal_dr[i].first = false;
                            }
                        }
                    }
                }
            }
        }
        Sleep(500);
    }
}
void OnIPC(DWORD* a)
{
    char* buff = new char[8192];
    while (true)
    {
        if (config::global::plugin_.read_pipe(buff))
        {
            EDataType type = (EDataType)(*(uint8_t*)buff);
            switch (type)
            {
                case API:
                {
                    PApiMonitorInfo pApi = reinterpret_cast<PApiMonitorInfo>(buff);
                    if (config::syscall::monitor.count(pApi->modulename) <= 0)
                    {
                        config::syscall::monitor[pApi->modulename][pApi->function] = 1;
                    }
                    else
                    {
                        config::syscall::monitor[pApi->modulename][pApi->function]++;
                    }
                    break;
                } 
                case DEBG:
                {
                    Debugger::PDbgBreakInfo pDbg = reinterpret_cast<Debugger::PDbgBreakInfo>(buff);
                    auto& curtData = Debugger::GetDbgInfoRef(pDbg->id);
                    //如果为执行断点，关闭启用选项
                    if (config::dbg::Dr[pDbg->id].type == 0)
                        config::dbg::Dr[pDbg->id].statue = 0;

                    if (curtData.capture.count(pDbg->ctx.Rip) <= 0)
                    {
                        /*MEMORY_BASIC_INFORMATION meminfo{};
                        Mem::QueryMem(config::global::targetProcess.GetPid(),(PVOID)pDbg->ctx.Rip,&meminfo,sizeof(MEMORY_BASIC_INFORMATION));
                        curtData.memoryRegion.start = (uint64_t)meminfo.AllocationBase;
                        curtData.memoryRegion.data.resize(meminfo.RegionSize);
                       */
                        //generate region disassembly
                        uint8_t buf[100]{};
                        Mem::ReadMemory(config::global::targetProcess.GetPid(), pDbg->ctx.Rip-0x50, buf, 0x100);
                        auto disam = Debugger::Disassembly(!config::global::targetProcess.IsWow64(), pDbg->ctx.Rip - 0x50, buf, sizeof(buf));
                        for (int i = 0; i < disam.size(); i++)
                        {
                            if (disam[i][0] == utils::conver::IntegerTohex(pDbg->ctx.Rip))
                            {
                                if (config::dbg::Dr[pDbg->id].type == 0)
                                {
                                    strcpy_s(pDbg->disassembly, disam[i ][2].c_str());
                                    curtData.capture[pDbg->ctx.Rip] = { *pDbg ,disam[i][0],1 };
                                }
                                else
                                {
                                    //find breakpoint,because the dr-break is interrupt at next line
                                    strcpy_s(pDbg->disassembly, disam[i - 1][2].c_str());
                                    //fix rip to real-rip
                                    curtData.capture[pDbg->ctx.Rip] = { *pDbg ,disam[i - 1][0],1 };
                                }
                                memcpy(curtData.capture[pDbg->ctx.Rip].stack, pDbg->stack, sizeof(pDbg->stack));
                                break;
                            }
                        }
                    }
                    else
                    {
                        //update count、stack
                        auto& t = curtData.capture[pDbg->ctx.Rip];
                        memcpy(t.stack, pDbg->stack,sizeof(t.dbginfo.stack));
                        t.count++;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        
    }
    delete[] buff;
}


int main()
{
    //检查目录是否存在
    std::filesystem::path data_dir("./Data");
    if(!std::filesystem::exists(data_dir))
        std::filesystem::create_directory(data_dir);

    //初始化pdb
    char nt_path[MAX_PATH]{};
    GetModuleFileNameA(GetModuleHandleA("ntdll.dll"), nt_path, MAX_PATH);
    std::string pdbPath = EasyPdb::EzPdbDownload(nt_path);
    if (pdbPath.empty())
    {
        MessageBoxA(NULL,"Download Pdb Error!","PJArk",NULL);
        return 1;
    }
    EasyPdb::EZPDB pdb;
    if (!EasyPdb::EzPdbLoad(pdbPath, &pdb))
    {
        MessageBoxA(NULL, "Load Pdb Failed!", "PJArk", NULL);
        return 1;
    }
    auto rva_LdrpVectorHandlerList = EzPdbGetRva(&pdb, "LdrpVectorHandlerList");
    if (rva_LdrpVectorHandlerList <= 0)
    {
        MessageBoxA(NULL, "Get rva_LdrpVectorHandlerList Failed!", "PJArk", NULL);
        return 1;
    }
    VehHandlerItem::SetLdrpVectorHandlerList(rva_LdrpVectorHandlerList);
    EasyPdb::EzPdbUnload(&pdb);

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)OnIPC, NULL, NULL, NULL);
    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)OnUpdate, NULL, NULL, NULL);
    render::get_instasnce()->CreatGui(L"PJArk", L"CPJArk", 1440, 900, OnGui);
    //WindowItem::EnumAllWindows();
    return 0;
}
