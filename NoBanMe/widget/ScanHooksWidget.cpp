#include <imgui.h>
#include "ScanHooksWidget.h"
#include "../module/hooks.h"

void ScanHooksWidget::OnPaint()
{
	if (!show_)
		return;
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = { 0.f,0.f };
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::Begin(u8"扫描列表", &show_, ImGuiWindowFlags_NoCollapse);
	if (ImGui::BeginTable("#hookslist", 5, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_Sortable | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
	{
		ImGui::TableSetupColumn(u8"地址", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, HookItem::EInfo::ADDRESS);
		ImGui::TableSetupColumn(u8"类型", ImGuiTableColumnFlags_WidthFixed, 0.0f, HookItem::EInfo::TYPE);
		ImGui::TableSetupColumn(u8"原始字节码", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 0.0f, HookItem::EInfo::ORIGINCODE);
		ImGui::TableSetupColumn(u8"当前字节码", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 0.0f, HookItem::EInfo::CURTCODE);
		ImGui::TableSetupColumn(u8"说明", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 0.0f, HookItem::EInfo::TIP);
		ImGui::TableHeadersRow();


		ImGui::EndTable();

	}
	ImGui::End();
}

void ScanHooksWidget::SetDataSource(uint32_t pid)
{
}

void ScanHooksWidget::Load()
{
	show_ = true;
}
