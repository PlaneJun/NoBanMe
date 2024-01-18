#include "hooks.h"

const ImGuiTableSortSpecs* HookItem::s_current_sort_specs_ = nullptr;

HookItem::HookItem()
{
	address_ = std::string();
	type_ = HookItem::EHOOK_TYPE::Unkown;
	curt_address_=0;
	origin_address_ = 0;

}

void HookItem::SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, HookItem* items, int items_count)
{
	s_current_sort_specs_ = sort_specs; // Store in variable accessible by the sort function.
	if (items_count > 1)
		qsort(items, (size_t)items_count, sizeof(items[0]), HookItem::CompareWithSortSpecs);
	s_current_sort_specs_ = NULL;
}

int __cdecl HookItem::CompareWithSortSpecs(const void* lhs, const void* rhs)
{
	const HookItem* a = (const HookItem*)lhs;
	const HookItem* b = (const HookItem*)rhs;
	for (int n = 0; n < s_current_sort_specs_->SpecsCount; n++)
	{
		// Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
		// We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
		const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_->Specs[n];
		int delta = 0;
		switch (sort_spec->ColumnUserID)
		{
			case ADDRESS:              delta = strcmp(a->address_.c_str(),b->address_.c_str());                                      break;
			case TYPE:              delta = (a->type_ - b->type_);                                    break;
			default: IM_ASSERT(0); break;
		}
		if (delta > 0)
			return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
		if (delta < 0)
			return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
	}

	return strcmp(a->address_.c_str(), b->address_.c_str());
}

void HookItem::SetSortSpecs(ImGuiTableSortSpecs* a)
{
	s_current_sort_specs_ = a;
}
