#pragma once
#include <stdint.h>

class ScanHooksWidget
{
public:

public:
	void OnPaint();

	void SetDataSource(uint32_t pid);

	void Load();

private:
	/*std::vector<ThreadItem> DataSource_{};*/
	int selected_ = -1;
	bool show_;

};