#pragma once
#include <string>
#include <Windows.h>
#include <stdint.h>
#include <imgui.h>
#include <D3DX11tex.h>

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


	WindowItem();

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
		uintptr_t callback);

	void SetRectRelativa(RECT r);

	RECT GetRectRelativa();

	void SetRectScreen(RECT r);

	RECT GetRectScreen();

	void SetProcessName(std::string fullname);

	std::string GetProcessName();

	void SetHwnd(::HWND hwnd);

	::HWND GetHwnd();

	void SetParent(::HWND hwnd);

	::HWND GetParent();

	void SetTitle(const char* title);

	std::string GetTitle();

	void SetKlassName(const char* klass);

	std::string GetKlassName();

	std::string GetText();

	void SetIcon(ID3D11ShaderResourceView* icon);

	ID3D11ShaderResourceView* GetIcon();

	void SetStyle(int style);

	int GetStyle();

	void SetStyleEx(int styleEx);

	int GetStyleEx();

	void SetWndCb(uintptr_t cb);

	uintptr_t GetWndCb();

	void SetPid(uint32_t pid);

	uint32_t GetPid();

	void SetThreadId(uint32_t tid);

	uint32_t GetThreadId();

	static void SetSortSpecs(ImGuiTableSortSpecs* a);

	static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, WindowItem* items, int items_count);

	static int __cdecl CompareWithSortSpecs(const void* lhs, const void* rhs);

	void Show();

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