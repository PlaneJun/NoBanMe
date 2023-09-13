#pragma once


namespace utils
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

	HICON GetProcessIcon(std::string path)
	{
		HICON hIcon{};
		ExtractIconExA(path.c_str(), 0, nullptr, &hIcon, 1);
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
			GetEncoderClsid(_T("image/png"), &encoderCLSID);
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

	HMODULE GetProcessModuleHandle(DWORD pid, CONST TCHAR* moduleName) {	// 根据 PID 、模块名(需要写后缀，如：".dll")，获取模块入口地址。
		MODULEENTRY32 moduleEntry;
		HANDLE handle = NULL;
		handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid); //  获取进程快照中包含在th32ProcessID中指定的进程的所有的模块。
		if (!handle) {
			CloseHandle(handle);
			return NULL;
		}
		ZeroMemory(&moduleEntry, sizeof(MODULEENTRY32));
		moduleEntry.dwSize = sizeof(MODULEENTRY32);
		if (!Module32First(handle, &moduleEntry)) {
			CloseHandle(handle);
			return NULL;
		}
		do {
			if (_tcscmp(moduleEntry.szModule, moduleName) == 0) { return moduleEntry.hModule; }
		} while (Module32Next(handle, &moduleEntry));
		CloseHandle(handle);
		return 0;
	}

	uint32_t GetMainThreadId(uint32_t pid)
	{
		THREADENTRY32 te32;
		HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (hThreadSnap == INVALID_HANDLE_VALUE)
			return 0;

		te32.dwSize = sizeof(THREADENTRY32);
		if (!Thread32First(hThreadSnap, &te32)) 
		{
			CloseHandle(hThreadSnap);     // 必须在使用后清除快照对象!
			return(FALSE);
		}

		do 
		{
			if (te32.th32OwnerProcessID == pid) 
			{
				CloseHandle(hThreadSnap);
				return te32.th32ThreadID;
			}
		} while (Thread32Next(hThreadSnap, &te32));

		CloseHandle(hThreadSnap);
		return 0;
	}

	void EnumCurtAllProcess(std::vector<ProcessItem>& items)
	{
		HANDLE hSnapshot_proc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot_proc)
		{
			PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
			bool bprocess = Process32First(hSnapshot_proc, &pe);
			if(!bprocess)
				return;

			std::filesystem::path icon_dir("./Data/icon/");
			if (std::filesystem::exists(icon_dir))
				std::filesystem::remove_all(icon_dir);
			std::filesystem::create_directory(icon_dir);

			while (bprocess)
			{
				BOOL bIsWow64 =false;
				std::wstring wtmp(pe.szExeFile);
				std::string fullPath(wtmp.begin(), wtmp.end());
				char imagepath[MAX_PATH]{}, startuptime[MAX_PATH]{};
				auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
				if (hProc)
				{
					//Get Module Path
					GetModuleFileNameExA(hProc, NULL, imagepath, MAX_PATH);
					//GetStartupTime
					FILETIME ftCreation, ftExit, ftKernel, ftUser;
					SYSTEMTIME stCreation, lstCreation;
					if (GetProcessTimes(hProc, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
						FileTimeToSystemTime(&ftCreation, &stCreation);
						SystemTimeToTzSpecificLocalTime(NULL, &stCreation, &lstCreation);
						sprintf(startuptime,"%04d-%02d-%02d %02d:%02d:%02d", lstCreation.wYear, lstCreation.wMonth, lstCreation.wDay, lstCreation.wHour, lstCreation.wMinute, lstCreation.wSecond);
					}
					IsWow64Process(hProc,&bIsWow64);
					CloseHandle(hProc);
				}
				std::string proName = fullPath.substr(fullPath.rfind("\\") + 1);

				//创建保存icon的目录
				

				ID3D11ShaderResourceView* icon = nullptr;
				auto hIcon = GetProcessIcon(imagepath);
				if (hIcon != NULL && SaveIconToPng(hIcon, ("./Data/icon/" + proName + ".png").c_str()))
					icon = render::get_instasnce()->DX11LoadTextureImageFromFile(("./Data/icon/" + proName + ".png").c_str());
				items.push_back(ProcessItem(icon,pe.th32ProcessID ,proName,pe.th32ParentProcessID ,imagepath ,startuptime, bIsWow64));
				bprocess = Process32Next(hSnapshot_proc, &pe);
			}
			CloseHandle(hSnapshot_proc);
		}
	}

	void EnumPidModules(uint32_t pid ,std::vector<ModuleItem>& items)
	{
		HANDLE hSnapshot_proc = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		if (hSnapshot_proc)
		{
			MODULEENTRY32  me32 = { sizeof(MODULEENTRY32) };
			bool bprocess = Module32First(hSnapshot_proc, &me32);
			while (bprocess)
			{
				std::wstring wtmp(me32.szExePath);
				std::string fullPath(wtmp.begin(), wtmp.end());
				items.push_back(ModuleItem( fullPath,(uint64_t)me32.modBaseAddr,me32.modBaseSize ));
				bprocess = Module32Next(hSnapshot_proc, &me32);
			}
			CloseHandle(hSnapshot_proc);
		}
	}

	void EnumPidThread(uint32_t pid,std::vector<ThreadItem>& items)
	{
		THREADENTRY32 te32;
		HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (hThreadSnap == INVALID_HANDLE_VALUE)
			return;

		te32.dwSize = sizeof(THREADENTRY32);
		if (!Thread32First(hThreadSnap, &te32))
		{
			CloseHandle(hThreadSnap);     // 必须在使用后清除快照对象!
			return;
		}

		HMODULE hNtdll = LoadLibraryW(L"ntdll.dll");
		NTQUERYINFORMATIONTHREAD NtQueryInformationThread = NULL;
		NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)GetProcAddress(hNtdll, "NtQueryInformationThread");

		do
		{
			
			if (te32.th32OwnerProcessID == pid)
			{
				uint64_t startaddr{};
				DWORD retlen{};
				auto hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
				if (hThread)
				{
					NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &startaddr, sizeof(startaddr), &retlen);
					CloseHandle(hThread);
				}
				items.push_back(ThreadItem(te32.th32ThreadID, te32.tpDeltaPri, startaddr));
			}
			
		} while (Thread32Next(hThreadSnap, &te32));

		CloseHandle(hThreadSnap);
	}

	void EnumVehHandler(uint32_t pid, std::vector<VehHandlerItem>& items)
	{
		static auto QueryList = [=](HANDLE process,uint64_t LdrpVectorHandlerList, BOOL exception,BOOL isWow64) {

			std::vector<VehHandlerItem> items{};

			PLIST_ENTRY startLink{};
			PLIST_ENTRY currentLink{};
			ULONG64 decode_vehHandler{};


			VehHandlerItem::RTL_VECTORED_HANDLER_LIST* vectorPtr=new VehHandlerItem::RTL_VECTORED_HANDLER_LIST();
			ReadProcessMemory(process,(PVOID)LdrpVectorHandlerList,vectorPtr,sizeof(VehHandlerItem::RTL_VECTORED_HANDLER_LIST),NULL);
			if (exception)
			{
				startLink = (PLIST_ENTRY)PTR_ADD_OFFSET((LPVOID)LdrpVectorHandlerList, UFIELD_OFFSET(VehHandlerItem::RTL_VECTORED_HANDLER_LIST, ExceptionList));;
				currentLink = vectorPtr->ExceptionList.Flink;
			}
			else {
				startLink = (PLIST_ENTRY)PTR_ADD_OFFSET((LPVOID)LdrpVectorHandlerList, UFIELD_OFFSET(VehHandlerItem::RTL_VECTORED_HANDLER_LIST, ContinueList));;
				currentLink = vectorPtr->ContinueList.Flink;
			}
			int i = 0;
			while (currentLink != startLink && i <= 40)
			{
				VehHandlerItem::PRTL_VECTORED_EXCEPTION_ENTRY addressOfEntry = new VehHandlerItem::RTL_VECTORED_EXCEPTION_ENTRY();
				ReadProcessMemory(process, CONTAINING_RECORD(currentLink, VehHandlerItem::RTL_VECTORED_EXCEPTION_ENTRY, List), addressOfEntry, sizeof(VehHandlerItem::RTL_VECTORED_EXCEPTION_ENTRY), NULL);
				typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess)(HANDLE ProcessHandle, int ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
				fnNtQueryInformationProcess lpNtQueryInformationProcess = (fnNtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
				uint64_t decode_ptr{};
				uint64_t key = (uint64_t)addressOfEntry->VectoredHandler;
				uint32_t ProcessInformation{};
				uint32_t ret = lpNtQueryInformationProcess(process, 36, &ProcessInformation, 4, 0);
				if (ret >= 0)
				{
					if (!isWow64)
					{
						decode_ptr =  ProcessInformation^ __ROR4__(key, 32 - (ProcessInformation & 0x1F));
					}
					else
					{
						decode_ptr =  __ROR8__(key, 0x40 - (ProcessInformation & 0x3F)) ^ ProcessInformation;
					}
				}

				MEMORY_BASIC_INFORMATION m = { 0 };
				VirtualQueryEx(process, (PVOID)decode_vehHandler ,&m, sizeof(MEMORY_BASIC_INFORMATION));
				char module_path[MAX_PATH]{};
				if ((uint64_t)m.AllocationBase > 0)
					GetModuleFileNameExA(process, (HMODULE)m.AllocationBase, module_path,MAX_PATH);
				items.push_back(VehHandlerItem(decode_vehHandler, module_path, exception));
				currentLink = addressOfEntry->List.Flink;

				i++;
			}

			return items;
		};


		BOOL bIsWow64 = false;
		auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (hProc)
		{
			IsWow64Process(hProc, &bIsWow64);
			auto veh = QueryList(hProc,VehHandlerItem::GetLdrpVectorHandlerList(),true, bIsWow64);
			for (auto v : veh)
				items.push_back(v);
			veh = QueryList(hProc, VehHandlerItem::GetLdrpVectorHandlerList(), false, bIsWow64);
			for (auto v : veh)
				items.push_back(v);
			CloseHandle(hProc);
		}
	}

	BOOL InjectDLL(DWORD PID, const char* Path) {
		HANDLE hProcess;
		DWORD dwSize = 0;
		LPVOID PDllAddr;
		FARPROC func;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
		if (hProcess == NULL) 
		{
			MessageBoxA(NULL,"OpenProcess Failed!","control",NULL);
			return false;
		}
		dwSize = strlen(Path) + 1;
		PDllAddr = VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
		if (PDllAddr == NULL) 
		{
			MessageBoxA(NULL, "VirtualAllocEx Error", "control", NULL);
			return false;
		}
		BOOL pWrite = WriteProcessMemory(hProcess, PDllAddr, Path, dwSize, NULL);
		if (!pWrite) 
		{
			MessageBoxA(NULL, "WriteProcessMemory Error", "control", NULL);
			return false;
		}
		HMODULE hModule = GetModuleHandleA("kernel32.dll");
		func = GetProcAddress(hModule, "LoadLibraryA");
		if (func == NULL) 
		{
			MessageBoxA(NULL, "GetProcAddress Error", "control", NULL);
			return false;

		}
		HANDLE pRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)func, PDllAddr, 0, NULL);
		if (pRemoteThread == NULL) 
		{
			MessageBoxA(NULL, "CreateRemoteThread Error", "control", NULL);
			return false;
		}
		WaitForSingleObject(pRemoteThread, INFINITE);
		VirtualFreeEx(hProcess, PDllAddr, dwSize, MEM_COMMIT);
		CloseHandle(hProcess);
		return true;
	}

	bool CallFunction(ControlCmd cmd)
	{
		auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, data::global::target);
		if (hProcess == NULL)
		{
			MessageBoxA(NULL, "OpenProcess Failed!", "control", NULL);
			return false;
		}
		//申请参数内存
		auto lpParam = VirtualAllocEx(hProcess, NULL, sizeof(ControlCmd), MEM_COMMIT, PAGE_READWRITE);
		if (lpParam == NULL)
		{
			MessageBoxA(NULL, "VirtualAllocEx Params Failed!", "control", NULL);
			return false;
		}

		if (!WriteProcessMemory(hProcess, lpParam, &cmd, sizeof(ControlCmd), NULL))
		{
			MessageBoxA(NULL, "Write Params Failed!", "control", NULL);
			return false;
		}
		HANDLE  pRemoteThread = CreateRemoteThread(hProcess, NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(data::global::fnDispatch), lpParam, NULL, NULL);
		if (pRemoteThread == NULL)
		{
			MessageBoxA(NULL, "CreateRemoteThread Error", "control", NULL);
			return false;
		}
		WaitForSingleObject(pRemoteThread, INFINITE);
		VirtualFreeEx(hProcess, lpParam, sizeof(ControlCmd), MEM_COMMIT);
		CloseHandle(hProcess);
		return true;
	}


	
}

