#include "debugger.h"
#include "../../common/zydis/zydis_wrapper.h"
#include "../../common/utils/utils.h"

EXTERN_C ZyanStatus ZydisFormatterInit(ZydisFormatter* formatter, ZydisFormatterStyle style);

Debugger::DbgInfo Debugger::dbgInfo[4] = {};
std::vector<std::vector<std::string>> Debugger::Disassembly(bool Is64, uint64_t start, const uint8_t* buffer, size_t size)
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

    } while (i < size);

    return ret;
}

Debugger::DbgInfo Debugger::GetDbgInfo(uint8_t i)
{
    if (i >= 0 && i < 4)
        return dbgInfo[i];
    return DbgInfo{};
}

Debugger::DbgInfo& Debugger::GetDbgInfoRef(uint8_t i)
{
    DbgInfo info{};
    if (i >= 0 && i < 4)
        return dbgInfo[i];
    return info;
}
