#pragma once

namespace Mem
{
	uint64_t AllocMem(uint32_t pid,uint64_t base,uint64_t size, uint32_t flAllocationType, uint32_t flProtect)
	{
		auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (!hProc)
			return 0;
		uint64_t ret = (uint64_t)VirtualAllocEx(hProc, NULL, size, flAllocationType, flProtect);;
		CloseHandle(hProc);
		return ret;
	}

	void FreeMem(uint32_t pid, uint64_t base, uint64_t size, uint32_t flAllocationType)
	{
		auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (!hProc)
			return;
		VirtualFreeEx(hProc, NULL, size, flAllocationType);
		CloseHandle(hProc);
	}

	size_t QueryMem(uint32_t pid,LPCVOID lpAddress,PMEMORY_BASIC_INFORMATION lpBuffer,SIZE_T dwLength)
	{
		auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (!hProc)
			return 0;
		size_t ret = VirtualQueryEx(hProc, (PVOID)lpAddress, lpBuffer, dwLength);
		CloseHandle(hProc);
		return ret;
	}

	bool ReadMemory(uint32_t pid,uint64_t ptr, void* buff, size_t size)
	{
		auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		bool ret = ReadProcessMemory(hProc, (PVOID)ptr, buff, size, NULL);
		CloseHandle(hProc);
		return ret;
	}

	bool WriteMemory(uint32_t pid, uint64_t Address, PVOID Buffer, uint32_t Size)
	{
		auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (!hProc)
			return false;
		bool ret = WriteProcessMemory(hProc, (PVOID)Address, Buffer, Size, NULL);
		CloseHandle(hProc);
		return ret;
	}

	std::string GetModuleFullName(uint32_t pid,uint64_t base )
	{
		auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (!hProc)
			return std::string();
		char module_path[MAX_PATH]{};
		GetModuleFileNameExA(hProc, (HMODULE)base, module_path, MAX_PATH);
		return module_path;
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

	BOOL RemoteInjectDLL(DWORD PID, const char* Path)
	{
		HANDLE hProcess;
		DWORD dwSize = 0;
		LPVOID PDllAddr;
		FARPROC func;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
		if (hProcess == NULL)
		{
			MessageBoxA(NULL, "OpenProcess Failed!", "control", NULL);
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

	bool RemoteCallFunction(DWORD PID, uint64_t addr, PVOID pargs, size_t args_size)
	{
		auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
		if (hProcess == NULL)
		{
			MessageBoxA(NULL, "OpenProcess Failed!", "control", NULL);
			return false;
		}
		//申请参数内存
		auto lpParam = VirtualAllocEx(hProcess, NULL, args_size, MEM_COMMIT, PAGE_READWRITE);
		if (lpParam == NULL)
		{
			MessageBoxA(NULL, "VirtualAllocEx Params Failed!", "control", NULL);
			return false;
		}

		if (!WriteProcessMemory(hProcess, lpParam, pargs, args_size, NULL))
		{
			MessageBoxA(NULL, "Write Params Failed!", "control", NULL);
			return false;
		}

		ZECREATETHREADEX lpZwCreateThreadEx = (ZECREATETHREADEX)GetProcAddress(LoadLibraryA("ntdll.dll"), "ZwCreateThreadEx");
		if (!lpZwCreateThreadEx)
		{
			VirtualFreeEx(hProcess, lpParam, sizeof(ControlCmd), MEM_COMMIT);
			CloseHandle(hProcess);
			return false;
		}
			
		HANDLE hRemoteThread{};
		auto dwStatus = lpZwCreateThreadEx(&hRemoteThread, PROCESS_ALL_ACCESS, NULL, hProcess,(LPTHREAD_START_ROUTINE)addr, lpParam, 0, 0, 0, 0, NULL);
		if (dwStatus != NULL)
		{
			VirtualFreeEx(hProcess, lpParam, sizeof(ControlCmd), MEM_COMMIT);
			CloseHandle(hProcess);
			MessageBoxA(NULL, "CreateRemoteThread Error", "control", NULL);
			return false;
		}
		WaitForSingleObject(hRemoteThread, INFINITE);
		VirtualFreeEx(hProcess, lpParam, sizeof(ControlCmd), MEM_COMMIT);
		CloseHandle(hProcess);
		return true;
	}

	void IsWow64(uint32_t pid,BOOL* ret)
	{
		auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (hProcess == NULL)
		{
			*ret = false;
			return;
		}
		IsWow64Process(hProcess,ret);
		CloseHandle(hProcess);
		return;
	}

	NTSTATUS NtQueryInformationProcess(uint32_t pid, int ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength)
	{
		auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
		if (hProcess == NULL)
		{
			return -1;
		}
		typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess)(HANDLE ProcessHandle, int ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
		fnNtQueryInformationProcess lpNtQueryInformationProcess = (fnNtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
		NTSTATUS ret =lpNtQueryInformationProcess(hProcess, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		CloseHandle(hProcess);
		return ret;
	}



	template<class T>
	T Read(uint32_t pid, uint64_t ptr)
	{
		T buff;
		ReadMemory(pid,ptr, &buff, sizeof(T));
		return buff;
	}

	template <typename T>
	bool Write(uint32_t pid, uint64_t addr, T Val)
	{
		return WriteMemory(pid, addr,&Val,sizeof(T));
	}



	uint64_t GetDllExportRva(const char* module_path, const char* export_fun)
	{
		auto base = GetModuleHandleA(module_path);
		if (base == NULL)
			base = LoadLibraryA(module_path);
		uint64_t ret = (uint64_t)(reinterpret_cast<uint8_t*>(GetProcAddress(base, export_fun)) - (uint8_t*)base);
		FreeLibrary(base);
		return ret;
	}

}