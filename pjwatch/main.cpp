#include "pch.h"

std::mutex g_mutex;
bool g_lock = false;
pipe g_pipe;


void syscall_cb(PCONTEXT Context )
{
	if (!g_lock)
	{
		g_lock = true;
		uint8_t buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = { 0 };
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;
		uint64_t Displacement{};
		auto ret = SymFromAddr(GetCurrentProcess(), Context->R10, &Displacement, pSymbol);
		if (ret)
		{
			IMAGEHLP_MODULE64 image_module{};
			image_module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
			SymGetModuleInfo64(GetCurrentProcess(), Context->R10,&image_module);
			char buf[8192]{};
			ApiMonitorInfo apiinfo{};
			apiinfo.type = EDataType::API;
			strcpy_s(apiinfo.function, pSymbol->Name);
			strcpy_s(apiinfo.modulename, image_module.ModuleName);
			apiinfo.retvale = Context->Rax;
			g_pipe.write(& apiinfo,sizeof(ApiMonitorInfo));
		}
		g_lock = false;
	}
	
}

void ExceptionCb(uint8_t id,uint64_t addr,PCONTEXT ctx)
{
	g_mutex.lock();
	DbgBreakInfo dbginfo{};
	dbginfo.id = id;
	dbginfo.type = EDataType::DEBG;
	dbginfo.addr = addr;
	dbginfo.ctx = *ctx;

	auto start = reinterpret_cast<uint8_t*>(ctx->Rip);
	bool finded = false;
	while (true)
	{
		if (IsBadReadPtr(start, 8))
			break;
		else if (*(uint16_t*)start == 0xCCC3)
		{
			finded = true;
			break;
		}

		start--;
	}
	if (finded)
	{
		start += 2;
		dbginfo.region_start = (uint64_t)start;
		dbginfo.region_size = ctx->Rip - dbginfo.region_start;
	}
	else
	{
		dbginfo.region_start = dbginfo.addr;
		dbginfo.region_size = 0x1000;
	}
	g_pipe.write(&dbginfo, sizeof(DbgBreakInfo));
	g_mutex.unlock();
}

extern "C" __declspec(dllexport) void Dispatch(PControlCmd cmd)
{
	switch (cmd->cmd)
	{
		case Pipe:
		{
			g_pipe.clientConnPipe();
			break;
		}
		case SyscallMonitor:
		{
			static bool init = false;
			if (cmd->syscall_state)
			{
				if (!init)
				{
					//只初始化一次
					SymSetOptions(SYMOPT_UNDNAME);
					SymInitialize(GetCurrentProcess(), NULL, TRUE);
					InstrumentationCallback::Initialize();
					init = true;
				}
				InstrumentationCallback::AddCallback(syscall_cb);
			}
			else
			{
				InstrumentationCallback::RemoveCallback(syscall_cb);
			}
			break;
		}	
		case InitDbg:
		{
			dbg::init(cmd->threadid);
			dbg::set_callback(ExceptionCb);
			break;
		}
		case SetDrBreak:
		{
			int i = dbg::set_break(cmd->dr_index,cmd->hardbread.addr, cmd->hardbread.size, cmd->hardbread.type);
			printf("%d\n",i);
			break;
		}
		case UnSetDrBreak:
		{
			dbg::unset_hardbreak(cmd->dr_index);
			break;
		}
		case EnableDrBreak:
		{
			auto& dr = dbg::hbk_list[cmd->dr_index];
			dr.second = true;
			break;
		}
		case DisableDrBreak:
		{
			auto& dr = dbg::hbk_list[cmd->dr_index];
			dr.second = false;
			break;
		}
	}
}

void OnStart()
{
	
	AllocConsole();
	freopen_s((_iobuf**)__acrt_iob_func(0), "conin$", "r", (_iobuf*)__acrt_iob_func(0));
	freopen_s((_iobuf**)__acrt_iob_func(1), "conout$", "w", (_iobuf*)__acrt_iob_func(1));
	freopen_s((_iobuf**)__acrt_iob_func(2), "conout$", "w", (_iobuf*)__acrt_iob_func(2));
	{
		//auto hwnd = FindWindowA("UnrealWindow", "ShooterGame (64-bit Development PCD3D_SM5) ");
		//DWORD pid{};
		//auto threadid = GetWindowThreadProcessId(hwnd, &pid);
		//dbg::init(threadid);
		//dbg::set_callback(ExceptionCb);
		//auto base = (uint64_t)GetModuleHandleA(NULL) + 0x9D2C318;
		//auto ret = dbg::set_break(base, dbg::DBG_SIZE::SIZE_8, dbg::DBG_TYPE::TYPE_READWRITE);
	}

}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		//OnStart();
	}
	return TRUE;
}




