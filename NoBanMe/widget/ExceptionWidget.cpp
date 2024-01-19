#include "ExceptionWidget.h"
#include <imgui.h>
#include "../module/process.h"
#include "../render/render.h"
#include "../../common/utils/utils.h"

void ExceptionWidget::OnPaint()
{
    if (!show_)
        return;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = { 0.f,0.f };
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::Begin(u8"�쳣�ص��б�", &show_, ImGuiWindowFlags_NoCollapse);
    if (ImGui::BeginTable("#vehhandlerlist", 3, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
    {
        ImGui::TableSetupColumn(u8"�߳����", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, VehHandlerItem::EInfo::ENTRY);
        ImGui::TableSetupColumn(u8"����", ImGuiTableColumnFlags_WidthFixed, 0.0f, VehHandlerItem::EInfo::TYPE);
        ImGui::TableSetupColumn(u8"ģ��·��", ImGuiTableColumnFlags_WidthFixed, 0.0f, VehHandlerItem::EInfo::MODULEPATH);
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
                VehHandlerItem* item = &DataSource_[row_n];
                ImGui::PushID(item->GetEntry());
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Selectable(utils::conver::IntegerTohex(item->GetEntry()).c_str(), selected_ == row_n, ImGuiSelectableFlags_SpanAllColumns))
                {
                    selected_ = row_n;
                }
                ImGui::TableNextColumn();
                ImGui::Text(item->IsVeh() ? "VEH Exception" : "VEH Continue");
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
			ImGui::OpenPopup("exception_option");

		if (ImGui::BeginPopup("exception_option"))
		{
            if (ImGui::BeginMenu(u8"����"))
            {
				switch (int s = render::get_instasnce()->DrawItemBlock({ u8"�߳����",u8"ģ��·��" }))
				{
					case 0:
					{
						utils::normal::CopyStringToClipboard(render::get_instasnce()->GetHwnd(), utils::conver::IntegerTohex(DataSource_[selected_].GetEntry()).c_str());
						break;
					}
					case 1:
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

void ExceptionWidget::SetDataSource(uint32_t pid)
{
    DataSource_.clear();
    VehHandlerItem::EnumVehHandler(pid, DataSource_);
    selected_ = -1;
}

void ExceptionWidget::Load()
{
    show_ = true;
}
