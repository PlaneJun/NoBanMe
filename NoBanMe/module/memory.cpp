#include "memory.h"
#include <Windows.h>

const ImGuiTableSortSpecs* MemoryItem::s_current_sort_specs_ = nullptr;

MemoryItem::MemoryItem()
{
    this->base_ = 0;
    this->size_ = 0;
    this->protect_ = 0;
    this->state_ = 0;
    this->type_ = 0;
}

MemoryItem::MemoryItem(uint64_t base, uint32_t size,uint32_t protect,uint32_t state,uint32_t type)
{
    this->base_ = base;
    this->size_ = size;
    this->protect_ = protect;
    this->state_ = state;
    this->type_ = type;
}


uint64_t MemoryItem::GetBase()
{
    return base_;
}

uint32_t MemoryItem::GetSize()
{
    return size_;
}

uint32_t MemoryItem::GetProtect()
{
    return protect_;
}

uint32_t MemoryItem::GetType()
{
    return type_;
}

uint32_t MemoryItem::GetState()
{
    return state_;
}

void MemoryItem::EnumPidMemoryBlocks(uint32_t pid, std::vector<MemoryItem>& items)
{
    auto hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
    if (!hProcess)
        return;
    PBYTE pAddress = NULL;
    MEMORY_BASIC_INFORMATION  mbi = { 0 };
       
    while (TRUE)
    {
        MemoryItem item{};
        if (VirtualQueryEx(hProcess, pAddress, &mbi, sizeof(mbi)) != sizeof(mbi))
            break;

        item.base_ = reinterpret_cast<uint64_t>(mbi.BaseAddress);
        item.size_ = mbi.RegionSize;
        item.type_ = mbi.Type;
        item.state_ = mbi.State;
        item.protect_ = mbi.AllocationProtect;
        items.push_back(item);
        pAddress = ((PBYTE)mbi.BaseAddress + mbi.RegionSize);
    }
    CloseHandle(hProcess);
}

void MemoryItem::SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, MemoryItem* items, int items_count)
{
    s_current_sort_specs_ = sort_specs; // Store in variable accessible by the sort function.
    if (items_count > 1)
        qsort(items, (size_t)items_count, sizeof(items[0]), MemoryItem::CompareWithSortSpecs);
    s_current_sort_specs_ = NULL;
}

int __cdecl MemoryItem::CompareWithSortSpecs(const void* lhs, const void* rhs)
{
    const MemoryItem* a = (const MemoryItem*)lhs;
    const MemoryItem* b = (const MemoryItem*)rhs;
    for (int n = 0; n < s_current_sort_specs_->SpecsCount; n++)
    {
        // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
        // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
        const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_->Specs[n];
        int delta = 0;
        switch (sort_spec->ColumnUserID)
        {
        case BASE:              delta = (a->base_- b->base_);                                      break;
        case SIZE:              delta = (a->size_ - b->size_);                                    break;
        case PROTECT:              delta = (a->protect_ - b->protect_);                                    break;
        case STATE:              delta = (a->state_ - b->state_);                                    break;
        case TYPE:              delta = (a->type_ - b->type_);                                    break;
        default: IM_ASSERT(0); break;
        }
        if (delta > 0)
            return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
        if (delta < 0)
            return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
    }

    return (a->base_ - b->base_);
}

void MemoryItem::SetSortSpecs(ImGuiTableSortSpecs* a)
{
    s_current_sort_specs_ = a;
}


