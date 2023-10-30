
#include <filesystem>
#include "pdb/EasyPdb.h"
#include "ntStruct.h"
#include "widget/MainWidget.h"
#include "render/render.h"

MainWidget g_main_widget;
void IPC_Loop(DWORD* a)
{
    g_main_widget.OnIPC();
};

void Update_Loop(DWORD* a)
{
    g_main_widget.OnUpdate();
};

void Init(float w, float h)
{
    static bool init = false;
    if (!init)
    {
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)IPC_Loop, NULL, NULL, NULL);
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Update_Loop, NULL, NULL, NULL);
        init = true;
    }
    g_main_widget.SetWindowSize(w, h);
    g_main_widget.OnPaint();
}

int main()
{
    std::filesystem::path data_dir("./Data");
    if (!std::filesystem::exists(data_dir))
        std::filesystem::create_directory(data_dir);

    //初始化pdb
    char nt_path[MAX_PATH]{};
    GetModuleFileNameA(GetModuleHandleA("ntdll.dll"), nt_path, MAX_PATH);
    EasyPdb easyPdb;
    std::string pdbPath = easyPdb.EzPdbDownload(nt_path);
    if (pdbPath.empty())
    {
        MessageBoxA(NULL, "Download Pdb Error!", "PJArk", NULL);
        return 1;
    }
    EasyPdb::EZPDB pdb;
    if (!easyPdb.EzPdbLoad(pdbPath, &pdb))
    {
        MessageBoxA(NULL, "Load Pdb Failed!", "PJArk", NULL);
        return 1;
    }
    NT_Normal::LdrpVectorHandlerList = easyPdb.EzPdbGetRva(&pdb, "LdrpVectorHandlerList");
    if (NT_Normal::LdrpVectorHandlerList <= 0)
    {
        MessageBoxA(NULL, "Get rva LdrpVectorHandlerList Failed!", "PJArk", NULL);
        return 1;
    }
    VehHandlerItem::SetLdrpVectorHandlerList(NT_Normal::LdrpVectorHandlerList);
    easyPdb.EzPdbUnload(&pdb);

    render::get_instasnce()->CreatGui(L"PJArk", L"CPJArk", 1440, 900, Init);
}
