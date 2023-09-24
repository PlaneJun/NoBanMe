#include "pch.h"

DisassemblerWidget disassemblerWidget;
ProcessWidget processWidget;
ModuleWidget moduleWidget;
BreakRecordWidget breakrecordWidget;
ContextWidget contextWidget;
StackWidget stackWidget;

void Table_Process()
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
    static Debugger::DbgCaptureInfo last_choose_capture;
    auto curtData = Debugger::GetDbgInfo(config::dbg::curtChoose);
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
            breakrecordWidget.SetDataSource(curtData.capture);
            breakrecordWidget.OnPaint();
            if (last_choose_capture.text != breakrecordWidget.GetCurtSelect().text)
            {
                contextWidget.SetDataSource(curtData.ctx);
                disassemblerWidget.SetData(processWidget.GetPluginProcessItem(),utils::conver::hexToInteger(breakrecordWidget.GetCurtSelect().text));
                std::vector<uint8_t> stack{};
                stack.resize(sizeof(breakrecordWidget.GetCurtSelect().stack));
                memcpy(&stack[0], breakrecordWidget.GetCurtSelect().stack, sizeof(breakrecordWidget.GetCurtSelect().stack));
                stackWidget.SetDataSource(processWidget.GetPluginProcessItem().GetPid(), stack,curtData.ctx.Rsp);
                last_choose_capture = breakrecordWidget.GetCurtSelect();
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (ImGui::BeginChild("ChildDisassembly", ImVec2(ImGui::GetContentRegionAvail().x * 0.66, ImGui::GetContentRegionAvail().y * 0.66)))
        {
            disassemblerWidget.OnPaint();
            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (ImGui::BeginChild("ChildContext", ImVec2(ImGui::GetContentRegionAvail().x * 0.98, ImGui::GetContentRegionAvail().y * 0.66)))
        {
            
            contextWidget.OnPaint();
            ImGui::EndChild();
        }
        ImGui::Separator();
        if (ImGui::BeginChild("ChildMem", ImVec2(ImGui::GetContentRegionAvail().x * 0.65, ImGui::GetContentRegionAvail().y)))
        {
            /*ImGui::SeparatorText(u8"内存");
            static MemoryEditor mem_edit;
            mem_edit.DrawWindow("Memory Editor", curtData.memoryRegion.data.data(), curtData.memoryRegion.data.size(), curtData.memoryRegion.start);*/
            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (ImGui::BeginChild("ChildStack"))
        {
            stackWidget.OnPaint();
            ImGui::EndChild();
        }
    }
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
                if (config::syscall::active)
                {
                    if (!signal_syscallmonitor)
                    {
                        ControlCmd cmd{};
                        cmd.cmd = ECMD::syscallmonitor_init;
                        cmd.syscall_state = config::syscall::active;
                        if (!utils::mem::InvokePluginFunction(processWidget.GetPluginProcessItem().GetPid(), cmd))
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
                        if (!utils::mem::InvokePluginFunction(processWidget.GetPluginProcessItem().GetPid(), cmd))
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
    //WindowItem::EnumAllWindows();
    return 0;
}
