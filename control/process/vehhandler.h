#pragma once

#define FIELD_OFFSET(type, field)    ((LONG)(LONG_PTR)&(((type *)0)->field))
#define UFIELD_OFFSET(type, field)    ((DWORD)(LONG_PTR)&(((type *)0)->field))
#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))

class VehHandlerItem
{
public:
    enum EInfo
    {
        ENTRY,
        MODULEPATH
    };

    typedef struct _RTL_VECTORED_HANDLER_LIST
    {
        SRWLOCK ExceptionLock;
        LIST_ENTRY ExceptionList;
        SRWLOCK ContinueLock;
        LIST_ENTRY ContinueList;
    } RTL_VECTORED_HANDLER_LIST, * PRTL_VECTORED_HANDLER_LIST;


    typedef struct _RTL_VECTORED_EXCEPTION_ENTRY
    {
        LIST_ENTRY List;
        PULONG_PTR Flag;
        ULONG RefCount;
        PVECTORED_EXCEPTION_HANDLER VectoredHandler;
    } RTL_VECTORED_EXCEPTION_ENTRY, * PRTL_VECTORED_EXCEPTION_ENTRY;


public:

    VehHandlerItem(uint64_t entry, std::string module_path,bool isVeh)
    {
        this->entry_ = entry;
        this->module_path_ = module_path;
        this->isVeh_ = isVeh;
    }

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, VehHandlerItem* items, int items_count)
    {
        s_current_sort_specs_ = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), VehHandlerItem::CompareWithSortSpecs);
        s_current_sort_specs_ = NULL;
    }

    static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const VehHandlerItem* a = (const VehHandlerItem*)lhs;
        const VehHandlerItem* b = (const VehHandlerItem*)rhs;
        for (int n = 0; n < s_current_sort_specs_->SpecsCount; n++)
        {
            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
            const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID)
            {
            case ENTRY:                   delta = (a->entry_ - b->entry_);   break;
            case MODULEPATH:                  delta = (strcmp(a->module_path_.c_str(), b->module_path_.c_str()));                                      break;
            default: IM_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }

        return (a->entry_ - b->entry_);
    }

    auto GetEntry()
    {
        return entry_;
    }

    auto GetModulePath()
    {
        return module_path_;
    }

    auto IsVeh()
    {
        return isVeh_;
    }

    static void SetSortSpecs(ImGuiTableSortSpecs* a)
    {
        s_current_sort_specs_ = a;
    }
    static void SetLdrpVectorHandlerList(uint64_t rva)
    {
        rva_LdrpVectorHandlerList_ = reinterpret_cast<uint64_t>(GetModuleHandleA("ntdll.dll")) + rva;
    }
    static uint64_t GetLdrpVectorHandlerList()
    {
        return rva_LdrpVectorHandlerList_;
    }
private:
    static const ImGuiTableSortSpecs* s_current_sort_specs_;
    static uint64_t rva_LdrpVectorHandlerList_;
    uint64_t entry_;
    std::string module_path_;
    bool isVeh_;
};

const ImGuiTableSortSpecs* VehHandlerItem::s_current_sort_specs_ = nullptr;
uint64_t VehHandlerItem::rva_LdrpVectorHandlerList_ = NULL;