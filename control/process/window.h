#pragma once
class WindowItem
{
public:
	enum EInfo
	{
		TITLE,
		CLASSNAME,
		STYLE,
		STYLEEX,
		HWND,
		PHWND,
		TID
	};


	WindowItem()
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

	WindowItem(
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

	void SetRectRelativa(RECT r)
	{
		rectRelativa_ = r;
	}

	auto GetRectRelativa()
	{
		return rectRelativa_;
	}

	void SetRectScreen(RECT r)
	{
		rectScreen_ = r;
	}

	auto GetRectScreen()
	{
		return rectScreen_;
	}

	void SetProcessName(std::string fullname)
	{
		int index = fullname.rfind("\\");
		if (index != std::string::npos)
			index++;
		else
			index = 0;
		procName_ = fullname.substr(index);
	}

	auto GetProcessName()
	{
		return procName_;
	}

	void SetHwnd(::HWND hwnd)
	{
		hwnd_ = hwnd;
	}

	auto GetHwnd()
	{
		return hwnd_;
	}

	void SetParent(::HWND hwnd)
	{
		hwndParen_ = hwnd;
	}

	auto GetParent()
	{
		return hwndParen_;
	}

	void SetTitle(const char* title)
	{
		title_ = title;
	}

	auto GetTitle()
	{
		return title_;
	}

	void SetKlassName(const char* klass)
	{
		classname_ = klass;
	}

	auto GetKlassName()
	{
		return classname_;
	}

	std::string GetText()
	{
		char buf[1024]{};
		if (!title_.empty())
			title_ = "[" + title_ + "]";
		sprintf_s(buf, "0x%zX (%s) %s", hwnd_, classname_.c_str(), title_.c_str());
		return buf;
	}

	void SetIcon(ID3D11ShaderResourceView* icon)
	{
		icon_ = icon;
	}

	auto GetIcon()
	{
		return icon_;
	}

	void SetStyle(int style)
	{
		style_ = style;
	}

	auto GetStyle()
	{
		return style_;
	}

	void SetStyleEx(int styleEx)
	{
		styleEx_ = styleEx;
	}

	auto GetStyleEx()
	{
		return styleEx_;
	}

	void SetWndCb(uintptr_t cb)
	{
		callback_ = cb;
	}

	auto GetWndCb()
	{
		return callback_;
	}

	void SetPid(DWORD pid)
	{
		pid_ = pid;
	}

	auto GetPid()
	{
		return pid_;
	}

	void SetThreadId(DWORD tid)
	{
		thread_ = tid;
	}

	auto GetThreadId()
	{
		return thread_;
	}

	static void SetSortSpecs(ImGuiTableSortSpecs* a)
	{
		s_current_sort_specs_ = a;
	}

	static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, WindowItem* items, int items_count)
	{
		s_current_sort_specs_ = sort_specs; // Store in variable accessible by the sort function.
		if (items_count > 1)
			qsort(items, (size_t)items_count, sizeof(items[0]), ThreadItem::CompareWithSortSpecs);
		s_current_sort_specs_ = NULL;
	}

	static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs)
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

	void Show()
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

private:
	std::string text_;
	uint32_t type_;
	std::string title_;
	std::string classname_;
	std::string procName_;
	uint32_t style_;
	uint32_t styleEx_;
	RECT rectRelativa_;
	RECT rectScreen_;
	uint32_t pid_;
	uint32_t thread_;
	::HWND hwnd_;
	::HWND hwndParen_;
	uintptr_t callback_;
	ID3D11ShaderResourceView* icon_;

	static const ImGuiTableSortSpecs* s_current_sort_specs_;
};

const ImGuiTableSortSpecs* WindowItem::s_current_sort_specs_ = nullptr;