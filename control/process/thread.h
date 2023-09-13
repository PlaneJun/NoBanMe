#pragma once

class ThreadItem
{
public:
    enum EInfo
    {
        THREAID,
        PRIORITY,
        STARTADDR
    };

public:

    ThreadItem(uint32_t threadid, uint32_t Prority, uint64_t startAddr)
    {
        this->threadid_ = threadid;
        this->Prority_ = Prority;
        this->startAddr_ = startAddr;
    }

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ThreadItem* items, int items_count)
    {
        s_current_sort_specs_ = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), ThreadItem::CompareWithSortSpecs);
        s_current_sort_specs_ = NULL;
    }

    static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const ThreadItem* a = (const ThreadItem*)lhs;
        const ThreadItem* b = (const ThreadItem*)rhs;
        for (int n = 0; n < s_current_sort_specs_->SpecsCount; n++)
        {
            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
            const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID)
            {
            case THREAID:                   delta = (a->threadid_ - b->threadid_);   break;
            case PRIORITY:                  delta = (a->Prority_ - b->Prority_);                                      break;
            case STARTADDR:                 delta = (a->startAddr_ - b->startAddr_);                                    break;
            default: IM_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }

        return (a->threadid_ - b->threadid_);
    }

    auto GetThreadId() 
    {
        return threadid_;
    }

    auto GetPrority()
    {
        return Prority_;
    }

    auto GetThreadEntry()
    {
        return startAddr_;
    }

    static void SetSortSpecs(ImGuiTableSortSpecs* a)
    {
        s_current_sort_specs_ = a;
    }
private:
    static const ImGuiTableSortSpecs* s_current_sort_specs_;
    uint32_t threadid_;
    uint32_t Prority_;
    uint64_t startAddr_;
};

const ImGuiTableSortSpecs* ThreadItem::s_current_sort_specs_ = nullptr;