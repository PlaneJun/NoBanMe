#include "MainWidget.h"
#include <imgui_theme.h>
#include "../3rdParty/imgui/Imgui_text_editor.h"
#include "../module/debugger.h"
#include "../mem/memstub.h"
#include "../utils/utils.h"
#include "../nativeStruct.h"
#include "../global.h"
#include "../IconsFontAwesome5.h"


void MainWidget::OnPaint()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = { 2.f,2.f };
    if (ImGui::Begin("main", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        ImGui::SetWindowPos("main", { 0,0 });
        ImGui::SetWindowSize("main", size_);
        //static ImGuiTheme::ImGuiTweakedTheme theme;
        //ImGuiTheme::ShowThemeTweakGui(&theme);
        //ImGuiTheme::ApplyTweakedTheme(theme);
        //ImGui::StyleColorsLight();
        ImGuiTheme::ApplyTheme(ImGuiTheme::ImGuiTheme_MaterialFlat);
        if (ImGui::BeginTabBar("Main_Tabls", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem(u8"进程"))
            {
                static ProcessItem lastChooseProcess{};
                if (ImGui::BeginChild("#process", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.7), false, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    processWidget_.OnPaint();
                    ImGui::EndChild();
                }
                if (ImGui::BeginChild("#module", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    //设置数据源
                    if (processWidget_.GetCurtProcessItem().GetPid() != lastChooseProcess.GetPid())
                    {
                        moduleWidget_.SetDataSource(processWidget_.GetCurtProcessItem().GetPid());
                        lastChooseProcess = processWidget_.GetCurtProcessItem();
                    }
                    moduleWidget_.OnPaint();
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(u8"VEH插件"))
            {
                if (ImGui::BeginTabBar("Plugin_Tables", ImGuiTabBarFlags_None))
                {
                    static int choose = -1;
                    static std::vector<std::string> plugin_tables = { u8"SYSCALL监控" ,u8"调试器"};
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
                            syscallMonitorWidget_.OnPaint();
                            break;
                        }
                        case 1:
                        {
                            vehDebuggerWidget_.SetDataSource(processWidget_.GetPluginProcessItem());
                            vehDebuggerWidget_.OnPaint();
                            break;
                        }
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(u8"窗口"))
            {
                windowsWidget_.OnPaint();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}

void MainWidget::OnUpdate()
{
    while (true)
    {
        if (processWidget_.GetPluginProcessItem().GetPid() > 0)
        {
            //Syscall Monotor
            {
                static bool signal_syscallmonitor = false;
                if (syscallMonitorWidget_.GetActive())
                {
                    if (!signal_syscallmonitor)
                    {
                        ControlCmd cmd{};
                        cmd.cmd = ECMD::syscallmonitor_init;
                        cmd.syscall_state = syscallMonitorWidget_.GetActive();
                        if (!utils::mem::InvokePluginFunction(processWidget_.GetPluginProcessItem().GetPid(), cmd))
                        {
                            MessageBoxA(NULL, "SyscallMonitor Open Failed!", NULL, NULL);
                            syscallMonitorWidget_.SetActive(false); //clear checked
                        }
                        else {
                            signal_syscallmonitor = true; // make sure do once
                        }
                    }
                }
                else if (!syscallMonitorWidget_.GetActive())
                {
                    if (signal_syscallmonitor)
                    {
                        ControlCmd cmd{};
                        cmd.cmd = ECMD::syscallmonitor_init;
                        cmd.syscall_state = syscallMonitorWidget_.GetActive();
                        if (!utils::mem::InvokePluginFunction(processWidget_.GetPluginProcessItem().GetPid(), cmd))
                        {
                            MessageBoxA(NULL, "SyscallMonitor Open Failed!", NULL, NULL);
                            syscallMonitorWidget_.SetActive(true); //clear check
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
                    auto& tmp_dr = global::dbg::Dr[i];

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
                            if (cmd.hardbread.addr <= 0 || !utils::mem::InvokePluginFunction(processWidget_.GetPluginProcessItem().GetPid(), cmd))
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
                            if (!utils::mem::InvokePluginFunction(processWidget_.GetPluginProcessItem().GetPid(), cmd))
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
                            if (!utils::mem::InvokePluginFunction(processWidget_.GetPluginProcessItem().GetPid(), cmd))
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
                            if (!utils::mem::InvokePluginFunction(processWidget_.GetPluginProcessItem().GetPid(), cmd))
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

void MainWidget::OnIPC()
{
    char* buff = new char[8192];
    while (true)
    {
        if (global::plugin::plugin_.read_buffer(buff))
        {
            EDataType type = (EDataType)(*(uint8_t*)buff);
            switch (type)
            {
                case API:
                {
                    PApiMonitorInfo pApi = reinterpret_cast<PApiMonitorInfo>(buff);
                    if (!syscallMonitorWidget_.IsExsist(pApi->modulename))
                    {
                        syscallMonitorWidget_.SetValue(pApi->modulename, pApi->function, 1);
                    }
                    else
                    {
                        syscallMonitorWidget_.SetValue(pApi->modulename, pApi->function, 1, true);
                    }
                    break;
                }
                case DEBG:
                {
                    Debugger::PDbgBreakInfo pDbg = reinterpret_cast<Debugger::PDbgBreakInfo>(buff);
                    auto& curtData = Debugger::GetDbgInfoRef(pDbg->id);
                    //如果为执行断点，关闭启用选项
                    if (global::dbg::Dr[pDbg->id].type == 0)
                        global::dbg::Dr[pDbg->id].statue = 0;

                    if (curtData.capture.count(pDbg->ctx.Rip) <= 0)
                    {
                        curtData.ctx = pDbg->ctx;
                        //generate region disassembly
                        uint8_t buf[100]{};
                        MemStub::ReadMemory(processWidget_.GetPluginProcessItem().GetPid(), pDbg->ctx.Rip - 0x50, reinterpret_cast<uintptr_t>(buf), 0x100);
                        auto disam = Debugger::Disassembly(!processWidget_.GetPluginProcessItem().IsWow64(), pDbg->ctx.Rip - 0x50, buf, sizeof(buf));
                        for (int i = 0; i < disam.size(); i++)
                        {
                            if (disam[i][0] == utils::conver::IntegerTohex(pDbg->ctx.Rip))
                            {
                                if (global::dbg::Dr[pDbg->id].type == 0)
                                {
                                    strcpy_s(pDbg->disassembly, disam[i][2].c_str());
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
                        memcpy(t.stack, pDbg->stack, sizeof(t.dbginfo.stack));
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
    
void MainWidget::SetWindowSize(float w, float h)
{
    size_.x = w;
    size_.y = h;
}
