#pragma once

class ProcessItem
{
public :
    enum EInfo
    {
        PID,
        NAME,
        PPID,
        FULLPATH,
        STARTUPTIME
    };

public:

    ProcessItem(ID3D11ShaderResourceView* icon, uint32_t pid, std::string name,uint32_t ppid,std::string fullpath,std::string startuptime,bool isWow64)
    {
        this->icon_ = icon;
        this->pid_ = pid;
        this->name_ = name;
        this->ppid_ = ppid;
        this->fullpath_ = fullpath;
        this->startuptime_ = startuptime;
        this->bWow64 = isWow64;
    }

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ProcessItem* items, int items_count)
    {
        s_current_sort_specs_ = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), ProcessItem::CompareWithSortSpecs);
        s_current_sort_specs_ = NULL;
    }

    static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const ProcessItem* a = (const ProcessItem*)lhs;
        const ProcessItem* b = (const ProcessItem*)rhs;
        for (int n = 0; n < s_current_sort_specs_->SpecsCount; n++)
        {
            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
            const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID)
            {
            case PID:               delta = (a->pid_ - b->pid_);                                          break;
            case NAME:              delta = (strcmp(a->name_.c_str(), b->name_.c_str()));                 break;
            case PPID:              delta = (a->ppid_ - b->ppid_);                                        break;
            case FULLPATH:          delta = (strcmp(a->fullpath_.c_str(), b->fullpath_.c_str()));         break;
            case STARTUPTIME:       delta = (strcmp(a->startuptime_.c_str(), b->startuptime_.c_str()));   break;
            default: IM_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }

        return (a->pid_ - b->pid_);
    }

    static void SetSortSpecs(ImGuiTableSortSpecs* a)
    {
        s_current_sort_specs_ = a;
    }

    auto GetIcon()
    {
        return icon_;
    }

    auto GetPid()
    {
        return pid_;
    }

    auto GetName()
    {
        return name_;
    }

    auto GetPPid()
    {
        return ppid_;
    }

    auto GetFullPath()
    {
        return fullpath_;
    }

    auto GetStartUpTime()
    {
        return startuptime_;
    }

    auto IsWow64()
    {
        return bWow64;
    }

private:
    
    static const ImGuiTableSortSpecs* s_current_sort_specs_;
    ID3D11ShaderResourceView* icon_;
    uint32_t pid_;
    std::string name_;
    uint32_t ppid_;
    std::string fullpath_;
    std::string startuptime_;
    bool bWow64;
};

const ImGuiTableSortSpecs* ProcessItem::s_current_sort_specs_=nullptr;
