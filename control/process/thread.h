#pragma once

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

    ThreadItem(uint32_t threadid, uint32_t Prority, uint64_t startAddr,const char* path)
    {
        this->threadid_ = threadid;
        this->Prority_ = Prority;
        this->startAddr_ = startAddr;
        this->module_path_ = path;
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

    auto GetModulePath()
    {
        return module_path_;
    }

public:

    static 	void EnumPidThread(uint32_t pid, std::vector<ThreadItem>& items)
    {
        THREADENTRY32 te32;
        HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hThreadSnap == INVALID_HANDLE_VALUE)
            return;

        te32.dwSize = sizeof(THREADENTRY32);
        if (!Thread32First(hThreadSnap, &te32))
        {
            CloseHandle(hThreadSnap);     // 必须在使用后清除快照对象!
            return;
        }

        HMODULE hNtdll = LoadLibraryW(L"ntdll.dll");
        NTQUERYINFORMATIONTHREAD NtQueryInformationThread = NULL;
        NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)GetProcAddress(hNtdll, "NtQueryInformationThread");
        auto hProc = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
        do
        {

            if (te32.th32OwnerProcessID == pid)
            {
                uint64_t startaddr{};
                DWORD retlen{};
                char path[MAX_PATH]{};
                auto hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
                if (hThread)
                {
                    NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &startaddr, sizeof(startaddr), &retlen);
                    if (startaddr > 0)
                    {
                        MEMORY_BASIC_INFORMATION m{};
                        VirtualQueryEx(hProc, (PVOID)startaddr, &m, sizeof(MEMORY_BASIC_INFORMATION));
                        if (m.AllocationBase > 0)
                        {
                            GetModuleFileNameExA(hProc, (HMODULE)m.AllocationBase, path, MAX_PATH);
                        }

                    }
                    CloseHandle(hThread);
                }
                items.push_back(ThreadItem(te32.th32ThreadID, te32.tpDeltaPri, startaddr, path));
            }

        } while (Thread32Next(hThreadSnap, &te32));
        CloseHandle(hProc);
        CloseHandle(hThreadSnap);
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
            case MODULEPATH:                delta = (strcmp(a->module_path_.c_str() ,b->module_path_.c_str()));                                    break;
            default: IM_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }

        return (a->threadid_ - b->threadid_);
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
    std::string module_path_;

};

const ImGuiTableSortSpecs* ThreadItem::s_current_sort_specs_ = nullptr;