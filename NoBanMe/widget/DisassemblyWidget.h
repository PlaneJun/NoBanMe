#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include "../module/process.h"

class DisassemblyWidget
{
public:
    int GetSelected();

    void SetData(ProcessItem proc, uint64_t jmpto);

    void OnPaint();

private:
    ProcessItem DataSource_;
    int selected_ = -1;
    std::vector<std::vector<std::string>> dissambly_block_{};
    uint64_t dissambly_base_;
    uint64_t dissambly_jmp_;
    uint32_t curt_page_index_;
    uint32_t max_page_index_;
    uint8_t wheel_count_; //ÂÖ¶¯¼ÇÂ¼5´Î·­Ò³
    std::string curtModule;
};