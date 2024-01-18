#pragma once
#include <vector>
#include <imgui.h>
#include <stdint.h>

class MemoryItem
{
public:
    enum EInfo
    {
        BASE,
        SIZE,
        PROTECT,
        STATE,
        TYPE
    };

public:

    MemoryItem();

    MemoryItem(uint64_t base, uint32_t size, uint32_t protect, uint32_t state, uint32_t type);

    uint64_t GetBase();

    uint32_t GetSize();

    uint32_t GetProtect();

    uint32_t GetType();

    uint32_t GetState();

public:

    static void EnumPidMemoryBlocks(uint32_t pid, std::vector<MemoryItem>& items);

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, MemoryItem* items, int items_count);

    static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs);

    static void SetSortSpecs(ImGuiTableSortSpecs* a);

private:
    static const ImGuiTableSortSpecs* s_current_sort_specs_;
    uint64_t base_;
    uint32_t size_;
    uint32_t protect_;
    uint32_t state_;
    uint32_t type_;
};
