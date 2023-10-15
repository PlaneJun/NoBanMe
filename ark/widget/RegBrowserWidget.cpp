#include "RegBrowserWidget.h"
#include <Windows.h>
#include "../3rdParty/imgui/imgui.h"

void RegBrowserWidget::OnPaint()
{
	char name[256]{};
	DWORD len = _countof(name);
	GetComputerNameA(name, &len);
	if (ImGui::TreeNode(std::string(name).append(" (Local)").c_str()))
	{
		if (ImGui::TreeNode("Standard Registry"))
		{
			for (auto& k : Keys) {
				if (!k.text.empty() && GetSubKeyCount(k.hKey)>0)
				{
					
					if (ImGui::TreeNode(k.text.c_str()))
					{
						ImGui::TreePop();
					}
					
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("REGISTRY"))
		{
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
}

DWORD RegBrowserWidget::GetSubKeyCount(HKEY hKey, DWORD* values, FILETIME* ft)
{
	DWORD subkeys = 0;
	auto error = ::RegQueryInfoKey(hKey, nullptr, 0, nullptr, &subkeys, nullptr,
		nullptr, values, nullptr, nullptr, nullptr, ft);
	::SetLastError(error);
	return subkeys;
}