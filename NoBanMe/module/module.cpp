#include "module.h"
#include <Windows.h>
#include <tlhelp32.h>
#include "../utils/utils.h"

const ImGuiTableSortSpecs* ModuleItem::s_current_sort_specs_ = nullptr;

ModuleItem::ModuleItem()
{
    this->imagepath_ = std::string();
    this->base_ = 0;
    this->size_ = 0;
    this->fileversion_ = std::string();
    this->companyname_ = std::string();
    this->description_ = std::string();
}

ModuleItem::ModuleItem(std::string imagepath,uint64_t base,size_t size,std::string descripttion, std::string fileversion, std::string companyname)
{
    this->imagepath_ = imagepath;
    this->base_ = base;
    this->size_ = size;
    this->fileversion_ = fileversion;
    this->companyname_ = companyname;
    this->description_ = descripttion;
}

std::string ModuleItem::GetImagePath()
{
    return imagepath_;
}

uint64_t ModuleItem::GetBase()
{
    return base_;
}

size_t ModuleItem::GetSize()
{
    return size_;
}

std::string ModuleItem::GetFileVersion()
{
    return fileversion_;
}

std::string ModuleItem::GetDecription()
{
    return description_;
}

std::string ModuleItem::GetCompanyName()
{
    return companyname_;
}

void ModuleItem::EnumPidModules(uint32_t pid, std::vector<ModuleItem>& items)
{
    HANDLE hSnapshot_proc = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (hSnapshot_proc)
    {
        MODULEENTRY32  me32 = { sizeof(MODULEENTRY32) };
        bool bprocess = Module32First(hSnapshot_proc, &me32);
        while (bprocess)
        {
            std::string imagepath = utils::conver::wstring_to_stirng(me32.szExePath);
            items.push_back(ModuleItem(imagepath, (uint64_t)me32.modBaseAddr, me32.modBaseSize, utils::file::GetFileDescription(imagepath), utils::file::GetFileVersion(imagepath), utils::file::GetProductName(imagepath)));
            bprocess = Module32Next(hSnapshot_proc, &me32);
        }
        CloseHandle(hSnapshot_proc);
    }
}

void ModuleItem::SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ModuleItem* items, int items_count)
{
    s_current_sort_specs_ = sort_specs; // Store in variable accessible by the sort function.
    if (items_count > 1)
        qsort(items, (size_t)items_count, sizeof(items[0]), ModuleItem::CompareWithSortSpecs);
    s_current_sort_specs_ = NULL;
}

int __cdecl ModuleItem::CompareWithSortSpecs(const void* lhs, const void* rhs)
{
    const ModuleItem* a = (const ModuleItem*)lhs;
    const ModuleItem* b = (const ModuleItem*)rhs;
    for (int n = 0; n < s_current_sort_specs_->SpecsCount; n++)
    {
        // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
        // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
        const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_->Specs[n];
        int delta = 0;
        switch (sort_spec->ColumnUserID)
        {
        case IMAGEPATH:         delta = (strcmp(a->imagepath_.c_str(), b->imagepath_.c_str()));   break;
        case BASE:              delta = (a->base_- b->base_);                                      break;
        case SIZE:              delta = (a->size_ - b->size_);                                    break;
        default: IM_ASSERT(0); break;
        }
        if (delta > 0)
            return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
        if (delta < 0)
            return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
    }

    return (strcmp(a->imagepath_.c_str(), b->imagepath_.c_str()));
}

void ModuleItem::SetSortSpecs(ImGuiTableSortSpecs* a)
{
    s_current_sort_specs_ = a;
}

