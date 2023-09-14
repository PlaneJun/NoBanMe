#pragma once

namespace dbg
{
	using cbDbgHandler = void(*)(uint8_t,uint64_t, PCONTEXT);

	enum DBG_TYPE
	{
		TYPE_EXECUTE,
		TYPE_READWRITE,
		TYPE_WRITE
	};
	enum DBG_SIZE
	{
		SIZE_1,
		SIZE_2,
		SIZE_4,
		SIZE_8
	};

	struct DBG_PARAMS
	{
		uint8_t index;
		DBG_TYPE type;
		DBG_SIZE size;
		uint64_t address;
	};

	cbDbgHandler callback;
	std::map<uint8_t, std::pair<struct DBG_PARAMS, bool>> hbk_list;

	LONG NTAPI ExceptionHandler(_EXCEPTION_POINTERS* ExceptionInfo)
	{
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP)
		{
			//printf("%p\n", ExceptionInfo->ContextRecord->Rip);
			uint8_t index = 0xFF;
			auto dr6 = ExceptionInfo->ContextRecord->Dr6;
			if (dr6 & 1)
				index = 0;
			else if (dr6 & 2)
				index = 1;
			else if (dr6 & 4)
				index = 2;
			else if (dr6 & 8)
				index = 3;
			
			if (hbk_list.count(index)>0 && hbk_list[index].second)
			{
				if (callback)
					callback(index, hbk_list[index].first.address, ExceptionInfo->ContextRecord);
			}		
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}

	void init()
	{
		//Dr0-Dr3
		hbk_list[0] = {};
		hbk_list[1] = {};
		hbk_list[2] = {};
		hbk_list[3] = {};

		auto kernel = GetModuleHandleA("Kernel32.dll");
		if (kernel)
		{
			auto RtlAddVectoredExceptionHandler = (uintptr_t)GetProcAddress(kernel, "AddVectoredContinueHandler") + 0x06;
			if (RtlAddVectoredExceptionHandler)
			{
				auto offset = *(int32_t*)(RtlAddVectoredExceptionHandler + 1);
				RtlAddVectoredExceptionHandler = RtlAddVectoredExceptionHandler + offset + 5;
				if (RtlAddVectoredExceptionHandler)
				{
					reinterpret_cast<void(*)(uint32_t, PVECTORED_EXCEPTION_HANDLER, uint32_t)>(RtlAddVectoredExceptionHandler)(1, ExceptionHandler, 0);
				}
			}
		}
	}

	void set_bits(uint64_t& dw, int lowBit, int bits, int newValue)
	{
		uint64_t mask = (1 << bits) - 1;
		dw = (dw & ~(mask << lowBit)) | (newValue << lowBit);
	}

	void set_callback(cbDbgHandler cb)
	{
		callback = cb;
	}

	int set_break(uint8_t dr_index,uint64_t addr, DBG_SIZE len, DBG_TYPE type)
	{
		if (IsBadReadPtr((PVOID)addr,8))
			return -1;
		if (len < SIZE_1 || len >SIZE_8)
			return -1;
		if (type < TYPE_EXECUTE || type >TYPE_WRITE)
			return -1;

		//给所有线程都下断点
		HANDLE hThreadShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
		THREADENTRY32* threadInfo = new THREADENTRY32;
		threadInfo->dwSize = sizeof(THREADENTRY32);
		while (Thread32Next(hThreadShot, threadInfo) != FALSE)
		{
			if (GetCurrentProcessId() == threadInfo->th32OwnerProcessID)
			{
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadInfo->th32ThreadID);
				SuspendThread(hThread);
				CONTEXT ctx{};
				ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS | CONTEXT_FULL;//设置线程上下文的类型
				if (!GetThreadContext(hThread, &ctx))//通过句柄获取进程的上下文
				{
					return -3;
				}
				if (dr_index == 0)
				{
					ctx.Dr0 = addr;
				}
				else if (dr_index == 1)
				{
					ctx.Dr1 = addr;
				}
				else if (dr_index == 2)
				{
					ctx.Dr2 = addr;
				}
				else if (dr_index == 3)
				{
					ctx.Dr3 = addr;
				}
				ctx.Dr6 = 0;
				int st = 0;
				if (type == TYPE_EXECUTE)
					st = 0;
				if (type == TYPE_READWRITE)
					st = 3;
				if (type == TYPE_WRITE)
					st = 1;

				int le = 0;
				if (len == SIZE_1)
					le = 0;
				if (len == SIZE_2)
					le = 1;
				if (len == SIZE_4)
					le = 3;
				if (len == SIZE_8)
					le = 2;

				set_bits(ctx.Dr7, 16 + dr_index * 4, 2, st);
				set_bits(ctx.Dr7, 18 + dr_index * 4, 2, le);
				set_bits(ctx.Dr7, dr_index * 2, 1, 1);
				//ctx.Dr7 = 1;
				if (!SetThreadContext(hThread, &ctx))
				{
					return -4;
				}
				ResumeThread(hThread);
				CloseHandle(hThread);
				
			}
		}
		CloseHandle(hThreadShot);
		hbk_list[dr_index] = { { dr_index ,type ,len,addr},false };
		return dr_index;
	}

	bool unset_hardbreak(uint8_t dr_index)
	{
		if (dr_index < 0 && dr_index>3)
			return false;

		HANDLE hThreadShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
		THREADENTRY32* threadInfo = new THREADENTRY32;
		threadInfo->dwSize = sizeof(THREADENTRY32);
		while (Thread32Next(hThreadShot, threadInfo) != FALSE)
		{
			if (GetCurrentProcessId() == threadInfo->th32OwnerProcessID)
			{
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadInfo->th32ThreadID);
				SuspendThread(hThread);

				CONTEXT ctx{};
				ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS | CONTEXT_FULL;
				if (!GetThreadContext(hThread, &ctx))
				{
					return false;
				}

				int FlagBit = 0;
				if (dr_index == 0)
				{
					FlagBit = 0;
					ctx.Dr0 = 0;
				}
				if (dr_index == 1)
				{
					FlagBit = 2;
					ctx.Dr1 = 0;
				}
				if (dr_index == 2)
				{
					FlagBit = 4;
					ctx.Dr2 = 0;
				}
				if (dr_index == 3)
				{
					FlagBit = 6;
					ctx.Dr3 = 0;
				}
				ctx.Dr7 &= ~(1 << FlagBit);

				if (!SetThreadContext(hThread, &ctx))
				{
					return false;
				}
				ResumeThread(hThread);
				CloseHandle(hThread);
			}
		}
		hbk_list[dr_index] = { {},false };
		return true;
	}
}