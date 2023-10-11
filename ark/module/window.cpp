#include "window.h"

const ImGuiTableSortSpecs* WindowItem::s_current_sort_specs_ = nullptr;
WindowItem::WindowItem()
{
	this->text_ = std::string();
	this->type_ = NULL;
	this->title_ = std::string();
	this->classname_ = std::string();
	this->procName_ = std::string();
	this->style_ = NULL;
	this->styleEx_ = NULL;
	this->rectRelativa_ = { 0 };
	this->rectScreen_ = { 0 };
	this->pid_ = NULL;
	this->thread_ = NULL;
	this->hwnd_ = NULL;
	this->hwndParen_ = NULL;
	this->icon_ = NULL;
	this->callback_ = NULL;
}

WindowItem::WindowItem(
	std::string text,
	uint32_t type,
	std::string title,
	std::string classname,
	std::string procName,
	uint32_t style,
	uint32_t styleEx,
	RECT rectRelativa,
	RECT rectScreen,
	uint32_t pid,
	uint32_t thread,
	::HWND hwnd,
	::HWND hwndParen,
	uintptr_t callback)
{
	this->text_ = text;
	this->type_ = type;
	this->title_ = title;
	this->classname_ = classname;
	this->procName_ = procName;
	this->style_ = style;
	this->styleEx_ = styleEx;
	this->rectRelativa_ = rectRelativa;
	this->rectScreen_ = rectScreen;
	this->pid_ = pid;
	this->thread_ = thread;
	this->hwnd_ = hwnd;
	this->hwndParen_ = hwndParen;
	this->callback_ = callback;
}

void WindowItem::SetRectRelativa(RECT r)
{
	rectRelativa_ = r;
}

RECT WindowItem::GetRectRelativa()
{
	return rectRelativa_;
}

void WindowItem::SetRectScreen(RECT r)
{
	rectScreen_ = r;
}

RECT WindowItem::GetRectScreen()
{
	return rectScreen_;
}

void WindowItem::SetProcessName(std::string fullname)
{
	int index = fullname.rfind("\\");
	if (index != std::string::npos)
		index++;
	else
		index = 0;
	procName_ = fullname.substr(index);
}

std::string WindowItem::GetProcessName()
{
	return procName_;
}

void WindowItem::SetHwnd(::HWND hwnd)
{
	hwnd_ = hwnd;
}

::HWND WindowItem::GetHwnd()
{
	return hwnd_;
}

void WindowItem::SetParent(::HWND hwnd)
{
	hwndParen_ = hwnd;
}

::HWND WindowItem::GetParent()
{
	return hwndParen_;
}

void WindowItem::SetTitle(const char* title)
{
	title_ = title;
}

std::string WindowItem::GetTitle()
{
	return title_;
}

void WindowItem::SetKlassName(const char* klass)
{
	classname_ = klass;
}

std::string WindowItem::GetKlassName()
{
	return classname_;
}

std::string WindowItem::GetText()
{
	char buf[1024]{};
	if (!title_.empty())
		title_ = "[" + title_ + "]";
	sprintf_s(buf, "0x%zX (%s) %s", hwnd_, classname_.c_str(), title_.c_str());
	return buf;
}

void WindowItem::SetIcon(ID3D11ShaderResourceView* icon)
{
	icon_ = icon;
}

ID3D11ShaderResourceView* WindowItem::GetIcon()
{
	return icon_;
}

void WindowItem::SetStyle(int style)
{
	style_ = style;
}

int WindowItem::GetStyle()
{
	return style_;
}

void WindowItem::SetStyleEx(int styleEx)
{
	styleEx_ = styleEx;
}

int WindowItem::GetStyleEx()
{
	return styleEx_;
}

void WindowItem::SetWndCb(uintptr_t cb)
{
	callback_ = cb;
}

uintptr_t WindowItem::GetWndCb()
{
	return callback_;
}

void WindowItem::SetPid(uint32_t pid)
{
	pid_ = pid;
}

uint32_t WindowItem::GetPid()
{
	return pid_;
}

void WindowItem::SetThreadId(uint32_t tid)
{
	thread_ = tid;
}

uint32_t WindowItem::GetThreadId()
{
	return thread_;
}

void WindowItem::SetSortSpecs(ImGuiTableSortSpecs* a)
{
	s_current_sort_specs_ = a;
}

void WindowItem::SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, WindowItem* items, int items_count)
{
	s_current_sort_specs_ = sort_specs; // Store in variable accessible by the sort function.
	if (items_count > 1)
		qsort(items, (size_t)items_count, sizeof(items[0]), WindowItem::CompareWithSortSpecs);
	s_current_sort_specs_ = NULL;
}

int __cdecl WindowItem::CompareWithSortSpecs(const void* lhs, const void* rhs)
{
	const WindowItem* a = (const WindowItem*)lhs;
	const WindowItem* b = (const WindowItem*)rhs;
	for (int n = 0; n < s_current_sort_specs_->SpecsCount; n++)
	{
		// Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
		// We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
		const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs_->Specs[n];
		int delta = 0;
		switch (sort_spec->ColumnUserID)
		{
			case TITLE:                   delta = (strcmp(a->title_ .c_str(), b->title_.c_str()));   break;
			case CLASSNAME:                  delta = (strcmp(a->classname_.c_str(), b->classname_.c_str()));                                      break;
			case STYLE:                 delta = (a->style_ - b->style_);                                    break;
			case STYLEEX:                delta = (a->styleEx_ - b->styleEx_);                                    break;
			case HWND:                delta = (a->hwnd_ - b->hwnd_);                                   break;
			case PHWND:                delta = (a->hwndParen_ - b->hwndParen_);                                    break;
			case TID:                delta = (a->thread_ - b->thread_);                                    break;
			default: IM_ASSERT(0); break;
		}
		if (delta > 0)
			return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
		if (delta < 0)
			return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
	}

	return (a->hwnd_ - b->hwnd_);
}

void WindowItem::Show()
{
	if (!hwnd_ || !::IsWindow(hwnd_))
		return;

	::HWND hWndCurrentWindow = GetForegroundWindow();
	if (hWndCurrentWindow == NULL)
	{
		SetForegroundWindow(hwnd_);
		return;
	}

	DWORD dwCurProcId, dwCurThreadId = ::GetWindowThreadProcessId(hWndCurrentWindow, &dwCurProcId);
	DWORD dwHWndProcId;
	::GetWindowThreadProcessId(hwnd_, &dwHWndProcId);

	BOOL bReturn = FALSE;

	if (dwHWndProcId == dwCurProcId)
	{
		SetForegroundWindow(hwnd_);
	}
	else
	{
		DWORD dwMyThreadId = GetCurrentThreadId();

		if (AttachThreadInput(dwMyThreadId, dwCurThreadId, TRUE))
		{
			SetForegroundWindow(hwnd_);
			AttachThreadInput(dwMyThreadId, dwCurThreadId, FALSE);
		}
		else
		{
			SetForegroundWindow(hwnd_);
		}
	}
}
