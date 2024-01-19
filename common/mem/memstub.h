#pragma once
#include <string>
#include <Windows.h>
#include <stdint.h>


class MemStub
{
public:

	template<typename T>
	T Read(uint32_t pid, uint64_t ptr)
	{
		T buff;
		ReadMemory(pid, ptr, &buff, sizeof(T));
		return buff;
	}

	template <typename T>
	bool Write(uint32_t pid, uint64_t addr, T Val)
	{
		return WriteMemory(pid, addr, &Val, sizeof(T));
	}
	
	static uint64_t AllocMem(uint32_t pid, uint64_t base, uint64_t size, uint32_t flAllocationType, uint32_t flProtect);

	static void FreeMem(uint32_t pid, uint64_t base, uint64_t size, uint32_t flAllocationType);

	static size_t QueryMem(uint32_t pid, uintptr_t lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, size_t dwLength);

	static bool ReadMemory(uint32_t pid, uint64_t ptr, uintptr_t buff, size_t size);

	static bool WriteMemory(uint32_t pid, uint64_t Address, uintptr_t Buffer, uint32_t Size);

	static std::string GetModuleFullName(uint32_t pid, uint64_t base);

	static HMODULE GetProcessModuleHandle(uint32_t pid, const wchar_t* moduleName);

	static bool RemoteInjectDLL(uint32_t PID, const char* Path);

	static bool RemoteCallFunction(uint32_t PID, uint64_t addr, uintptr_t pargs, size_t args_size);

	static void IsWow64(uint32_t pid, PBOOL ret);

	static NTSTATUS NtQueryInformationProcess(uint32_t pid, int ProcessInformationClass, uintptr_t ProcessInformation, uint32_t ProcessInformationLength, uint32_t* ReturnLength);

	static uint64_t GetDllExportRva(const char* module_path, const char* export_fun);

};
