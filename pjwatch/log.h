#pragma once

enum LOG_TYPE
{
    LOG_SYSCALL,
    LOG_HARDBREAK,
    LOG_UNKNOWN,
    LOG_NUM
};

class LogPrinter
{
public:

    LogPrinter()
    {
        refreshFileName();
    }

    void ShowOnConsole()
    {
        if (!bConsole)
        {
            bConsole = true;
            AllocConsole();
            freopen_s((_iobuf**)__acrt_iob_func(0), "conin$", "r", (_iobuf*)__acrt_iob_func(0));
            freopen_s((_iobuf**)__acrt_iob_func(1), "conout$", "w", (_iobuf*)__acrt_iob_func(1));
            freopen_s((_iobuf**)__acrt_iob_func(2), "conout$", "w", (_iobuf*)__acrt_iob_func(2));
        }
        
        bShowOnConsole_ = true;
    }

    void ShowOffConsole()
    {
        bShowOnConsole_ = false;
    }

    void Print(LOG_TYPE printLev, const char* format, ...)
    {
        if (printLev < 0 || printLev >= LOG_NUM)
            printLev = LOG_NUM;

        logFileSizeCtl();
        prevInfoPrint(printLev);

        va_list args;
        va_start(args, format);

        if (bShowOnConsole_)
        {
            vprintf(format, args);
            printf("\n");
        }

        FILE* fp;
        fopen_s(&fp, filename_, "a+");
        if (!fp) 
        {
            return;
        }

        vfprintf(fp, format, args);
        fwrite("\n", 1, 1, fp);
        fclose(fp);
    }

private:
    bool bConsole;
    bool bShowOnConsole_;
    char filename_[256];
    enum
    {
        LOG_FILE_MAX_SIZE=60 * 1024 * 1024 // 单文件大小限制:60MB
    };
private:

    void refreshFileName()
    {
        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now);
        sprintf_s(filename_, "[pjwatch]%d-%02d-%02dT%2d-%02d-%02d.log",
            ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday,
            ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    }

    void prevInfoPrint(LOG_TYPE printLev)
    {
        char buf[128];
        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now);

        const std::string logType[LOG_NUM] =
        {
            "Syscall",
            "HardBreak",
            "Unknown",
        };

        sprintf_s(buf, "[%d/%02d/%02d %2d:%02d:%02d] [%s]: ",
            ltm.tm_year + 1900, ltm.tm_mon + 1, ltm.tm_mday,
            ltm.tm_hour, ltm.tm_min, ltm.tm_sec,
            logType[printLev].data());

        if (bShowOnConsole_)
        {
            printf("%s", buf);
        }
        
        FILE* fp = nullptr;
        fopen_s(&fp, filename_, "a+");
        if (!fp) 
        {
            return;
        }

        fwrite(buf, sizeof(char), strlen(buf), fp);
        fclose(fp);
    }

    int GetFileSize() 
    {
        struct stat statbuf;
        stat(filename_, &statbuf);
        return statbuf.st_size;
    }

    void logFileSizeCtl() 
    {
        if (GetFileSize() < LOG_FILE_MAX_SIZE)
        {
            return;
        }
        refreshFileName();
    }
};

