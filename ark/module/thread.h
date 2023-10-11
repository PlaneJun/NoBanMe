#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include <imgui.h>

class ThreadItem
{
public:
    enum EInfo
    {
        THREAID,
        PRIORITY,
        STARTADDR,
        MODULEPATH
    };

public:

    ThreadItem();

    ThreadItem(uint32_t threadid, uint32_t Prority, uint64_t startAddr, const char* path);

    uint32_t GetThreadId();

    uint32_t GetPrority();

    uint64_t GetThreadEntry();

    std::string GetModulePath();

public:

    static 	void EnumPidThread(uint32_t pid, std::vector<ThreadItem>& items);

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ThreadItem* items, int items_count);

    static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs);

    static void SetSortSpecs(ImGuiTableSortSpecs* a);

private:
    static const ImGuiTableSortSpecs* s_current_sort_specs_;
    uint32_t threadid_;
    uint32_t Prority_;
    uint64_t startAddr_;
    std::string module_path_;

};