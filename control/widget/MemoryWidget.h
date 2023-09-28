#pragma once
#pragma once
class MemoryWidget
{
public:
    void OnPaint()
    {
        if (show_)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowPadding = { 0.f,0.f };
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::Begin(u8"内存列表", &show_, ImGuiWindowFlags_NoCollapse);
            if (ImGui::BeginTable("#memorylist", 5, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
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
                        if (DataSource_.size() > 1)
                            qsort(&DataSource_[0], (size_t)DataSource_.size(), sizeof(DataSource_[0]), MemoryItem::CompareWithSortSpecs);
                        MemoryItem::SetSortSpecs(NULL);
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
                        MemoryItem* item = &DataSource_[row_n];
                        ImGui::PushID(item->GetBase());
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        if (ImGui::Selectable(utils::conver::IntegerTohex(item->GetBase()).c_str(), selected_ == row_n, ImGuiSelectableFlags_SpanAllColumns))
                        {
                            selected_ = row_n;
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
    }

    void SetDataSource(uint32_t pid)
    {
        DataSource_.clear();
        MemoryItem::EnumPidMemoryBlocks(pid, DataSource_);
        selected_ = -1;
    }

    void Load()
    {
        show_ = true;
    }
private:

    std::vector<MemoryItem> DataSource_{};
    int selected_ = -1;
    bool show_;
};