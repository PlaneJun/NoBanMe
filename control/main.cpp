#include "pch.h"


std::vector<std::vector<std::string>> Disassembly(uint64_t start,size_t size, const uint8_t* buffer)
{
    std::vector<std::vector<std::string>> ret;
    std::vector<std::string> tmp;
    ZydisDisassembledInstruction instruction;
    ZyanU64 runtime_address = (ZyanU64)start;
    for (int i = 0; i < size; )
    {
        tmp.clear();
        if (ZYAN_SUCCESS(ZydisDisassembleIntel(
            /* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
            /* runtime_address: */ runtime_address,
            /* buffer:          */ buffer + i,
            /* length:          */ size - i,
            /* instruction:     */ &instruction
        )))
        {
            tmp.push_back("0x" + utils::IntegerTohex(runtime_address));
            tmp.push_back(utils::bytesToHexString(buffer + i, instruction.info.length).c_str());
            tmp.push_back(instruction.text);
            ret.push_back(tmp);
            i += instruction.info.length;
            runtime_address += instruction.info.length;
        }
        else
        {
            tmp.push_back("0x" + utils::IntegerTohex(runtime_address));
            tmp.push_back(utils::bytesToHexString(buffer + i, 1).c_str());
            tmp.push_back("db");
            ret.push_back(tmp);
            i++;
            runtime_address += i;
        }
    }

    return ret;
}

void Render_Process()
{
    static std::vector<data::process::ProcessItem> pitems;
    static std::vector<data::process::ModuleItem> mitems;
    if (ImGui::Button(u8"刷新进程"))
        pitems.clear();

    //Enum Process
    if (pitems.size() == 0)
    {
        utils::EnumCurtAllProcess(pitems);
    }

    static int selected_process = -1;
    static int selected_processid = 0;
    if (ImGui::BeginChild("#process", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.7), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::SeparatorText(u8"进程列表");
        if (ImGui::BeginTable("#processlist", 6, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
        {
            ImGui::TableSetupColumn("##icon", ImGuiTableColumnFlags_NoSort, 0.0f);
            ImGui::TableSetupColumn(u8"进程ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, data::process::processinfo::PID);
            ImGui::TableSetupColumn(u8"进程名", ImGuiTableColumnFlags_WidthFixed, 0.0f, data::process::processinfo::NAME);
            ImGui::TableSetupColumn(u8"父进程ID", ImGuiTableColumnFlags_WidthFixed, 0.0f, data::process::processinfo::PPID);
            ImGui::TableSetupColumn(u8"进程路径", ImGuiTableColumnFlags_WidthFixed, 0.0f, data::process::processinfo::FULLPATH);
            ImGui::TableSetupColumn(u8"启动时间", ImGuiTableColumnFlags_WidthFixed, 0.0f, data::process::processinfo::STARTUPTIME);
            ImGui::TableHeadersRow();

            if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
            {
                if (sorts_specs->SpecsDirty)
                {
                    data::process::ProcessItem::s_current_sort_specs_process = sorts_specs; // Store in variable accessible by the sort function.
                    if (pitems.size() > 1)
                        qsort(&pitems[0], (size_t)pitems.size(), sizeof(pitems[0]), data::process::ProcessItem::CompareWithSortSpecs);
                    data::process::ProcessItem::s_current_sort_specs_process = NULL;
                    sorts_specs->SpecsDirty = false;
                }
            }

            ImGuiListClipper clipper;
            clipper.Begin(pitems.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    data::process::ProcessItem* item = &pitems[row_n];
                    ImGui::PushID(item->pid);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (item->icon)
                        ImGui::Image(item->icon, ImVec2(16, 16));
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(std::to_string(item->pid).c_str(), selected_process == row_n, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        selected_process = row_n;
                        if (selected_processid != item->pid)
                        {
                            selected_processid = item->pid;
                            mitems.clear();
                        }
                    }
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::string_To_UTF8(item->name).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", item->ppid);
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::string_To_UTF8(item->fullpath).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text(utils::string_To_UTF8(item->startuptime).c_str());
                    ImGui::PopID();
                }
            }

            if (selected_process != -1 &&
                ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(1))
            {
                ImGui::OpenPopup("process_option");
            }
            if (ImGui::BeginPopup("process_option"))
            {
                static const char* proc_info[] = {u8"枚举窗口",u8"线程列表",u8"进程句柄",u8"异常回调"};
                for (int i = 0; i < IM_ARRAYSIZE(proc_info); i++)
                {
                    if (ImGui::Selectable(proc_info[i]))
                    {
                        if (i == 1)
                        {
                            config::process::thread::bShow = true;
                            config::process::thread::pid = selected_processid;
                           
                        }
                    }
                }
                ImGui::Separator();
                static const char* proc_option[] = { u8"结束进程",u8"隐藏进程",u8"进程属性",u8"文件定位" };
                for (int i = 0; i < IM_ARRAYSIZE(proc_option); i++)
                {
                    if (ImGui::Selectable(proc_option[i]))
                    {

                    }
                }
                ImGui::Separator();
                static const char* proc_file[] = { u8"转储内存",u8"拷贝文件",u8"文件定位" };
                for (int i = 0; i < IM_ARRAYSIZE(proc_file); i++)
                {
                    if (ImGui::Selectable(proc_file[i]))
                    {

                    }
                }
                ImGui::Separator();
                static const char* proc_hack[] = { u8"注入Dll",u8"监控Syscall",u8"硬件调试" };
                for (int i = 0; i < IM_ARRAYSIZE(proc_hack); i++)
                {
                    if (ImGui::Selectable(proc_hack[i]))
                    {
                        if (i == 2)
                        {
                            char caption[256]{};
                            sprintf(caption, "Are you sure dbg target:%d ?", selected_processid);
                            if (MessageBoxA(NULL, caption, NULL, MB_OKCANCEL) == IDOK)
                            {
                                if (!data::global::inject)
                                {
                                    if (utils::InjectDLL(selected_processid, data::global::plugin_path))
                                    {
                                        data::global::target = selected_processid;
                                        data::global::fnDispatch += (uint64_t)utils::GetProcessModuleHandle(selected_processid, L"pjwatch.dll");
                                        data::global::inject = true;
                                    }
                                }

                                //打开管道
                                ControlCmd cmd{};
                                cmd.cmd = ECMD::Pipe;
                                if (!utils::CallFunction(cmd))
                                    MessageBoxA(NULL, "Create Pipe Failed!", NULL, NULL);


                                //初始化调试
                                auto tid = utils::GetMainThreadId(selected_processid);
                                if (tid <= 0)
                                {
                                    MessageBoxA(NULL, "init dbg envir failed", NULL, NULL);
                                }
                                else
                                {
                                    ControlCmd cmd{};
                                    cmd.cmd = ECMD::InitDbg;
                                    cmd.threadid = tid;
                                    if (!utils::CallFunction(cmd))
                                        MessageBoxA(NULL, "Init Dbg Failed!", NULL, NULL);
                                    else
                                        MessageBoxA(NULL, "done!", NULL, NULL);
                                }
                            }
                        }
                    }
                }
                ImGui::Separator();
                static const char* proc_copy[] = { u8"进程ID",u8"进程名",u8"父进程ID",u8"进程路径" ,u8"启动时间" };
                if (ImGui::BeginMenu(u8"复制"))
                {
                    for (int i = 0; i < IM_ARRAYSIZE(proc_copy); i++)
                    {
                        if (ImGui::Selectable(proc_copy[i]))
                        {

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
        if (selected_processid != 0)
        {
            //Enum Modules
            if (mitems.size() == 0)
            {
                utils::EnumPidModules(selected_processid, mitems);
            }
        }

        ImGui::SeparatorText(u8"模块列表");
        static int selected_module = -1;
        if (ImGui::BeginTable("#modulelist", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
        {
            ImGui::TableSetupColumn(u8"模块路径", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, data::process::moduleinfo::IMAGEPATH);
            ImGui::TableSetupColumn(u8"基地址", ImGuiTableColumnFlags_WidthFixed, 0.0f, data::process::moduleinfo::BASE);
            ImGui::TableSetupColumn(u8"大小", ImGuiTableColumnFlags_WidthFixed, 0.0f, data::process::moduleinfo::SIZE);
            ImGui::TableHeadersRow();

            if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
            {
                if (sorts_specs->SpecsDirty)
                {
                    data::process::ModuleItem::s_current_sort_specs_module = sorts_specs; // Store in variable accessible by the sort function.
                    if (mitems.size() > 1)
                        qsort(&mitems[0], (size_t)mitems.size(), sizeof(mitems[0]), data::process::ModuleItem::CompareWithSortSpecs);
                    data::process::ModuleItem::s_current_sort_specs_module = NULL;
                    sorts_specs->SpecsDirty = false;
                }
            }
            ImGuiListClipper clipper;
            clipper.Begin(mitems.size());
            while (clipper.Step())
            {
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    data::process::ModuleItem* item = &mitems[row_n];
                    ImGui::PushID(item->base);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(utils::string_To_UTF8(item->imagepath).c_str(), selected_module == row_n, ImGuiSelectableFlags_SpanAllColumns))
                        selected_module = row_n;
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%p", item->base);
                    ImGui::TableNextColumn();
                    ImGui::Text("0x%llx", item->size);
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}
void Render_SyscallMonitor()
{
    ImGui::SeparatorText(u8"选项");
    ImGui::Checkbox(u8"启用", &config::Syscall::active); ImGui::SameLine();
    ImGui::Checkbox(u8"保存日志", &config::Syscall::save); ImGui::SameLine();
    ImGui::Button(u8"清除");
    ImGui::SeparatorText(u8"输出");
    if (ImGui::TreeNode("root"))
    {
        for (auto n : data::Syscall::monitor)
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
void Render_VehDebuger()
{
    auto curtData = data::dbg::dbgInfo[config::dbg::curtChoose];
    static std::pair< std::string, bool> JmpToAddress{};
    if (ImGui::CollapsingHeader(u8"硬件断点", ImGuiTreeNodeFlags_DefaultOpen))
    {
        static std::string dr_statue[] = { u8"添加",u8"移除" };
        static const char* dr_size[] = { "1 byte", "2 byte", "4 byte", "8 byte" };
        static const char* dr_type[] = { u8"执行", u8"读/写", u8"写" };
        for (int i = 0;i < 4;i++)
        {
            char check_title[100]{};
            sprintf(check_title, u8"启用     Dr%d:", i);
            if (config::dbg::Dr[i].statue == 0)
                ImGui::BeginDisabled();
            ImGui::Checkbox(check_title, &config::dbg::Dr[i].active); ImGui::SameLine();
            if (config::dbg::Dr[i].statue == 0)
                ImGui::EndDisabled();

            char input_title[100]{};
            sprintf(input_title, "##input_dr%d", i);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3);
            ImGui::InputTextWithHint(input_title, "adrress", config::dbg::Dr[i].addr, 1024); ImGui::SameLine();
            ImGui::PopItemWidth();

            ImGui::Text(u8"类型:");ImGui::SameLine();
            char combo_type_title[100]{};
            sprintf(combo_type_title, "##combot_type_dr%d", i);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.2);
            ImGui::Combo(combo_type_title, &config::dbg::Dr[i].type, dr_type, IM_ARRAYSIZE(dr_type));ImGui::SameLine();
            ImGui::PopItemWidth();

            ImGui::Text(u8"大小:");ImGui::SameLine();
            char combo_size_title[100]{};
            sprintf(combo_size_title, "##combot_size_dr%d", i);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.2);
            ImGui::Combo(combo_size_title, &config::dbg::Dr[i].size, dr_size, IM_ARRAYSIZE(dr_size));ImGui::SameLine();
            ImGui::PopItemWidth();

            char button_id[100]{};
            sprintf(button_id, "btn_dr%d", i);
            ImGui::PushID(button_id);
            if (ImGui::Button(dr_statue[config::dbg::Dr[i].statue].c_str()))
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
                sprintf(buf, "Dr%d", i);
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
                        curtData.ctx = ii.second.dbginfo.ctx;
                        curtData.disassembly = ii.second.disamSeg;
                        if (JmpToAddress.first != ii.second.text)
                        {
                            JmpToAddress.first = ii.second.text; //设置要跳转的地址
                            JmpToAddress.second = false;
                        }
                        break;
                    }
                    i++;
                }
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (ImGui::BeginChild("ChildDisassembly", ImVec2(ImGui::GetContentRegionAvail().x * 0.66, ImGui::GetContentRegionAvail().y * 0.66)))
        {
            ImGui::SeparatorText(u8"反汇编");
            ImGui::Columns(3, "##disassembly");
            ImGui::Text(u8"地址"); ImGui::NextColumn();
            ImGui::Text(u8"字节码"); ImGui::NextColumn();
            ImGui::Text(u8"指令"); ImGui::NextColumn();
            ImGui::Separator();
            static int selected = -1;
            for (int i = 0; i < curtData.disassembly.size(); i++)
            {
                if (ImGui::Selectable(curtData.disassembly[i][0].c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns))
                    selected = i;
                ImGui::NextColumn();
                for (int j = 1; j < curtData.disassembly[i].size(); j++)
                {
                    ImGui::Text(curtData.disassembly[i][j].c_str()); ImGui::NextColumn();
                }

                //如果到了指定行就停止滚动
                if (JmpToAddress.first == curtData.disassembly[i][0])
                {
                    if (!JmpToAddress.second)
                        selected = i; //只在跳转的时候选择一次
                    JmpToAddress.second = true;
                }
                if (!JmpToAddress.second)
                {
                    ImGui::SetScrollHereY(0);
                }
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();
        if (ImGui::BeginChild("ChildContext", ImVec2(ImGui::GetContentRegionAvail().x * 0.98, ImGui::GetContentRegionAvail().y * 0.66)))
        {
            ImGui::SeparatorText(u8"寄存器");
            render::get_instasnce()->HelpMarker(u8"当前显示的上下文为断点执行后数据\n");
            static int selected = -1;
            std::vector<std::string> headers = { u8"Reg" ,u8"Value" };
            std::vector<std::pair<std::string, std::string>> text = {
                {"RAX","0x" + utils::IntegerTohex(curtData.ctx.Rax)},
                {"RBX","0x" + utils::IntegerTohex(curtData.ctx.Rbx)},
                {"RCX","0x" + utils::IntegerTohex(curtData.ctx.Rcx) },
                {"RDX","0x" + utils::IntegerTohex(curtData.ctx.Rdx)},
                {"RBP","0x" + utils::IntegerTohex(curtData.ctx.Rbp)},
                {"RSP","0x" + utils::IntegerTohex(curtData.ctx.Rsp)},
                {"RSI","0x" + utils::IntegerTohex(curtData.ctx.Rsi)},
                {"RDI","0x" + utils::IntegerTohex(curtData.ctx.Rdi)},
                { "R8" ,"0x" + utils::IntegerTohex(curtData.ctx.R8) },
                { "R9","0x" + utils::IntegerTohex(curtData.ctx.R9) },
                { "R10","0x" + utils::IntegerTohex(curtData.ctx.R10) },
                { "R11","0x" + utils::IntegerTohex(curtData.ctx.R11) },
                { "R12","0x" + utils::IntegerTohex(curtData.ctx.R12) },
                { "R13" ,"0x" + utils::IntegerTohex(curtData.ctx.R13)},
                { "R14","0x" + utils::IntegerTohex(curtData.ctx.R14) },
                { "R15","0x" + utils::IntegerTohex(curtData.ctx.R15) },
                { "RIP" ,"0x" + utils::IntegerTohex(curtData.ctx.Rip) },
                { "RFLAGS" ,"0x" + utils::IntegerTohex(curtData.ctx.EFlags) },
                {"Xmm0","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm0)},
                {"Xmm1","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm1)},
                {"Xmm2","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm2)},
                {"Xmm3","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm3)},
                {"Xmm4","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm4)},
                {"Xmm5","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm5)},
                {"Xmm6","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm6)},
                {"Xmm7","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm7)},
                {"Xmm8","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm8)},
                {"Xmm9","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm9)},
                {"Xmm10","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm10)},
                {"Xmm11","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm11)},
                {"Xmm12","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm12)},
                {"Xmm13","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm13)},
                {"Xmm14","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm14)},
                {"Xmm15","0x" + utils::IntegerTohex(*(uint64_t*)&curtData.ctx.Xmm15)}
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
            ImGui::SeparatorText(u8"堆栈");
            static int selected = -1;
            std::vector<std::string> headers = { "Rsp" ,"Value" };
            std::vector<std::pair<std::string, std::string>> text = {

            };
            render::get_instasnce()->AddListBox("##Stack", selected, headers, text);
            ImGui::EndChild();
        }
    }
}
void Render_Thread(bool* show,uint32_t pid)
{
    static std::vector<data::process::ThreadItem> items{};
    static int selected = -1;
    static const std::vector<std::string> headers = { u8"线程ID",u8"线程入口" ,u8"优先级" };
    if (*show)
    {
        char title[256]{};
        if (ImGui::Begin("Thread", show,ImGuiWindowFlags_NoCollapse))
        {

            if (items.size() <= 0)
                utils::EnumPidThread(pid, items);

            std::vector<std::vector<std::string>> text{};
            for (auto i : items)
            {
                std::vector<std::string> tmp{};
                tmp.push_back(std::to_string(i.threadid));
                tmp.push_back("0x" + utils::IntegerTohex(i.startAddr));
                tmp.push_back(std::to_string(i.DeltaPri));
                text.push_back(tmp);
            }
            render::get_instasnce()->AddListBox("##thread_list", selected, 1, headers, text);
            ImGui::End();
        }
    }
    else
    {
        selected = -1;
        items.clear();
    }
}
void OnGui(uint32_t w, uint32_t h)
{
    if (ImGui::Begin("main", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.TabRounding = 0.f;
        ImGui::SetWindowPos("main", { 0,0 });
        ImGui::SetWindowSize("main", ImVec2(w, h));
        ImGuiTheme::ApplyTheme(ImGuiTheme::ImGuiTheme_MaterialFlat);
        if (ImGui::BeginTabBar("Main_Tabls", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem(u8"进程"))
            {
                Render_Process();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(u8"系统调用"))
            {
                Render_SyscallMonitor();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(u8"VEH调试器"))
            {
                Render_VehDebuger();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        
        ImGui::End();
    }

    Render_Thread(&config::process::thread::bShow, config::process::thread::pid);
}

void OnUpdate()
{
    while (true)
    {
        //Syscall Monotor
        {
            static bool signal_syscallmonitor = false;
            if (config::Syscall::active)
            {
                if (!signal_syscallmonitor)
                {
                    ControlCmd cmd{};
                    cmd.cmd = SyscallMonitor;
                    cmd.syscall_state = config::Syscall::active;
                    if (!utils::CallFunction(cmd))
                    {
                        MessageBoxA(NULL, "SyscallMonitor Open Failed!", NULL, NULL);
                        config::Syscall::active = false; //clear checked
                    }
                    else {
                        signal_syscallmonitor = true; // make sure do once
                    }
                }
            }
            else if (!config::Syscall::active)
            {
                if (signal_syscallmonitor)
                {
                    ControlCmd cmd{};
                    cmd.cmd = SyscallMonitor;
                    cmd.syscall_state = config::Syscall::active;
                    if (!utils::CallFunction(cmd))
                    {
                        MessageBoxA(NULL, "SyscallMonitor Open Failed!", NULL, NULL);
                        config::Syscall::active = true; //clear check
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
                        cmd.cmd = SetDrBreak;
                        cmd.dr_index = i;
                        cmd.hardbread.addr = utils::hexToInteger(tmp_dr.addr);
                        cmd.hardbread.size = tmp_dr.size;
                        cmd.hardbread.type = tmp_dr.type;
                        if (cmd.hardbread.addr<=0 || !utils::CallFunction(cmd))
                        {
                            MessageBoxA(NULL, "Add BreakPoint Failed!", NULL, NULL);
                            tmp_dr.statue = 0; //add fault,so set button title to "Add"
                        }
                        else
                        {
                            signal_dr[i].second=true; //make sure do once
                        }
                    }
                }
                else
                {
                    //remove,make sure do once!!!
                    if (signal_dr.count(i) && signal_dr[i].second)
                    {
                        ControlCmd cmd{};
                        cmd.cmd = UnSetDrBreak;
                        cmd.dr_index = i;
                        if (!utils::CallFunction(cmd))
                        {
                            MessageBoxA(NULL, "Remove BreakPoint Failed!", NULL, NULL);
                            tmp_dr.statue = 1; //remove fault,set button title to "Remove"
                        }
                        else
                        {
                            //clear data
                            data::dbg::dbgInfo[i].capture.clear();
                            signal_dr[i].second=false; //make sure do once
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
                        cmd.cmd = EnableDrBreak;
                        cmd.dr_index = i;
                        if (!utils::CallFunction(cmd))
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
                        cmd.cmd = DisableDrBreak;
                        cmd.dr_index = i;
                        if (!utils::CallFunction(cmd))
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
        Sleep(500);
    }
}

void OnIPC()
{
    char* buff = new char[8192];
    while (true)
    {
        if (data::global::pipe_.read(buff))
        {
            EDataType type = (EDataType)(*(uint8_t*)buff);
            switch (type)
            {
                case API:
                {
                    PApiMonitorInfo pApi = reinterpret_cast<PApiMonitorInfo>(buff);
                    if (data::Syscall::monitor.count(pApi->modulename) <= 0)
                    {
                        data::Syscall::monitor[pApi->modulename][pApi->function] = 1;
                    }
                    else
                    {
                        data::Syscall::monitor[pApi->modulename][pApi->function]++;
                    }
                    break;
                } 
                case DEBG:
                {
                    PDbgBreakInfo pDbg = reinterpret_cast<PDbgBreakInfo>(buff);
                    auto& curtData = data::dbg::dbgInfo[pDbg->id];
                    if (curtData.capture.count(pDbg->ctx.Rip) <= 0)
                    {
                        uint32_t aglim = (pDbg->region_size & 0xfffff000) + 0x1000;
                        curtData.memoryRegion.data.resize(aglim);
                        //read mem
                        auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, data::global::target);
                        if (hProc)
                        {
                            ReadProcessMemory(hProc, (PVOID)pDbg->region_start, &curtData.memoryRegion.data[0], aglim, NULL);
                            CloseHandle(hProc);
                        }
                        //generate region disassembly
                        auto disam = Disassembly(pDbg->region_start, aglim, curtData.memoryRegion.data.data());
                        for (int i = 0; i < disam.size(); i++)
                        {
                            //find breakpoint,because the dr-break is interrupt at next line
                            if (disam[i][0] == "0x" + utils::IntegerTohex(pDbg->ctx.Rip))
                            {
                                strcpy(pDbg->disassembly, disam[i - 1][2].c_str());
                                //fix rip to real-rip
                                curtData.capture[pDbg->ctx.Rip] = { *pDbg ,disam[i - 1][0],1 ,disam };
                                break;
                            }
                        }
                    }
                    else
                    {
                        //only update count
                        auto& t = curtData.capture[pDbg->ctx.Rip];
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

    ULONG_PTR m_gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
    //calc pjwatch.Dispatch offset、path
    auto base = LoadLibraryA("pjwatch.dll");
    auto lpDispatch = reinterpret_cast<uint8_t*>(GetProcAddress(base, "Dispatch"));
    data::global::fnDispatch = (uint64_t)(lpDispatch - (uint8_t*)base);
    GetModuleFileNameA(base,data::global::plugin_path,256);
    FreeLibrary(base);

    //init pipe
    if (!data::global::pipe_.initPipe())
    {
        MessageBoxA(NULL,"create pipe failed!","pjark",NULL);
        return 0;
    }
    std::thread(OnIPC).detach();
    std::thread(OnUpdate).detach();
    render::get_instasnce()->CreatGui(L"PJArk",L"CPJArk",1440,900, OnGui);
    return 0;
}
