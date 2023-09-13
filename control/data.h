#pragma once
namespace data
{
    namespace global
    {
        pipe pipe_;
        bool inject;
        uint32_t target;
        uint64_t fnDispatch;
        char plugin_path[256];
    }

    namespace Syscall
    {
        std::map<std::string, std::map<std::string, uint64_t>> monitor{};
    }

    namespace dbg
    {
        typedef struct _TDBG_CAPTURE_INFO
        {
            DbgBreakInfo dbginfo;
            std::string text;
            uint64_t count;
            std::vector<std::vector<std::string>> disamSeg;
        }DbgCaptureInfo, * PDbgCaptureInfo;

        typedef struct _TMEM_REGION
        {
            uint64_t start;
            std::vector<uint8_t> data;
        }MemRegion, * PMemRegion;

        typedef struct _TDBG_INFO
        {
            std::map<uint64_t, DbgCaptureInfo> capture;
            CONTEXT ctx;
            std::vector<std::vector<std::string>> disassembly{};
            MemRegion memoryRegion;
        }DbgInfo;

        DbgInfo dbgInfo[4];
    }

    namespace process
    {
        enum processinfo
        {
            PID,
            NAME,
            PPID,
            FULLPATH,
            STARTUPTIME
        };

        enum moduleinfo
        {
            IMAGEPATH,
            BASE,
            SIZE
        };

        enum threadinfo
        {
            THREAID,
            PRIORITY,
            STARTADDR
        };
        struct ProcessItem
        {
            ID3D11ShaderResourceView* icon;
            uint32_t pid;
            std::string name;
            uint32_t ppid;
            std::string fullpath;
            std::string startuptime;

            static const ImGuiTableSortSpecs* s_current_sort_specs_process;

            static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ProcessItem* items, int items_count)
            {
                s_current_sort_specs_process = sort_specs; // Store in variable accessible by the sort function.
                if (items_count > 1)
                    qsort(items, (size_t)items_count, sizeof(items[0]), ProcessItem::CompareWithSortSpecs);
                s_current_sort_specs_process = NULL;
            }

            static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs)
            {
                const ProcessItem* a = (const ProcessItem*)lhs;
                const ProcessItem* b = (const ProcessItem*)rhs;
                for (int n = 0; n < s_current_sort_specs_process->SpecsCount; n++)
                {
                    // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                    // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                    const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_process->Specs[n];
                    int delta = 0;
                    switch (sort_spec->ColumnUserID)
                    {
                    case PID:               delta = (a->pid - b->pid);                                          break;
                    case NAME:              delta = (strcmp(a->name.c_str(), b->name.c_str()));                 break;
                    case PPID:              delta = (a->ppid - b->ppid);                                        break;
                    case FULLPATH:          delta = (strcmp(a->fullpath.c_str(), b->fullpath.c_str()));         break;
                    case STARTUPTIME:       delta = (strcmp(a->startuptime.c_str(), b->startuptime.c_str()));   break;
                    default: IM_ASSERT(0); break;
                    }
                    if (delta > 0)
                        return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
                    if (delta < 0)
                        return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
                }

                return (a->pid - b->pid);
            }
        };
        struct ModuleItem
        {
            std::string imagepath;
            uint64_t base;
            size_t size;


            static const ImGuiTableSortSpecs* s_current_sort_specs_module;

            static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ModuleItem* items, int items_count)
            {
                s_current_sort_specs_module = sort_specs; // Store in variable accessible by the sort function.
                if (items_count > 1)
                    qsort(items, (size_t)items_count, sizeof(items[0]), ModuleItem::CompareWithSortSpecs);
                s_current_sort_specs_module = NULL;
            }

            static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs)
            {
                const ModuleItem* a = (const ModuleItem*)lhs;
                const ModuleItem* b = (const ModuleItem*)rhs;
                for (int n = 0; n < s_current_sort_specs_module->SpecsCount; n++)
                {
                    // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                    // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                    const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_module->Specs[n];
                    int delta = 0;
                    switch (sort_spec->ColumnUserID)
                    {
                    case IMAGEPATH:         delta = (strcmp(a->imagepath.c_str(), b->imagepath.c_str()));   break;
                    case BASE:              delta = (a->base, b->base);                                      break;
                    case SIZE:              delta = (a->size - b->size);                                    break;
                    default: IM_ASSERT(0); break;
                    }
                    if (delta > 0)
                        return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
                    if (delta < 0)
                        return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
                }

                return (strcmp(a->imagepath.c_str(), b->imagepath.c_str()));
            }
        };
        struct ThreadItem
        {
            uint32_t threadid;
            uint32_t DeltaPri;
            uint64_t startAddr;


            static const ImGuiTableSortSpecs* s_current_sort_specs_thread;

            static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ThreadItem* items, int items_count)
            {
                s_current_sort_specs_thread = sort_specs; // Store in variable accessible by the sort function.
                if (items_count > 1)
                    qsort(items, (size_t)items_count, sizeof(items[0]), ThreadItem::CompareWithSortSpecs);
                s_current_sort_specs_thread = NULL;
            }

            static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs)
            {
                const ThreadItem* a = (const ThreadItem*)lhs;
                const ThreadItem* b = (const ThreadItem*)rhs;
                for (int n = 0; n < s_current_sort_specs_thread->SpecsCount; n++)
                {
                    // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                    // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                    const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_thread->Specs[n];
                    int delta = 0;
                    switch (sort_spec->ColumnUserID)
                    {
                    case THREAID:                   delta = (a->threadid-b->threadid);   break;
                    case PRIORITY:                  delta = (a->DeltaPri- b->DeltaPri);                                      break;
                    case STARTADDR:                 delta = (a->startAddr-b->startAddr);                                    break;
                    default: IM_ASSERT(0); break;
                    }
                    if (delta > 0)
                        return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
                    if (delta < 0)
                        return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
                }

                return (a->threadid - b->threadid);
            }
        };
        const ImGuiTableSortSpecs* ProcessItem::s_current_sort_specs_process = NULL;
        const ImGuiTableSortSpecs* ModuleItem::s_current_sort_specs_module = NULL;
        const ImGuiTableSortSpecs* ThreadItem::s_current_sort_specs_thread = NULL;
    }
}
