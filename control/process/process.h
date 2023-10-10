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
    ProcessItem()
    {
        this->icon_ = nullptr;
        this->pid_ = 0;
        this->name_ = std::string();
        this->ppid_ = 0;
        this->fullpath_ = std::string();
        this->startuptime_ = std::string();
        this->fileversion_ = std::string();
        this->companyname_ = std::string();
        this->description_ = std::string();
        this->bWow64 = false;
        this->cmdline_ = std::string();
    }

    ProcessItem(ID3D11ShaderResourceView* icon, uint32_t pid, std::string name,uint32_t ppid,std::string fullpath,std::string startuptime,
                bool isWow64,std::string descripttion,std::string fileversion,std::string companyname,std::string cmdline)
    {
        this->icon_ = icon;
        this->pid_ = pid;
        this->name_ = name;
        this->ppid_ = ppid;
        this->fullpath_ = fullpath;
        this->startuptime_ = startuptime;
        this->bWow64 = isWow64;
        this->fileversion_ = fileversion;
        this->companyname_ = companyname;
        this->description_ = descripttion;
        this->cmdline_ = cmdline;
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

    auto GetFileVersion()
    {
        return fileversion_;
    }

    auto GetDecription()
    {
        return description_;
    }

    auto GetCompanyName()
    {
        return companyname_;
    }

    auto IsWow64()
    {
        return bWow64;
    }

    auto GetCmdLine()
    {
        return cmdline_;
    }

public:

    static void EnumCurtAllProcess(std::vector<ProcessItem>& items)
    {
        HANDLE hSnapshot_proc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot_proc)
        {
            PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
            bool bprocess = Process32First(hSnapshot_proc, &pe);
            if (!bprocess)
                return;

            //创建保存icon的目录
            std::filesystem::path icon_dir("./Data/icon/");
            if (std::filesystem::exists(icon_dir))
                std::filesystem::remove_all(icon_dir);
            std::filesystem::create_directory(icon_dir);

            while (bprocess)
            {
                BOOL bIsWow64 = false;
                std::string fullPath=utils::conver::wstring_to_stirng(pe.szExeFile);
                char imagepath[MAX_PATH]{}, startuptime[MAX_PATH]{};
                auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
                if (hProc)
                {
                    //Get Module Path
                    GetModuleFileNameExA(hProc, NULL, imagepath, MAX_PATH);
                    //GetStartupTime
                    FILETIME ftCreation, ftExit, ftKernel, ftUser;
                    SYSTEMTIME stCreation, lstCreation;
                    if (GetProcessTimes(hProc, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
                        FileTimeToSystemTime(&ftCreation, &stCreation);
                        SystemTimeToTzSpecificLocalTime(NULL, &stCreation, &lstCreation);
                        sprintf_s(startuptime, "%04d/%02d/%02d %02d:%02d:%02d", lstCreation.wYear, lstCreation.wMonth, lstCreation.wDay, lstCreation.wHour, lstCreation.wMinute, lstCreation.wSecond);
                    }
                    IsWow64Process(hProc, &bIsWow64);

                    CloseHandle(hProc);
                }
                std::string proName(std::filesystem::path(fullPath).filename().string());
                ID3D11ShaderResourceView* icon = nullptr;
                auto hIcon = utils::image::GetProcessIcon(imagepath);
                if (hIcon != NULL && utils::image::SaveIconToPng(hIcon, ("./Data/icon/" + proName + ".png").c_str()))
                    icon = render::get_instasnce()->DX11LoadTextureImageFromFile(("./Data/icon/" + proName + ".png").c_str());

                auto cmd = utils::conver::wstring_to_stirng(utils::process::GetCommandLineW(pe.th32ProcessID));
                items.push_back(ProcessItem(icon, pe.th32ProcessID, proName, pe.th32ParentProcessID, imagepath, startuptime, bIsWow64, utils::file::GetFileDescription(imagepath), utils::file::GetFileVersion(imagepath), utils::file::GetProductName(imagepath), cmd));
                bprocess = Process32Next(hSnapshot_proc, &pe);
            }
            CloseHandle(hSnapshot_proc);
        }
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

const ImGuiTableSortSpecs* ProcessItem::s_current_sort_specs_=nullptr;
