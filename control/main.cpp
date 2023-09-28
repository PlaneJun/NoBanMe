#include "pch.h"


MainWidget mainWidget;

void IPC_Loop (DWORD* a)
{
    mainWidget.OnIPC();
};

void Update_Loop (DWORD* a)
{
    mainWidget.OnUpdate();
};

void Init(float w,float h)
{
    static bool init = false;
    if (!init)
    {
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)IPC_Loop, NULL, NULL, NULL);
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Update_Loop, NULL, NULL, NULL);
        init = true;
    }
    mainWidget.SetWindowSize(w,h);
    mainWidget.OnPaint();
}


BOOL CALLBACK RetrieveWndCallback(HWND wnd, LPARAM param)
{
    DWORD pid;
    std::vector<HWND>& wnds = *(std::vector<HWND>*)param;
    wnds.push_back(wnd);
    return TRUE;
}

std::vector<HWND> GetSystemWnds()
{
    std::vector<HWND> wnds;
    EnumChildWindows(GetDesktopWindow(), (WNDENUMPROC)RetrieveWndCallback, (LPARAM)&wnds);
    return wnds;
}




int main()
{
    //检查目录是否存在
    std::filesystem::path data_dir("./Data");
    if(!std::filesystem::exists(data_dir))
        std::filesystem::create_directory(data_dir);

    //初始化pdb
    char nt_path[MAX_PATH]{};
    GetModuleFileNameA(GetModuleHandleA("ntdll.dll"), nt_path, MAX_PATH);
    std::string pdbPath = EasyPdb::EzPdbDownload(nt_path);
    if (pdbPath.empty())
    {
        MessageBoxA(NULL,"Download Pdb Error!","PJArk",NULL);
        return 1;
    }
    EasyPdb::EZPDB pdb;
    if (!EasyPdb::EzPdbLoad(pdbPath, &pdb))
    {
        MessageBoxA(NULL, "Load Pdb Failed!", "PJArk", NULL);
        return 1;
    }
    auto rva_LdrpVectorHandlerList = EzPdbGetRva(&pdb, "LdrpVectorHandlerList");
    if (rva_LdrpVectorHandlerList <= 0)
    {
        MessageBoxA(NULL, "Get rva_LdrpVectorHandlerList Failed!", "PJArk", NULL);
        return 1;
    }
    VehHandlerItem::SetLdrpVectorHandlerList(rva_LdrpVectorHandlerList);
    EasyPdb::EzPdbUnload(&pdb);

    //auto hv = GetSystemWnds();
    render::get_instasnce()->CreatGui(L"PJArk", L"CPJArk", 1440, 900, Init);
    return 0;
}
