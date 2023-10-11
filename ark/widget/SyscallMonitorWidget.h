#pragma once
#include <string>
#include <map>
#include <stdint.h>

class SysCallMonitorWidget
{
public:
    void OnPaint();

    bool GetActive();

    void  SetActive(bool b);

    bool GetSave();

    bool IsExsist(std::string key);

    void SetValue(std::string k1, std::string k2, uint64_t v, bool inc = false);

private:
    bool active_=false;
    bool save_=false;
    std::map<std::string, std::map<std::string, uint64_t>> monitor_{};

};