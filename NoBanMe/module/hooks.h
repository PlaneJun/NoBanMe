#pragma once 
#include <vector>
#include <string>
#include <imgui.h>

class HookItem
{
public:
	enum EInfo
	{
		ADDRESS,
		TYPE,
		ORIGINCODE,
		CURTCODE,
		TIP
	};

	enum EHOOK_TYPE : uint8_t
	{
		EAT,
		IAT,
		Inline,
		Unkown
	};

public:
	HookItem();

	static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, HookItem* items, int items_count);

	static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs);

	static void SetSortSpecs(ImGuiTableSortSpecs* a);

private:
	static const ImGuiTableSortSpecs* s_current_sort_specs_;

	std::string address_;
	EHOOK_TYPE type_;
	uint64_t curt_address_;
	uint64_t origin_address_;
	std::vector<uint8_t> curt_opcode_;
	std::vector<uint8_t> origin_opcode_;
};