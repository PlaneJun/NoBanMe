#pragma once
class Debugger
{
public:
    typedef struct _THARD_BREAK_INFO
    {
        EDataType type;
        uint8_t id;
        uint64_t addr;
        CONTEXT ctx;
        uint8_t stack[1024]; //1mb stack
        uint64_t region_start;
        uint64_t region_size;
        char disassembly[256];
    }DbgBreakInfo, * PDbgBreakInfo;

    typedef struct _TDBG_CAPTURE_INFO
    {
        DbgBreakInfo dbginfo;
        std::string text;
        uint64_t count;
        std::vector<std::vector<std::string>> disamSeg;
        uint8_t stack[1024]; //1mb stack
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

public:
    static std::vector<std::vector<std::string>> Disassembly(uint64_t start, size_t size, const uint8_t* buffer)
    {
        std::vector<std::vector<std::string>> ret;
        std::vector<std::string> tmp;
        ZydisDisassembledInstruction instruction;
        ZyanU64 runtime_address = (ZyanU64)start;
        for (int i = 0; i < size; )
        {
            tmp.clear();
            if (ZYAN_SUCCESS(ZydisDisassembleIntel(
                /* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
                /* runtime_address: */ runtime_address,
                /* buffer:          */ buffer + i,
                /* length:          */ size - i,
                /* instruction:     */ &instruction
            )))
            {
                tmp.push_back("0x" + utils::conver::IntegerTohex(runtime_address));
                tmp.push_back(utils::conver::bytesToHexString(buffer + i, instruction.info.length).c_str());
                tmp.push_back(instruction.text);
                ret.push_back(tmp);
                i += instruction.info.length;
                runtime_address += instruction.info.length;
            }
            else
            {
                tmp.push_back("0x" + utils::conver::IntegerTohex(runtime_address));
                tmp.push_back(utils::conver::bytesToHexString(buffer + i, 1).c_str());
                tmp.push_back("db");
                ret.push_back(tmp);
                i++;
                runtime_address += i;
            }
        }

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