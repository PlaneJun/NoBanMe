#pragma once
class WindowItem
{
public:
	WindowItem()
	{
		this->id_ = NULL;
		this->text_ = std::string();
		this->type_ = NULL;
		this->title_ =std::string();
		this->classname_= std::string();
		this->procName_	 =std::string();
		this->style_ = NULL;
		this->styleEx_ = NULL;
		this->rectRelativa_ = {0};
		this->rectScreen_ = {0};
		this->pid_ = NULL;
		this->thread_ = NULL;
		this->hwnd_ = NULL;
		this->hwndTop_ = NULL;
		this->hwndParen_ = NULL;
	}

	WindowItem(uint32_t id,
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
				HWND hwnd,
				HWND  hwndTop,
				HWND hwndParen)
	{
		this->id_ = id;
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
		this->hwndTop_ = hwndTop;
		this->hwndParen_ = hwndParen;
	}


	static void EnumAllWindows()
	{
		EnumWindows(EnumWindowsProc, 0);
	}
private:
	uint32_t id_;
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
	HWND hwnd_;
	HWND  hwndTop_;
	HWND hwndParen_;

	//BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
	//{

	//	LPWSTR lpString = (LPWSTR)malloc(1024 * sizeof(WCHAR));
	//	LPWSTR ClassString = (LPWSTR)malloc(1024 * sizeof(WCHAR));
	//	GetWindowText(hwnd, lpString, 1024);
	//	GetClassName(hwnd, ClassString, 1024);
	//	if (wcscmp(ClassString, _T("Edit")) == 0) {

	//		Ghwnd[i] = hwnd;
	//		i++;
	//	}
	//	//wprintf(L"%s\n", lpString);
	//	return true;
	//}



	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) // 回调函数
	{
		TCHAR szTitle[200];
		TCHAR szClass[200];
		GetWindowText(hwnd, szTitle, sizeof(szTitle) / sizeof(TCHAR)); // 获取窗口名称
		GetClassName(hwnd, szClass, sizeof(szClass) / sizeof(TCHAR)); // 窗口类
		if (szTitle[0] != '\0' && IsWindowVisible(hwnd)) { // 判断窗口标题不为空，并且窗口可见
			printf("%s,%s\n", szTitle, szClass);
		}
		return TRUE;
	}
};