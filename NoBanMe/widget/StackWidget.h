#pragma once
#include <vector>
#include <stdint.h>
#include "../module/module.h"

class StackWidget
{
public:
	void OnPaint();

	void SetDataSource(uint32_t pid, std::vector<uint8_t> data, uint64_t rsp);

private:
	int selected_ = -1;
	uint32_t pid_;
	std::vector<uint8_t> DataSource_;
	std::vector<ModuleItem> mitems_;
	uint64_t rsp_;
	
};