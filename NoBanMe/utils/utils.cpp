#include "utils.h"
#include <sstream>
#include <Shlobj.h>
#include <versionhelpers.h>
#include <gdiplus.h>
#include "../mem/memstub.h"
#include "../render/render.h"
#include "../global.h"
#include "../pe/pefile.h"

namespace utils
{
	namespace conver
	{
		uint64_t hexToInteger(std::string hex)
		{
			std::stringstream ss2;
			uint64_t d2;
			ss2 << std::hex << hex;
			ss2 >> d2;
			return d2;
		}

		std::string IntegerTohex(uint64_t dec)
		{
			std::stringstream ss2;
			ss2 << std::hex << dec;
			return ss2.str();
		}


		std::string bytesToHexString(const BYTE* bytes, const int length)
		{
			if (bytes == NULL) {
				return "";
			}
			std::string buff;
			const int len = length;
			for (int j = 0; j < len; j++) {
				int high = bytes[j] / 16, low = bytes[j] % 16;
				buff += (high < 10) ? ('0' + high) : ('a' + high - 10);
				buff += (low < 10) ? ('0' + low) : ('a' + low - 10);
				buff += " ";
			}
			return buff;
		}

		std::string wstring_to_stirng(std::wstring wstr)
		{
			std::string result;
			//��ȡ��������С��������ռ䣬��������С�°��ֽڼ����  
			int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
			char* buffer = new char[len + 1];
			//���ֽڱ���ת���ɶ��ֽڱ���  
			WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
			buffer[len] = '\0';
			//ɾ��������������ֵ  
			result.append(buffer);
			delete[] buffer;
			return result;
		}

		std::wstring string_to_wstirng(std::string str)
		{
			std::wstring result;
			//��ȡ��������С��������ռ䣬��������С���ַ�����  
			int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
			TCHAR* buffer = new TCHAR[len + 1];
			//���ֽڱ���ת���ɿ��ֽڱ���  
			MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
			buffer[len] = '\0';             //����ַ�����β  
			result.append(buffer);
			delete[] buffer;
			return result;
		}

		std::string string_To_UTF8(const std::string& str)
		{
			int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

			wchar_t* pwBuf = new wchar_t[nwLen + 1];
			ZeroMemory(pwBuf, nwLen * 2 + 2);

			::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

			int nLen = (int)(::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL));

			char* pBuf = new char[nLen + 1];
			ZeroMemory(pBuf, nLen + 1);

			::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

			std::string retStr(pBuf);

			delete[]pwBuf;
			delete[]pBuf;

			pwBuf = NULL;
			pBuf = NULL;

			return retStr;
		}
	}

	namespace process
	{
		std::string GetFullProcessName(DWORD pid) {
			HANDLE hProcess(::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
			if (hProcess) {
				WCHAR path[MAX_PATH];
				DWORD size = MAX_PATH;
				if (::QueryFullProcessImageName(hProcess, 0, path, &size))
					return conver::wstring_to_stirng(path);
			}
			return "";
		}

		std::wstring GetCommandLine(uint32_t pid) {
			if (::IsWindows8OrGreater()) {
				uint32_t size = 8192;
				auto buffer = std::make_unique<BYTE[]>(size);
				auto status = MemStub::NtQueryInformationProcess(pid, ProcessCommandLineInformation, reinterpret_cast<uintptr_t>(buffer.get()), size, &size);
				if (status == NULL) {
					auto str = (UNICODE_STRING*)buffer.get();
					return std::wstring(str->Buffer, str->Length / sizeof(WCHAR));
				}
				if (status == 0xC0000003) {
					return L"";
				}
			}
			else {
				/*
					std::wstring cmdLine;
						PEB peb;
						if (!GetProcessPeb(hProcess, &peb))
							return cmdLine;

						RTL_USER_PROCESS_PARAMETERS processParams;
						if (!::ReadProcessMemory(hProcess, peb.ProcessParameters, &processParams, sizeof(processParams), nullptr))
							return cmdLine;

						cmdLine.resize(processParams.CommandLine.Length / sizeof(WCHAR) + 1);
						if (!::ReadProcessMemory(hProcess, processParams.CommandLine.Buffer, cmdLine.data(),
							processParams.CommandLine.Length, nullptr))
							return L"";

						return cmdLine;

				*/
			}
			return L"";
		}

		bool DumpMemory(uint32_t pid, const wchar_t* module_name, const char* save_path)
		{
			PEFile pe;
			if (pe.load_from_pid(pid, module_name))
			{
				for (auto i : pe.get_section_headers())
				{
					i->PointerToRawData = i->VirtualAddress;
					i->SizeOfRawData = i->Misc.VirtualSize;
				}
				return pe.save_to_file(save_path);
			}
			return false;
		}
	}

	namespace image
	{
		HICON GetWindowIcon(HWND hwnd) {
			HICON hIcon{ nullptr };
			SendMessageTimeout(hwnd, WM_GETICON, ICON_SMALL2, 0, SMTO_ABORTIFHUNG | SMTO_ERRORONEXIT, 100, (DWORD_PTR*)&hIcon);
			if (!hIcon) {
				hIcon = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM);
			}
			return hIcon;
		}

		HICON GetProcessIcon(std::string path)
		{
			HICON hIcon{};
			ExtractIconExA(path.c_str(), 0, nullptr, &hIcon, 1);
			return hIcon;
		}

		HICON GetProcessIcon(HWND hwnd)
		{
			auto hIcon = GetWindowIcon(hwnd);
			if (!hIcon) {
				DWORD pid = 0;
				GetWindowThreadProcessId(hwnd, &pid);
				if (pid) {
					ExtractIconExA(process::GetFullProcessName(pid).c_str(), 0, nullptr, &hIcon, 1);
				}
			}

			return hIcon;
		}

		int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
		{
			UINT num = 0, size = 0;

			Gdiplus::GetImageEncodersSize(&num, &size);
			if (size == 0)
				return -1;  // Failure

			Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));

			Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
			bool found = false;
			for (UINT ix = 0; !found && ix < num; ++ix)
			{
				if (_wcsicmp(pImageCodecInfo[ix].MimeType, format) == 0)
				{
					*pClsid = pImageCodecInfo[ix].Clsid;
					found = true;
					break;
				}
			}

			free(pImageCodecInfo);
			return found;
		}

		bool SaveIconToPng(HICON hIcon, std::string lpszPicFileName)
		{
			static bool init = false;
			if (!init)
			{
				ULONG_PTR m_gdiplusToken;
				Gdiplus::GdiplusStartupInput gdiplusStartupInput;
				Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
				init = true;
			}

			if (hIcon == NULL)
			{
				return false;
			}

			ICONINFO icInfo = { 0 };
			if (!::GetIconInfo(hIcon, &icInfo))
			{
				return false;
			}

			BITMAP bitmap;
			GetObject(icInfo.hbmColor, sizeof(BITMAP), &bitmap);

			Gdiplus::Bitmap* pBitmap = NULL;
			Gdiplus::Bitmap* pWrapBitmap = NULL;

			do
			{
				if (bitmap.bmBitsPixel != 32)
				{
					pBitmap = Gdiplus::Bitmap::FromHICON(hIcon);
				}
				else
				{
					pWrapBitmap = Gdiplus::Bitmap::FromHBITMAP(icInfo.hbmColor, NULL);
					if (!pWrapBitmap)
						break;

					Gdiplus::BitmapData bitmapData;
					Gdiplus::Rect rcImage(0, 0, pWrapBitmap->GetWidth(), pWrapBitmap->GetHeight());

					pWrapBitmap->LockBits(&rcImage, Gdiplus::ImageLockModeRead, pWrapBitmap->GetPixelFormat(), &bitmapData);
					pBitmap = new (Gdiplus::Bitmap)(bitmapData.Width, bitmapData.Height, bitmapData.Stride, PixelFormat32bppARGB, (BYTE*)bitmapData.Scan0);
					pWrapBitmap->UnlockBits(&bitmapData);
				}

				CLSID encoderCLSID;
				GetEncoderClsid(L"image/png", &encoderCLSID);
				std::wstring aa{ lpszPicFileName.begin(),lpszPicFileName.end() };
				Gdiplus::Status st = pBitmap->Save(aa.c_str(), &encoderCLSID, NULL);
				if (st != Gdiplus::Ok)
					break;

			} while (false);

			delete pBitmap;
			if (pWrapBitmap)
				delete pWrapBitmap;
			DeleteObject(icInfo.hbmColor);
			DeleteObject(icInfo.hbmMask);

			return true;
		}
	}

	namespace file
	{
		bool QueryValue(const std::string& ValueName, const std::string& szModuleName, std::string& RetStr)
		{
			bool bSuccess = FALSE;
			BYTE* m_lpVersionData = NULL;
			DWORD  m_dwLangCharset = 0;
			CHAR* tmpstr = NULL;
			do
			{
				if (!ValueName.size() || !szModuleName.size())
				{
					break;
				}
				DWORD dwHandle;
				// �ж�ϵͳ�ܷ������ָ���ļ��İ汾��Ϣ
				//��԰����˰汾��Դ��һ���ļ����ж����� �ļ��汾��Ϣ��Ҫһ�����Ļ�����
				//����ֵ˵��Long�������ļ��İ汾��Դ����Ļ��������ȡ����ļ��������汾��Ϣ���򷵻�һ��0ֵ��������GetLastError������
				DWORD dwDataSize = GetFileVersionInfoSizeA((LPCSTR)szModuleName.c_str(),
					&dwHandle);
				if (dwDataSize == 0)
				{
					break;
				}
				//std::nothrow:���ڴ治��ʱ��new (std::nothrow)�����׳��쳣�����ǽ�ָ����NULL��
				m_lpVersionData = new (std::nothrow) BYTE[dwDataSize];// ���仺����
				if (NULL == m_lpVersionData)
				{
					break;
				}
				// ������Ϣ
				//��֧�ְ汾��ǵ�һ��ģ�����ȡ�ļ��汾��Ϣ
				if (!GetFileVersionInfoA((LPCSTR)szModuleName.c_str(), dwHandle, dwDataSize,
					(void*)m_lpVersionData))
				{
					break;
				}
				UINT nQuerySize;
				DWORD* pTransTable;
				if (!VerQueryValueA(m_lpVersionData, "\\VarFileInfo\\Translation",
					(void**)&pTransTable, &nQuerySize))
				{
					break;
				}
				//MAKELONG ������16λ�������ϳ�һ���޷��ŵ�32λ��
				m_dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));
				if (m_lpVersionData == NULL)
				{
					break;
				}
				tmpstr = new (std::nothrow) CHAR[128];// ���仺����
				if (NULL == tmpstr)
				{
					break;
				}
				sprintf_s(tmpstr, 128, "\\StringFileInfo\\%08lx\\%s", m_dwLangCharset,
					ValueName.c_str());
				LPVOID lpData;
				// ���ô˺�����ѯǰ��Ҫ�����ε��ú���GetFileVersionInfoSize��GetFileVersionInfo
				if (::VerQueryValueA((void*)m_lpVersionData, tmpstr, &lpData, &nQuerySize))
				{
					RetStr = (char*)lpData;
				}
				bSuccess = TRUE;
			} while (FALSE);
			// ���ٻ�����
			if (m_lpVersionData)
			{
				delete[] m_lpVersionData;
				m_lpVersionData = NULL;
			}
			if (tmpstr)
			{
				delete[] tmpstr;
				tmpstr = NULL;
			}
			return bSuccess;
		}

		//��ȡ�ļ�˵��
		std::string GetFileDescription(const std::string szModuleName)
		{
			std::string ret{};
			return QueryValue("FileDescription", szModuleName, ret) ? ret : "";
		}

		//��ȡ�ļ��汾
		std::string GetFileVersion(const std::string szModuleName)
		{
			std::string ret{};
			return QueryValue("FileVersion", szModuleName, ret) ? ret : "";
		}
		//��ȡ�ڲ�����
		std::string  GetInternalName(const std::string szModuleName)
		{
			std::string ret{};
			return QueryValue("InternalName", szModuleName, ret) ? ret : "";
		}

		//��ȡ��˾����
		std::string  GetCompanyName(const std::string szModuleName)
		{
			std::string ret{};
			return QueryValue("CompanyName", szModuleName, ret) ? ret : "";
		}

		//��ȡ��Ȩ
		std::string GetLegalCopyright(const std::string szModuleName)
		{
			std::string ret{};
			return QueryValue("LegalCopyright", szModuleName, ret) ? ret : "";
		}

		//��ȡԭʼ�ļ���
		std::string GetOriginalFilename(const std::string szModuleName)
		{
			std::string ret{};
			return QueryValue("OriginalFilename", szModuleName, ret) ? ret : "";
		}

		//��ȡ��Ʒ����
		std::string  GetProductName(const std::string szModuleName)
		{
			std::string ret{};
			return QueryValue("ProductName", szModuleName, ret) ? ret : "";
		}

		//��ȡ��Ʒ�汾
		std::string GetProductVersion(const std::string szModuleName)
		{
			std::string ret{};
			return QueryValue("ProductVersion", szModuleName, ret) ? ret : "";
		}

		void OpenFilePropertyDlg(const char* path)
		{
			SHELLEXECUTEINFOA shell;
			shell.hwnd = NULL;
			shell.lpVerb = "properties";
			shell.lpFile = path;
			shell.lpDirectory = NULL;
			shell.lpParameters = NULL;
			shell.nShow = SW_SHOWNORMAL;
			shell.fMask = SEE_MASK_INVOKEIDLIST;
			shell.lpIDList = NULL;
			shell.cbSize = sizeof(SHELLEXECUTEINFOW);
			ShellExecuteExA(&shell);
		}

		void OpenFolderAndSelectFile(const char* filepath)
		{
			LPSHELLFOLDER pDesktopFolder;
			CoInitialize(NULL);

			if (FAILED(SHGetDesktopFolder(&pDesktopFolder)))
			{
				CoUninitialize();
				return;
			}

			LPITEMIDLIST pidl;
			ULONG chEaten;
			HRESULT hr = pDesktopFolder->ParseDisplayName(NULL, 0, (LPWSTR)conver::string_to_wstirng(filepath).c_str(), &chEaten, &pidl, NULL);
			if (FAILED(hr))
			{
				pDesktopFolder->Release();
				CoUninitialize();
				return;
			}
			SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
			pDesktopFolder->Release();
			CoUninitialize();
		}

		std::string openFileDlg()
		{
			OPENFILENAMEA ofn;
			char szFile[MAX_PATH];
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = render::get_instasnce()->GetHwnd();
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = L'\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Dll\0*.Dll\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileNameA(&ofn))
			{
				return  ofn.lpstrFile;
			}
			else
			{
				return "";
			}
		}
	}

	namespace mem
	{
		std::string GetProtectByValue(uint32_t value)
		{
			std::string ret{};

			if (value & PAGE_NOACCESS)
				ret += "NoAccess|";
			if (value & PAGE_READONLY)
				ret += "ReadOnly|";
			if (value & PAGE_WRITECOPY)
				ret += "WriteCopy|";
			if (value & PAGE_EXECUTE)
				ret += "Execute|";
			if (value & PAGE_EXECUTE_READ)
				ret += "ExecuteRead|";
			if (value & PAGE_EXECUTE_READWRITE)
				ret += "ExecuteReadWrite|";
			if (value & PAGE_EXECUTE_WRITECOPY)
				ret += "ExecuteReadWriteCopy|";
			if (value & PAGE_NOCACHE)
				ret += "NoCache|";

			if (!ret.empty())
				ret = ret.substr(0, ret.length() - 1);
			else
				ret = "-";

			return ret;
		}

		std::string GetStateByValue(uint32_t value)
		{
			std::string ret{};

			if (value & MEM_COMMIT)
				ret += "Commit|";
			if (value & MEM_RESERVE)
				ret += "Reserve|";
			if (value & MEM_FREE)
				ret += "Free|";

			if (!ret.empty())
				ret = ret.substr(0, ret.length() - 1);
			else
				ret = "-";

			return ret;
		}

		std::string GetTypeByValue(uint32_t value)
		{
			std::string ret{};

			if (value & MEM_PRIVATE)
				ret += "Private";
			if (value & MEM_MAPPED)
				ret += "Image";
			if (value & MEM_FREE)
				ret += "MEM_IMAGE";

			if (!ret.empty())
				ret = ret.substr(0, ret.length() - 1);
			else
				ret = "-";

			return ret;
		}

		bool InvokePluginFunction(DWORD pid, ControlCmd cmd)
		{
			//printf("[DBG]global::plugin::lpPluginDispatch=%p\n", global::plugin::lpPluginDispatch);
			return MemStub::RemoteCallFunction(pid, global::plugin::lpPluginDispatch, reinterpret_cast<uintptr_t>(&cmd), sizeof(ControlCmd));
		}
	}

	namespace normal
	{
		void CopyStringToClipboard(const char* str)
		{
			if (OpenClipboard(render::get_instasnce()->GetHwnd())) {
				EmptyClipboard();
				HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, strlen(str) * 2 + 2);
				LPWSTR pData = (LPWSTR)GlobalLock(hData);
				lstrcpyW(pData, conver::string_to_wstirng(str).c_str());
				GlobalUnlock(hData);
				SetClipboardData(CF_UNICODETEXT, hData);
				CloseClipboard();
			}
		}
	}

	namespace window
	{
		HWND GetHwnd(DWORD dwProcessID)
		{
			HWND hTop = GetTopWindow(0);
			HWND retHwnd = NULL;
			while (hTop)
			{
				DWORD pid = 0;
				DWORD dwTheardId = GetWindowThreadProcessId(hTop, &pid);
				if (dwTheardId != 0)
				{
					if (pid == dwProcessID && GetParent(hTop) == NULL && ::IsWindowVisible(hTop))
					{
						retHwnd = hTop;    //���ж�����ֵ
					}
				}
				hTop = GetNextWindow(hTop, GW_HWNDNEXT);
			}
			return retHwnd;
		}

		std::string GetTitleName(HWND hwnd)
		{
			char title[256]{};
			GetWindowTextA(hwnd, title, 256);
			return title;
		}

		std::string GetKlassName(HWND hwnd)
		{
			char klassname[256]{};
			GetClassNameA(hwnd, klassname, 256);
			return klassname;
		}

		std::string GetWindowStyleToString(int style) {
			std::string text;

			static struct {
				DWORD style;
				PCSTR text;
			} styles[] = {
				{ WS_POPUP			, "POPUP"},
				{ WS_CHILD			, "CHILD"},
				{ WS_MINIMIZE		, "MINIMIZE"},
				{ WS_VISIBLE		, "VISIBLE"},
				{ WS_DISABLED		, "DISABLED" },
				{ WS_CLIPSIBLINGS	, "CLIPSIBLINGS" },
				{ WS_CLIPCHILDREN	, "CLIPCHILDREN" },
				{ WS_MAXIMIZE		, "MAXIMIZE" },
				{ WS_BORDER			, "BORDER" },
				{ WS_DLGFRAME		, "DLGFRAME" },
				{ WS_VSCROLL		, "VSCROLL" },
				{ WS_HSCROLL		, "HSCROLL" },
				{ WS_SYSMENU		, "SYSMENU" },
				{ WS_THICKFRAME		, "THICKFRAME" },
				{ WS_MINIMIZEBOX	, "MINIMIZEBOX" },
				{ WS_MAXIMIZEBOX	, "MAXIMIZEBOX" },
			};

			for (auto& item : styles) {
				if (style & item.style)
					text += std::string(item.text) += ", ";
			}
			if (text.empty())
				return "OVERLAPPED, ";

			return "(WS_) " + text.substr(0, text.length() - 2);
		}

		std::string GetWindowExtendedStyleToString(int style) {
			std::string text;

			static struct {
				DWORD style;
				PCSTR text;
			} styles[] = {
				{ WS_EX_DLGMODALFRAME		, "DLGMODALFRAME" },
				{ WS_EX_NOPARENTNOTIFY		, "NOPARENTNOTIFY" },
				{ WS_EX_TOPMOST				, "TOPMOST" },
				{ WS_EX_ACCEPTFILES			, "ACCEPTFILES" },
				{ WS_EX_TRANSPARENT			, "TRANSPARENT" },
				{ WS_EX_MDICHILD			, "MDICHILD" },
				{ WS_EX_TOOLWINDOW			, "TOOLWINDOW" },
				{ WS_EX_WINDOWEDGE			, "WINDOWEDGE" },
				{ WS_EX_CLIENTEDGE			, "CLIENTEDGE" },
				{ WS_EX_CONTEXTHELP			, "CONTEXTHELP" },
				{ WS_EX_RIGHT				, "RIGHT" },
				{ WS_EX_RTLREADING			, "RTLREADING" },
				{ WS_EX_LTRREADING			, "LTRREADING" },
				{ WS_EX_LEFTSCROLLBAR		, "LEFTSCROLLBAR" },
				{ WS_EX_CONTROLPARENT		, "CONTROLPARENT" },
				{ WS_EX_STATICEDGE			, "STATICEDGE" },
				{ WS_EX_APPWINDOW			, "APPWINDOW" },
				{ WS_EX_LAYERED				, "LAYERED" },
				{ WS_EX_NOINHERITLAYOUT		, "NOINHERITLAYOUT" },
				//{ WS_EX_NOREDIRECTIONBITMAP , L"NOREDIRECTIONBITMAP" },
				{ WS_EX_LAYOUTRTL			, "LAYOUTRTL" },
				{ WS_EX_COMPOSITED			, "COMPOSITED" },
				{ WS_EX_NOACTIVATE			, "NOACTIVATE" },
			};

			for (auto& item : styles) {
				if (style & item.style)
					text += std::string(item.text) += ", ";
			}

			if (text.empty())
				text = "LEFT, RIGHISCROLLBAR";

			return "(WS_EX_) " + text.substr(0, text.length() - 2);
		}
	}


}

