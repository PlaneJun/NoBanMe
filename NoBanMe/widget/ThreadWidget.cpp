#include "ThreadWidget.h"
#include <imgui.h>
#include <tchar.h>
#include "../render/render.h"
#include "../module/process.h"
#include "../../common/utils/utils.h"
void ThreadWidget::OnPaint()
{
    if (!show_)
        return;
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = { 0.f,0.f };
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::Begin(u8"线程列表", &show_, ImGuiWindowFlags_NoCollapse);
    if (ImGui::BeginTable("#threadlist", 4, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
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
                // Display a data item
                ThreadItem* item = &DataSource_[row_n];
                ImGui::PushID(item->GetThreadId());
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Selectable(std::to_string(item->GetThreadId()).c_str(), selected_ == row_n, ImGuiSelectableFlags_SpanAllColumns))
                {
                    selected_ = row_n;
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

	if (selected_ != -1)
	{
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
			ImGui::OpenPopup("thread_option");

		if (ImGui::BeginPopup("thread_option"))
		{
            if (ImGui::BeginMenu(u8"复制"))
            {
				switch (int s = render::get_instasnce()->DrawItemBlock({ u8"线程ID",u8"线程入口",u8"优先级",u8"所属模块" }))
				{
					case 0:
					{
						utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), std::to_string(DataSource_[selected_].GetThreadId()).c_str());
						break;
					}
					case 1:
					{
						utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), utils::conver::IntegerTohex(DataSource_[selected_].GetThreadEntry()).c_str());
						break;
					}
					case 2:
					{
						utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), std::to_string(DataSource_[selected_].GetPrority()).c_str());
						break;
					}
					case 3:
					{
						utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), DataSource_[selected_].GetModulePath().c_str());
						break;
					}
				}
                ImGui::EndMenu();
            }
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

void ThreadWidget::SetDataSource(uint32_t pid)
{
    DataSource_.clear();
    ThreadItem::EnumPidThread(pid, DataSource_);
    selected_ = -1;
}

void ThreadWidget::Load()
{
    show_ = true;
}
