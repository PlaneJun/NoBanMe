#pragma once
class Debugger
{
public:
    enum
    {
        DISAM_BLOCK=0x1000
    };

    typedef struct _THARD_BREAK_INFO
    {
        EDataType type;
        uint8_t id;
        uint64_t addr;
        CONTEXT ctx;
        uint8_t stack[4096]; //4kb stack
        char disassembly[256];
    }DbgBreakInfo, * PDbgBreakInfo;

    typedef struct _TDBG_CAPTURE_INFO
    {
        DbgBreakInfo dbginfo;
        std::string text;
        uint64_t count;
        uint8_t stack[4096]; //4kb stack
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
        MemRegion memoryRegion;
    }DbgInfo;

public:
    static std::vector<std::vector<std::string>> Disassembly(bool Is64,uint64_t start, const uint8_t* buffer,size_t size )
    {
        std::vector<std::vector<std::string>> ret;
        std::vector<std::string> tmp{};

        int i = 0;
        Zydis dism(Is64);
        do
        {
            tmp.clear();
            bool status = dism.Disassemble(start + i, buffer + i);
            if (status)
            {
                tmp.push_back(utils::conver::IntegerTohex(start + i));
                tmp.push_back(utils::conver::bytesToHexString(buffer + i, dism.Size()).c_str());
                tmp.push_back(dism.InstructionText().c_str());
                ret.push_back(tmp);
                i += dism.Size();
            }
            else
            {
                tmp.push_back(utils::conver::IntegerTohex(start + i));
                tmp.push_back(utils::conver::bytesToHexString(buffer + i, 1));
                tmp.push_back("???");
                ret.push_back(tmp);
                i++;
            }

        }while (i< size);

        return ret;
    }

    static auto GetDbgInfo(uint8_t i)
    {
        if (i >= 0 && i < 4)
            return dbgInfo[i];
        return DbgInfo{};
    }

    static auto& GetDbgInfoRef(uint8_t i)
    {
        DbgInfo info{};
        if (i >= 0 && i < 4)
            return dbgInfo[i];
        return info;
    }

private:
    static DbgInfo dbgInfo[4];
};

Debugger::DbgInfo Debugger::dbgInfo[4] = {};