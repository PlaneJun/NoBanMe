#include "ModuleWidget.h"
#include <imgui.h>
#include "../render/render.h"
#include "../utils/utils.h"

void ModuleWidget::OnPaint()
{
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
                if (DataSource_.size() > 1)
                    qsort(&DataSource_[0], (size_t)DataSource_.size(), sizeof(DataSource_[0]), ModuleItem::CompareWithSortSpecs);
                ModuleItem::SetSortSpecs(NULL);
                sorts_specs->SpecsDirty = false;
            }
        }
        ImGuiListClipper clipper;
        clipper.Begin(DataSource_.size());
        while (clipper.Step())
        {
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
            {
                ModuleItem* item = &DataSource_[row_n];
                ImGui::PushID(item->GetBase());
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Selectable(utils::conver::string_To_UTF8(item->GetImagePath()).c_str(), selected_ == row_n, ImGuiSelectableFlags_SpanAllColumns))
                    selected_ = row_n;
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

        if (selected_ != -1 && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(1))
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
                    utils::file::OpenFolderAndSelectFile(DataSource_[selected_].GetImagePath().c_str());
                    break;
                }
                case 1:
                {
                    utils::file::OpenFilePropertyDlg(DataSource_[selected_].GetImagePath().c_str());
                    break;
                }
            }
            ImGui::EndPopup();
        }
        ImGui::EndTable();
    }
}

void ModuleWidget::SetDataSource(uint32_t pid)
{
    DataSource_.clear();
    ModuleItem::EnumPidModules(pid, DataSource_);
}

