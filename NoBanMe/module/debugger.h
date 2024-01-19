#pragma once
#include <vector>
#include <string>
#include <map>
#include <Windows.h>
#include <stdint.h>
#include "../../common/nativeStruct.h"
#include "../../common/ipc_ctrl.h"

class Debugger
{
public:
    enum
    {
        DISAM_BLOCK = 0x1000
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
    static std::vector<std::vector<std::string>> Disassembly(bool Is64, uint64_t start, const uint8_t* buffer, size_t size);

    static DbgInfo GetDbgInfo(uint8_t i);

    static DbgInfo& GetDbgInfoRef(uint8_t i);

private:
    static DbgInfo dbgInfo[4];

};

