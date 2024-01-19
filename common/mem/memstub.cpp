#include"memstub.h"
#include <Psapi.h>
#include <tlhelp32.h>
#include "../ipc_ctrl.h"

uint64_t MemStub::AllocMem(uint32_t pid, uint64_t base, uint64_t size, uint32_t flAllocationType, uint32_t flProtect)
{
	auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProc)
	{
		return 0;
	}
		
	uint64_t ret =reinterpret_cast<uint64_t>(VirtualAllocEx(hProc, NULL, size, flAllocationType, flProtect));
	CloseHandle(hProc);
	return ret;
}

void MemStub::FreeMem(uint32_t pid, uint64_t base, uint64_t size, uint32_t flAllocationType)
{
	auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProc)
	{
		return;
	}
		
	VirtualFreeEx(hProc, NULL, size, flAllocationType);
	CloseHandle(hProc);
}

size_t MemStub::QueryMem(uint32_t pid, uintptr_t lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, size_t dwLength)
{
	auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProc)
	{
		return 0;
	}
		
	size_t ret = VirtualQueryEx(hProc, reinterpret_cast<PVOID>(lpAddress), lpBuffer, dwLength);
	CloseHandle(hProc);
	return ret;
}

bool MemStub::ReadMemory(uint32_t pid, uint64_t ptr, uintptr_t buff, size_t size)
{
	auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	bool ret = ReadProcessMemory(hProc, reinterpret_cast<PVOID>(ptr), reinterpret_cast<PVOID>(buff), size, NULL);
	CloseHandle(hProc);
	return ret;
}

bool MemStub::WriteMemory(uint32_t pid, uint64_t Address, uintptr_t Buffer, uint32_t Size)
{
	auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProc)
	{
		return false;
	}
		
	bool ret = WriteProcessMemory(hProc, reinterpret_cast<PVOID>(Address), reinterpret_cast<PVOID>(Buffer), Size, NULL);
	CloseHandle(hProc);
	return ret;
}

std::string MemStub::GetModuleFullName(uint32_t pid, uint64_t base)
{
	auto hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProc)
	{
		return std::string();
	}
		
	char module_path[MAX_PATH]{};
	GetModuleFileNameExA(hProc, (HMODULE)base, module_path, MAX_PATH);
	return module_path;
}

HMODULE MemStub::GetProcessModuleHandle(uint32_t pid, const wchar_t* moduleName) 
{	
	MODULEENTRY32 moduleEntry;
	HANDLE handle = NULL;
	handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid); 
	if (!handle) 
	{
		CloseHandle(handle);
		return NULL;
	}

	ZeroMemory(&moduleEntry, sizeof(MODULEENTRY32));
	moduleEntry.dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(handle, &moduleEntry)) 
	{
		CloseHandle(handle);
		return NULL;
	}

	do 
	{
		if (lstrcmpW(moduleEntry.szModule, moduleName) == 0) 
		{ 
			return moduleEntry.hModule; 
		}

	} while (Module32Next(handle, &moduleEntry));

	CloseHandle(handle);
	return 0;
}

bool MemStub::RemoteInjectDLL(uint32_t PID, const char* Path)
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
		VirtualFreeEx(hProcess, PDllAddr, sizeof(ControlCmd), MEM_COMMIT);
		CloseHandle(hProcess);
		MessageBoxA(NULL, "CreateRemoteThread Error", "control", NULL);
		return false;
	}

	WaitForSingleObject(pRemoteThread, INFINITE);
	VirtualFreeEx(hProcess, PDllAddr, dwSize, MEM_COMMIT);
	CloseHandle(hProcess);
	return true;
}

bool MemStub::RemoteCallFunction(uint32_t PID, uint64_t addr, uintptr_t pargs, size_t args_size)
{
	auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (hProcess == NULL)
	{
		MessageBoxA(NULL, "OpenProcess Failed!", "control", NULL);
		return false;
	}

	//…Í«Î≤Œ ˝ƒ⁄¥Ê
	auto lpParam = VirtualAllocEx(hProcess, NULL, args_size, MEM_COMMIT, PAGE_READWRITE);
	if (lpParam == NULL)
	{
		MessageBoxA(NULL, "VirtualAllocEx Params Failed!", "control", NULL);
		return false;
	}

	if (!WriteProcessMemory(hProcess, lpParam, reinterpret_cast<PVOID>(pargs), args_size, NULL))
	{
		MessageBoxA(NULL, "Write Params Failed!", "control", NULL);
		return false;
	}


	HANDLE pRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)addr, lpParam, 0, NULL);
	if (pRemoteThread == NULL)
	{
		VirtualFreeEx(hProcess, lpParam, sizeof(ControlCmd), MEM_COMMIT);
		CloseHandle(hProcess);
		MessageBoxA(NULL, "CreateRemoteThread Error", "control", NULL);
		return false;
	}

	WaitForSingleObject(pRemoteThread, INFINITE);
	VirtualFreeEx(hProcess, lpParam, sizeof(ControlCmd), MEM_COMMIT);
	CloseHandle(hProcess);
	return true;
}

void MemStub::IsWow64(uint32_t pid, PBOOL ret)
{
	auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL)
	{
		*ret = false;
		return;
	}

	IsWow64Process(hProcess, ret);
	CloseHandle(hProcess);
	return;
}

NTSTATUS MemStub::NtQueryInformationProcess(uint32_t pid, int ProcessInformationClass, uintptr_t ProcessInformation, uint32_t ProcessInformationLength, uint32_t* ReturnLength)
{
	auto hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL)
	{
		return -1;
	}

	typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess)(HANDLE ProcessHandle, int ProcessInformationClass, uintptr_t ProcessInformation, uint32_t ProcessInformationLength, uint32_t* ReturnLength);
	fnNtQueryInformationProcess lpNtQueryInformationProcess = (fnNtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
	NTSTATUS ret = lpNtQueryInformationProcess(hProcess, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
	CloseHandle(hProcess);
	return ret;
}

uint64_t MemStub::GetDllExportRva(const char* module_path, const char* export_fun)
{
	auto base = GetModuleHandleA(module_path);
	if (base == NULL)
	{
		base = LoadLibraryA(module_path);
	}
		
	uint64_t ret = (uint64_t)(reinterpret_cast<uint8_t*>(GetProcAddress(base, export_fun)) - (uint8_t*)base);
	FreeLibrary(base);
	return ret;
}