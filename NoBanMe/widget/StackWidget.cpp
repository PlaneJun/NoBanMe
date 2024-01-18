#include "StackWidget.h"
#include <filesystem>
#include "../utils/utils.h"
#include "../render/render.h"

void StackWidget::OnPaint()
{
	ImGui::SeparatorText(u8"��ջ");
	static std::vector<std::string> headers = { "Rsp" ,"Value"," " };
	std::vector<std::vector<std::string>> text{};
	if (DataSource_.size() > 0)
	{
		//���λ�ý�������32�Ļ�,8��Ҫ�ģ�������涯̬��ȡ
		for (int i = 0; i < DataSource_.size() / 8; i++)
		{
			std::vector<std::string> tmp{};
			uint64_t v = *(uint64_t*)&DataSource_[i * 8];
			tmp.push_back(utils::conver::IntegerTohex(rsp_ + i * 8));
			tmp.push_back(utils::conver::IntegerTohex(v));
			tmp.push_back("");
			for (auto& item : mitems_)
			{
				if (v >= item.GetBase() && v <= item.GetBase() + item.GetSize())
				{
					uint64_t detal = (item.GetBase() + item.GetSize()) - v;
					tmp[2] = std::filesystem::path(item.GetImagePath()).filename().string().append(utils::conver::IntegerTohex(detal));
					break;
				}
			}

			text.push_back(tmp);
		}
	}

	if (selected != -1)
	{
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
			ImGui::OpenPopup("stack_option");

		if (ImGui::BeginPopup("stack_option"))
		{
			if (ImGui::BeginMenu(u8"����"))
			{
				switch (int s = render::get_instasnce()->DrawItemBlock({ u8"Rsp",u8"Value",u8"ע��" }))
				{
					case 0:
					{
						utils::normal::CopyStringToClipboard(text[selected][0].c_str());
						break;
					}
					case 1:
					{
						utils::normal::CopyStringToClipboard(text[selected][1].c_str());
						break;
					}
					case 2:
					{
						utils::normal::CopyStringToClipboard(text[selected][2].c_str());
						break;
					}
				}
				ImGui::Separator();
				switch (int s = render::get_instasnce()->DrawItemBlock({ u8"����" }))
				{
					case 0:
					{
						char buff[8192]{};
						sprintf_s(buff, "%d | %s | %s", text[selected][0].c_str(), text[selected][1].c_str(), text[selected][2].c_str());
						utils::normal::CopyStringToClipboard(buff);
						break;
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}
	}

	render::get_instasnce()->AddListBox("##Stack", selected, 0, headers, text);
}


void StackWidget::SetDataSource(uint32_t pid,std::vector<uint8_t> data,uint64_t rsp)
{
	if (rsp != rsp_)
	{
		mitems_.clear();
		ModuleItem::EnumPidModules(pid_, mitems_);
		selected = -1;
	}

	pid_ = pid;
	DataSource_ = data;
	rsp_ = rsp;
		
}
