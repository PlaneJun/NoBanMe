#pragma once
#include <vector>
#include <string>
#include <Windows.h>
#include <stdint.h>
#include <imgui.h>

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

    VehHandlerItem();

    VehHandlerItem(uint64_t entry, std::string module_path, bool isVeh);

    uint64_t GetEntry();

    std::string GetModulePath();

    bool IsVeh();

public:

    static void EnumVehHandler(uint32_t pid, std::vector<VehHandlerItem>& items);

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, VehHandlerItem* items, int items_count);

    static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs);

    static void SetSortSpecs(ImGuiTableSortSpecs* a);

    static void SetLdrpVectorHandlerList(uint64_t rva);

    static uint64_t GetLdrpVectorHandlerList();
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

private:
    static const ImGuiTableSortSpecs* s_current_sort_specs_;
    static uint64_t rva_LdrpVectorHandlerList_;
    uint64_t entry_;
    std::string module_path_;
    bool isVeh_;

};
