#pragma once
class WindowsWidget
{
public:
    void OnPaint()
    {
        static int last_selected = -1;
        if (ImGui::BeginChild("#desktopwnd", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y*0.4),false,ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (MainDataSource_.empty())
                Refresh();
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            for (auto wnd : MainDataSource_)
            {
                if (ImGui::TreeNode(u8"窗口列表"))
                {
                    for (int n = 0; n < wnd.second.size(); n++)
                    {
                        if (wnd.second[n].GetIcon())
                            ImGui::Image(wnd.second[n].GetIcon(), ImVec2(16, 16));
                        else
                            ImGui::Text("");
                        ImGui::SameLine();
                        if (ImGui::Selectable(utils::conver::string_To_UTF8(wnd.second[n].GetText()).c_str(), main_selected_ == n))
                        {
                            main_selected_ = n;
                            if (last_selected != main_selected_)
                            {
                                ChildDataSource_.clear();
                                EnumChildWindows(wnd.second[n].GetHwnd(), [](auto hWnd, auto lp)->BOOL {
                                    std::vector<WindowItem>* r = reinterpret_cast<std::vector<WindowItem>*>(lp);
                                    r->push_back(AddNode(hWnd));
                                    return TRUE;
                                    }, reinterpret_cast<LPARAM>(&ChildDataSource_));
                                last_selected = main_selected_;
                            }
                        }
                       
                    }
                    ImGui::TreePop();
                }
            }

            if (main_selected_ != -1 && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(1))
                ImGui::OpenPopup("window_option");

            if (ImGui::BeginPopup("window_option"))
            {
                switch (int s = render::get_instasnce()->DrawItemBlock({ u8"刷新",u8"置顶" }))
                {
                    case 0:
                    {
                        Refresh();
                        break;
                    }
                    case 1:
                    {
                        for (auto wnd : MainDataSource_)
                        {
                            wnd.second[main_selected_].Show();
                            break;
                        }
                        break;
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::EndChild();
        }
        
        if (ImGui::BeginChild("#childwnd", ImVec2(0, 0), false))
        {
            if (ImGui::BeginTable("#childwndinfo", 13, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV, ImVec2(0.0f, 0), 0.0f))
            {
                ImGui::TableSetupColumn("##icon", ImGuiTableColumnFlags_NoSort, 0.0f);
                ImGui::TableSetupColumn(u8"类名", ImGuiTableColumnFlags_DefaultSort|ImGuiTableColumnFlags_WidthFixed, 0.0f, WindowItem::EInfo::CLASSNAME);
                ImGui::TableSetupColumn(u8"句柄", ImGuiTableColumnFlags_DefaultSort|ImGuiTableColumnFlags_WidthFixed, 0.0f, WindowItem::EInfo::HWND);
                ImGui::TableSetupColumn(u8"标题", ImGuiTableColumnFlags_DefaultSort| ImGuiTableColumnFlags_WidthFixed, 0.0f, WindowItem::EInfo::PHWND);
                ImGui::TableSetupColumn(u8"风格", ImGuiTableColumnFlags_DefaultSort|ImGuiTableColumnFlags_WidthFixed, 0.0f, WindowItem::EInfo::STYLE);
                ImGui::TableSetupColumn(u8"额外风格", ImGuiTableColumnFlags_DefaultSort|ImGuiTableColumnFlags_WidthFixed, 0.0f, WindowItem::EInfo::STYLEEX);
                ImGui::TableSetupColumn(u8"进程ID", ImGuiTableColumnFlags_DefaultSort|ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(u8"线程ID", ImGuiTableColumnFlags_DefaultSort|ImGuiTableColumnFlags_WidthFixed, 0.0f, WindowItem::EInfo::TID);
                ImGui::TableSetupColumn(u8"进程名", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(u8"相对位置", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(u8"绝对位置", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(u8"父句柄", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn(u8"窗口回调", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();
                if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
                {
                    if (sorts_specs->SpecsDirty)
                    {
                        WindowItem::SetSortSpecs(sorts_specs); // Store in variable accessible by the sort function.
                        if (ChildDataSource_.size() > 1)
                            qsort(&ChildDataSource_[0], (size_t)ChildDataSource_.size(), sizeof(ChildDataSource_[0]), WindowItem::CompareWithSortSpecs);
                        WindowItem::SetSortSpecs(NULL);
                        sorts_specs->SpecsDirty = false;
                    }
                }
                ImGuiListClipper clipper;
                clipper.Begin(ChildDataSource_.size());
                while (clipper.Step())
                {
                    for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                    {
                        WindowItem* item = &ChildDataSource_[row_n];
                        ImGui::PushID(item->GetHwnd());
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        if (item->GetIcon())
                            ImGui::Image(item->GetIcon(), ImVec2(16, 16));
                        ImGui::TableNextColumn();
                        ImGui::Text(utils::conver::string_To_UTF8(item->GetKlassName()).c_str());
                        ImGui::TableNextColumn();
                        if (ImGui::Selectable(std::to_string((int)item->GetHwnd()).c_str(), child_selected_ == row_n, ImGuiSelectableFlags_SpanAllColumns))
                        {
                            child_selected_ = row_n;
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text(utils::conver::string_To_UTF8(item->GetTitle()).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", utils::window::GetWindowStyleToString(item->GetStyle()).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", utils::window::GetWindowExtendedStyleToString(item->GetStyleEx()).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text("%d",item->GetPid());
                        ImGui::TableNextColumn();
                        ImGui::Text("%d",item->GetThreadId());
                        ImGui::TableNextColumn();
                        ImGui::Text("%s",item->GetProcessName().c_str());
                        ImGui::TableNextColumn();
                        auto r = item->GetRectRelativa();
                        ImGui::Text("(%d,%d)-(%d,%d)",r.left,r.top,r.right,r.bottom);
                        ImGui::TableNextColumn();
                         r = item->GetRectScreen();
                        ImGui::Text("(%d,%d)-(%d,%d)", r.left, r.top, r.right, r.bottom);
                        ImGui::TableNextColumn();
                        ImGui::Text("0x%zX",item->GetParent());
                        ImGui::TableNextColumn();
                        ImGui::Text("0x%zX",item->GetWndCb());
                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
        }
    }

    void Refresh()
    {
        MainDataSource_.clear();
        HWND hDesktop = GetDesktopWindow();
        if (hDesktop)
        {
            MainDataSource_[hDesktop].push_back(AddNode(hDesktop));
            EnumWindows([](auto hWnd, auto lp)->BOOL {
                if (IsWindowVisible(hWnd))
                {
                    std::map<HWND, std::vector<WindowItem>>* r = reinterpret_cast<std::map<HWND, std::vector<WindowItem>>*>(lp);
                    (*r)[GetAncestor(hWnd, GA_PARENT)].push_back(AddNode(hWnd));
                }
                return TRUE;
                }, reinterpret_cast<LPARAM>(&MainDataSource_));
            main_selected_ = 0;
        }
    }

private:
    int main_selected_ = -1;
    int child_selected_ = -1;
    std::map<HWND, std::vector<WindowItem>> MainDataSource_;
    std::vector<WindowItem> ChildDataSource_;
    static WindowItem AddNode(HWND hwnd)
    {
        WindowItem wnd{};
        wnd.SetHwnd(hwnd);
        wnd.SetParent(GetAncestor(hwnd, GA_PARENT));
        wnd.SetTitle(utils::window::GetTitleName(hwnd).c_str());
        wnd.SetKlassName(utils::window::GetKlassName(hwnd).c_str());
        wnd.SetStyle(GetWindowLong(hwnd, GWL_STYLE));
        wnd.SetStyleEx(GetWindowLong(hwnd, GWL_EXSTYLE));
        wnd.SetParent(GetParent(hwnd));
        wnd.SetWndCb(GetWindowLongPtr(hwnd,GWLP_WNDPROC));
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        wnd.SetPid(pid);
        wnd.SetThreadId(GetWindowThreadProcessId(hwnd, nullptr));
        wnd.SetProcessName(utils::process::GetFullProcessName(pid));
        RECT r{};
        GetWindowRect(hwnd,&r);
        wnd.SetRectScreen(r);
        GetClientRect(hwnd, &r);
        wnd.SetRectRelativa(r);
        ID3D11ShaderResourceView* icon = nullptr;
        auto hIcon = utils::image::GetProcessIcon(hwnd);
        if (hIcon != NULL && utils::image::SaveIconToPng(hIcon, ("./Data/icon/##" + std::to_string((int)hwnd) + ".png").c_str()))
            wnd.SetIcon(render::get_instasnce()->DX11LoadTextureImageFromFile(("./Data/icon/##" + std::to_string((int)hwnd) + ".png").c_str()));

        return wnd;
    }
};