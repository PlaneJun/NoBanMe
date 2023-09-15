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
        TYPE,
        MODULEPATH
    };

public:

    VehHandlerItem(uint64_t entry, std::string module_path,bool isVeh)
    {
        this->entry_ = entry;
        this->module_path_ = module_path;
        this->isVeh_ = isVeh;
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

public:

    static void EnumVehHandler(uint32_t pid, std::vector<VehHandlerItem>& items)
    {
        static auto QueryList = [=](HANDLE process, uint64_t LdrpVectorHandlerList, BOOL exception, BOOL isWow64) {

            std::vector<VehHandlerItem> items{};

            PLIST_ENTRY startLink{};
            PLIST_ENTRY currentLink{};

            VehHandlerItem::RTL_VECTORED_HANDLER_LIST* vectorPtr = new VehHandlerItem::RTL_VECTORED_HANDLER_LIST();
            ReadProcessMemory(process, (PVOID)LdrpVectorHandlerList, vectorPtr, sizeof(VehHandlerItem::RTL_VECTORED_HANDLER_LIST), NULL);
            if (exception)
            {
                startLink = (PLIST_ENTRY)PTR_ADD_OFFSET((LPVOID)LdrpVectorHandlerList, UFIELD_OFFSET(VehHandlerItem::RTL_VECTORED_HANDLER_LIST, ExceptionList));;
                currentLink = vectorPtr->ExceptionList.Flink;
            }
            else {
                startLink = (PLIST_ENTRY)PTR_ADD_OFFSET((LPVOID)LdrpVectorHandlerList, UFIELD_OFFSET(VehHandlerItem::RTL_VECTORED_HANDLER_LIST, ContinueList));;
                currentLink = vectorPtr->ContinueList.Flink;
            }
            int i = 0;
            while (currentLink != startLink && i <= 40)
            {
                VehHandlerItem::PRTL_VECTORED_EXCEPTION_ENTRY addressOfEntry = new VehHandlerItem::RTL_VECTORED_EXCEPTION_ENTRY();
                ReadProcessMemory(process, CONTAINING_RECORD(currentLink, VehHandlerItem::RTL_VECTORED_EXCEPTION_ENTRY, List), addressOfEntry, sizeof(VehHandlerItem::RTL_VECTORED_EXCEPTION_ENTRY), NULL);
                typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess)(HANDLE ProcessHandle, int ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
                fnNtQueryInformationProcess lpNtQueryInformationProcess = (fnNtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
                uint64_t decode_ptr{};
                uint64_t key = (uint64_t)addressOfEntry->VectoredHandler;
                uint32_t ProcessInformation{};
                uint32_t ret = lpNtQueryInformationProcess(process, 36, &ProcessInformation, 4, 0);
                if (ret >= 0)
                {
                    if (isWow64)
                    {
                        decode_ptr = ProcessInformation ^ __ROR4__(key, 32 - (ProcessInformation & 0x1F));
                    }
                    else
                    {
                        decode_ptr = __ROR8__(key, 0x40 - (ProcessInformation & 0x3F)) ^ ProcessInformation;
                    }
                }

                MEMORY_BASIC_INFORMATION m = { 0 };
                VirtualQueryEx(process, (PVOID)decode_ptr, &m, sizeof(MEMORY_BASIC_INFORMATION));
                char module_path[MAX_PATH]{};
                if ((uint64_t)m.AllocationBase > 0)
                    GetModuleFileNameExA(process, (HMODULE)m.AllocationBase, module_path, MAX_PATH);
                items.push_back(VehHandlerItem(decode_ptr, module_path, exception));
                currentLink = addressOfEntry->List.Flink;

                i++;
            }

            return items;
        };


        BOOL bIsWow64 = false;
        auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (hProc)
        {
            IsWow64Process(hProc, &bIsWow64);
            auto veh = QueryList(hProc, VehHandlerItem::GetLdrpVectorHandlerList(), true, bIsWow64);
            for (auto v : veh)
                items.push_back(v);
            veh = QueryList(hProc, VehHandlerItem::GetLdrpVectorHandlerList(), false, bIsWow64);
            for (auto v : veh)
                items.push_back(v);
            CloseHandle(hProc);
        }
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
            case TYPE:                  delta = (a->isVeh_ - b->isVeh_);                                      break;
            default: IM_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }

        return (a->entry_ - b->entry_);
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

    static const ImGuiTableSortSpecs* s_current_sort_specs_;
    static uint64_t rva_LdrpVectorHandlerList_;
    uint64_t entry_;
    std::string module_path_;
    bool isVeh_;
};

const ImGuiTableSortSpecs* VehHandlerItem::s_current_sort_specs_ = nullptr;
uint64_t VehHandlerItem::rva_LdrpVectorHandlerList_ = NULL;