#pragma once
#include <string>
#include <Windows.h>
#include "../nativeStruct.h"

#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib,"version.lib")
namespace utils
{
	namespace conver
	{
		uint64_t hexToInteger(std::string hex);

		std::string IntegerTohex(uint64_t dec);

		std::string bytesToHexString(const BYTE* bytes, const int length);

		std::string wstring_to_stirng(std::wstring wstr);

		std::wstring string_to_wstirng(std::string str);

		std::string string_To_UTF8(const std::string& str);
	}

	namespace process
	{
		std::string GetFullProcessName(DWORD pid);

		std::wstring GetCommandLine(uint32_t pid);
	}

	namespace image
	{
		HICON GetWindowIcon(HWND hwnd);

		HICON GetProcessIcon(std::string path);

		HICON GetProcessIcon(HWND hwnd);

		int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

		bool SaveIconToPng(HICON hIcon, std::string lpszPicFileName);
	}

	namespace file
	{
		bool QueryValue(const std::string& ValueName, const std::string& szModuleName, std::string& RetStr);

		//获取文件说明
		std::string GetFileDescription(const std::string szModuleName);

		//获取文件版本
		std::string GetFileVersion(const std::string szModuleName);

		//获取内部名称
		std::string  GetInternalName(const std::string szModuleName);

		//获取公司名称
		std::string  GetCompanyName(const std::string szModuleName);

		//获取版权
		std::string GetLegalCopyright(const std::string szModuleName);

		//获取原始文件名
		std::string GetOriginalFilename(const std::string szModuleName);

		//获取产品名称
		std::string  GetProductName(const std::string szModuleName);

		//获取产品版本
		std::string GetProductVersion(const std::string szModuleName);

		void OpenFilePropertyDlg(const char* path);

		void OpenFolderAndSelectFile(const char* filepath);

		std::string openFileDlg();
	}

	namespace mem
	{
		std::string GetProtectByValue(uint32_t value);

		std::string GetStateByValue(uint32_t value);

		std::string GetTypeByValue(uint32_t value);

		bool InvokePluginFunction(DWORD pid, ControlCmd cmd);
	}

	namespace normal
	{
		void CopyStringToClipboard(const char* str);
	}

	namespace window
	{
		HWND GetHwnd(DWORD dwProcessID);

		std::string GetTitleName(HWND hwnd);

		std::string GetKlassName(HWND hwnd);

		std::string GetWindowStyleToString(int style);

		std::string GetWindowExtendedStyleToString(int style);
	}


}