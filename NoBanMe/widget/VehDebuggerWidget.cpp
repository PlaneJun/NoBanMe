#include "VehDebuggerWidget.h"
#include "../../3part/imgui/Imgui_text_editor.h"
#include "../../common/utils/utils.h"
#include "../global.h"

void VehDebuggerWidget::OnPaint()
{
    static Debugger::DbgCaptureInfo last_choose_capture;
    auto curtData = Debugger::GetDbgInfo(global::dbg::curtChoose);
    if (ImGui::CollapsingHeader(u8"Ӳ���ϵ�", ImGuiTreeNodeFlags_DefaultOpen))
    {
        static const char* dr_statue[] = { u8"���",u8"�Ƴ�" };
        static const char* dr_size[] = { "1 byte", "2 byte", "4 byte", "8 byte" };
        static const char* dr_type[] = { u8"ִ��", u8"��/д", u8"д" };
        for (int i = 0;i < 4;i++)
        {
            char check_title[100]{};
            sprintf_s(check_title, u8"����     Dr%d:", i);
            if (global::dbg::Dr[i].statue == 0)//δ��Ӷϵ�ʱ��������
                ImGui::BeginDisabled();
            ImGui::Checkbox(check_title, &global::dbg::Dr[i].active); ImGui::SameLine();
            if (global::dbg::Dr[i].statue == 0)
                ImGui::EndDisabled();

            //����ʱ,�����޸ĵ�ַ�����͡���С
            if (global::dbg::Dr[i].active)
                ImGui::BeginDisabled();
            char input_title[100]{};
            sprintf_s(input_title, "##input_dr%d", i);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3);
            ImGui::InputTextWithHint(input_title, "adrress", global::dbg::Dr[i].addr, 1024); ImGui::SameLine();
            ImGui::PopItemWidth();

            ImGui::Text(u8"����:");ImGui::SameLine();
            char combo_type_title[100]{};
            sprintf_s(combo_type_title, "##combot_type_dr%d", i);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.2);
            ImGui::Combo(combo_type_title, &global::dbg::Dr[i].type, dr_type, IM_ARRAYSIZE(dr_type));ImGui::SameLine();
            ImGui::PopItemWidth();

            ImGui::Text(u8"��С:");ImGui::SameLine();
            char combo_size_title[100]{};
            sprintf_s(combo_size_title, "##combot_size_dr%d", i);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.2);
            ImGui::Combo(combo_size_title, &global::dbg::Dr[i].size, dr_size, IM_ARRAYSIZE(dr_size));ImGui::SameLine();
            ImGui::PopItemWidth();
            if (global::dbg::Dr[i].active)
                ImGui::EndDisabled();
            char button_id[100]{};
            sprintf_s(button_id, "btn_dr%d", i);
            ImGui::PushID(button_id);
            if (ImGui::Button(dr_statue[global::dbg::Dr[i].statue]))
            {
                global::dbg::Dr[i].statue++;
                if (global::dbg::Dr[i].statue > 1)
                    global::dbg::Dr[i].statue = 0;
            }
            ImGui::PopID();
        }
    }
    if (ImGui::CollapsingHeader(u8"���ݴ���", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::BeginTabBar("DataCollect_Tables", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem(u8"���ҷ���"))
            {
                if (ImGui::BeginTabBar("DrSelector_Tables", ImGuiTabBarFlags_None))
                {
                    for (int i = 0; i < 4; i++)
                    {
                        char buf[10]{};
                        sprintf_s(buf, "Dr%d", i);
                        if (ImGui::BeginTabItem(buf))
                        {
                            global::dbg::curtChoose = i;
                            ImGui::EndTabItem();
                        }
                    }
                    ImGui::EndTabBar();
                }
                if (ImGui::BeginChild("ChildAccess", ImVec2(ImGui::GetContentRegionAvail().x * 0.25, ImGui::GetContentRegionAvail().y * 0.66), false, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    breakrecordWidget_.SetDataSource(curtData.capture);
                    breakrecordWidget_.OnPaint();
                    if (last_choose_capture.text != breakrecordWidget_.GetCurtSelect().text)
                    {
                        contextWidget_.SetDataSource(curtData.ctx);
                        disassemblerWidget_.SetData(DataSource_, utils::conver::hexToInteger(breakrecordWidget_.GetCurtSelect().text));
                        std::vector<uint8_t> stack{};
                        stack.resize(sizeof(breakrecordWidget_.GetCurtSelect().stack));
                        memcpy(&stack[0], breakrecordWidget_.GetCurtSelect().stack, sizeof(breakrecordWidget_.GetCurtSelect().stack));
                        stackWidget_.SetDataSource(DataSource_.GetPid(), stack, curtData.ctx.Rsp);
                        last_choose_capture = breakrecordWidget_.GetCurtSelect();
                    }
                    ImGui::EndChild();
                }
                ImGui::SameLine();
                if (ImGui::BeginChild("ChildDisassembly", ImVec2(ImGui::GetContentRegionAvail().x * 0.66, ImGui::GetContentRegionAvail().y * 0.66)))
                {
                    disassemblerWidget_.OnPaint();
                    ImGui::EndChild();
                }
                ImGui::SameLine();
                if (ImGui::BeginChild("ChildContext", ImVec2(ImGui::GetContentRegionAvail().x * 0.98, ImGui::GetContentRegionAvail().y * 0.66)))
                {

                    contextWidget_.OnPaint();
                    ImGui::EndChild();
                }
                ImGui::Separator();
                if (ImGui::BeginChild("ChildMem", ImVec2(ImGui::GetContentRegionAvail().x * 0.65, ImGui::GetContentRegionAvail().y)))
                {
                    /*ImGui::SeparatorText(u8"�ڴ�");
                    static MemoryEditor mem_edit;
                    mem_edit.DrawWindow("Memory Editor", curtData.memoryRegion.data.data(), curtData.memoryRegion.data.size(), curtData.memoryRegion.start);*/
                    ImGui::EndChild();
                }
                ImGui::SameLine();
                if (ImGui::BeginChild("ChildStack"))
                {
                    stackWidget_.OnPaint();
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem(u8"���ݷ���"))
            {
                static TextEditor editor;
                static bool init = false;
                if (!init)
                {
                    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
                    //editor.SetPalette(TextEditor::GetLightPalette());
                    init = true;
                }
                
                editor.Render("Lua Script Engine");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
}

void VehDebuggerWidget::SetDataSource(ProcessItem d)
{
    DataSource_ = d;
}
