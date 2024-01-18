#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include <imgui.h>
#include <D3DX11tex.h>

class ProcessItem
{
public:
    enum EInfo
    {
        PID,
        NAME,
        PPID,
        FULLPATH,
        STARTUPTIME
    };

public:
    ProcessItem();

    ProcessItem(ID3D11ShaderResourceView* icon, uint32_t pid, std::string name, uint32_t ppid, std::string fullpath, std::string startuptime,
        bool isWow64, std::string descripttion, std::string fileversion, std::string companyname, std::string cmdline);

    ID3D11ShaderResourceView* GetIcon();

    uint32_t GetPid();

    std::string GetName();

    uint32_t GetPPid();

    std::string GetFullPath();

    std::string GetStartUpTime();

    std::string GetFileVersion();

    std::string GetDecription();

    std::string GetCompanyName();

    bool IsWow64();

    std::string GetCmdLine();

public:

    static void EnumCurtAllProcess(std::vector<ProcessItem>& items);

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, ProcessItem* items, int items_count);

    static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs);

    static void SetSortSpecs(ImGuiTableSortSpecs* a);

private:
    static const ImGuiTableSortSpecs* s_current_sort_specs_;
    ID3D11ShaderResourceView* icon_;
    uint32_t pid_;
    std::string name_;
    uint32_t ppid_;
    std::string fullpath_;
    std::string startuptime_;
    std::string fileversion_;
    std::string companyname_;
    std::string description_;
    std::string cmdline_;
    bool bWow64;

};

