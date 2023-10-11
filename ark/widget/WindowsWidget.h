#pragma once
#include <vector>
#include <map>
#include <Windows.h>
#include "../module/window.h"

class WindowsWidget
{
public:
    void OnPaint();

    void Refresh();

private:
    int main_selected_ = -1;
    int child_selected_ = -1;
    std::map<HWND, std::vector<WindowItem>> MainDataSource_;
    std::vector<WindowItem> ChildDataSource_;

    static WindowItem AddNode(HWND hwnd);
};