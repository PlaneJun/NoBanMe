#include "pch.h"


ProcessWidget processWidget;
ModuleWidget moduleWidget;
SysCallMonitorWidget syscallMonitorWidget;
VehDebuggerWidget vehDebuggerWidget;



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
                static ProcessItem lastChooseProcess{};
                if (ImGui::BeginChild("#process", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.7), false, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    processWidget.OnPaint();
                    ImGui::EndChild();
                }
                if (ImGui::BeginChild("#module", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    //设置数据源
                    if (processWidget.GetCurtProcessItem().GetPid() != lastChooseProcess.GetPid())
                    {
                        moduleWidget.SetDataSource(processWidget.GetCurtProcessItem().GetPid());
                        lastChooseProcess = processWidget.GetCurtProcessItem();
                    }
                    moduleWidget.OnPaint();
                    ImGui::EndChild();
                }
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
                        case 0:    
                        {
                            syscallMonitorWidget.OnPaint();
                            break;
                        }
                        case 1:     
                        {
                            vehDebuggerWidget.OnPaint();
                            break;
                        }
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(u8"窗口"))
            {
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
        if (processWidget.GetPluginProcessItem().GetPid() > 0)
        {
            //Syscall Monotor
            {
                static bool signal_syscallmonitor = false;
                if (syscallMonitorWidget.GetActive())
                {
                    if (!signal_syscallmonitor)
                    {
                        ControlCmd cmd{};
                        cmd.cmd = ECMD::syscallmonitor_init;
                        cmd.syscall_state = syscallMonitorWidget.GetActive();
                        if (!utils::mem::InvokePluginFunction(processWidget.GetPluginProcessItem().GetPid(), cmd))
                        {
                            MessageBoxA(NULL, "SyscallMonitor Open Failed!", NULL, NULL);
                            syscallMonitorWidget.SetActive(false); //clear checked
                        }
                        else {
                            signal_syscallmonitor = true; // make sure do once
                        }
                    }
                }
                else if (!syscallMonitorWidget.GetActive())
                {
                    if (signal_syscallmonitor)
                    {
                        ControlCmd cmd{};
                        cmd.cmd = ECMD::syscallmonitor_init;
                        cmd.syscall_state = syscallMonitorWidget.GetActive();
                        if (!utils::mem::InvokePluginFunction(processWidget.GetPluginProcessItem().GetPid(), cmd))
                        {
                            MessageBoxA(NULL, "SyscallMonitor Open Failed!", NULL, NULL);
                            syscallMonitorWidget.SetActive(true); //clear check
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
                            if (cmd.hardbread.addr <= 0 || !utils::mem::InvokePluginFunction(processWidget.GetPluginProcessItem().GetPid(), cmd))
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
                            if (!utils::mem::InvokePluginFunction(processWidget.GetPluginProcessItem().GetPid(), cmd))
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
                            if (!utils::mem::InvokePluginFunction(processWidget.GetPluginProcessItem().GetPid(), cmd))
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
                            if (!utils::mem::InvokePluginFunction(processWidget.GetPluginProcessItem().GetPid(), cmd))
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
                    if (!syscallMonitorWidget.IsExsist(pApi->modulename))
                    {
                        syscallMonitorWidget.SetValue(pApi->modulename,pApi->function , 1);
                    }
                    else
                    {
                        syscallMonitorWidget.SetValue(pApi->modulename, pApi->function, 1,true);
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
                        curtData.ctx = pDbg->ctx;
                        //generate region disassembly
                        uint8_t buf[100]{};
                        Mem::ReadMemory(processWidget.GetPluginProcessItem().GetPid(), pDbg->ctx.Rip - 0x50, buf, 0x100);
                        auto disam = Debugger::Disassembly(!processWidget.GetPluginProcessItem().IsWow64(), pDbg->ctx.Rip - 0x50, buf, sizeof(buf));
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
                        curtData.ctx = pDbg->ctx;
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
    return 0;
}
