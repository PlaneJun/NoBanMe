#pragma once
#include <string>
#include <vector>
#include <stdint.h>
#include <imgui.h>
class ModuleItem
{
public:
    enum EInfo
    {
        IMAGEPATH,
        BASE,
        SIZE
    };

public:
    ModuleItem();

    ModuleItem(std::string imagepath, uint64_t base, size_t size, std::string descripttion, std::string fileversion, std::string companyname);

    std::string GetImagePath();

    uint64_t GetBase();

    size_t GetSize();

    std::string GetFileVersion();

    std::string GetDecription();

    std::string GetCompanyName();

public:
    static void EnumPidModules(uint32_t pid, std::vector<ModuleItem>& items);

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ModuleItem* items, int items_count);

    static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs);

    static void SetSortSpecs(ImGuiTableSortSpecs* a);

private:

    static const ImGuiTableSortSpecs* s_current_sort_specs_;
    std::string imagepath_;
    uint64_t base_;
    size_t size_;
    std::string fileversion_;
    std::string companyname_;
    std::string description_;
};
