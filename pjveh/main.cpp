#include <mutex>
#include <Windows.h>
#include <DbgHelp.h>
#include "InstrumentationCallback.hpp"
#include "vehdbg.h"
#include "pipe.h"
#include "defs.h"


HMODULE g_hModule;
PipeCom g_plugin;
vehdbg g_vehdbg;

#define PIPE_NAME L"\\\\.\\pipe\\pjark"

void syscall_cb(PCONTEXT Context )
{
	static bool init = false;
	if (!init)
	{
		init = true;
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
			g_plugin.write_buffer((char*) & apiinfo, sizeof(ApiMonitorInfo));
		}
		init = false;
	}
	
}

void vehexception_cb(uint8_t id,PCONTEXT& ctx)
{
	static std::mutex lock;
	std::pair<vehdbg::DBG_PARAMS, bool> hbk{};
	if (g_vehdbg.get_hardBreak_by_index(id, hbk) && hbk.second)
	{
		lock.lock();
		DbgBreakInfo dbginfo{};
		dbginfo.id = id;
		dbginfo.type = EDataType::DEBG;
		dbginfo.addr = hbk.first.address;
		dbginfo.ctx = *ctx;
		memcpy(dbginfo.stack, (PVOID)ctx->Rsp, sizeof(dbginfo.stack));
		if (hbk.first.type == vehdbg::DBG_TYPE::TYPE_EXECUTE)
		{
			//取消断点
			if (id == 0)
				ctx->Dr7 &= ~1;
			else if (id == 1)
				ctx->Dr7 &= ~2;
			else if (id == 2)
				ctx->Dr7 &= ~0x10;
			else if (id == 3)
				ctx->Dr7 &= ~0x40;
		}
		g_plugin.write_buffer((char*)&dbginfo, sizeof(DbgBreakInfo));
		lock.unlock();
	}
}

extern "C" __declspec(dllexport) void Dispatch(PControlCmd cmd)
{
	switch (cmd->cmd)
	{
		case plugin_init:
		{
			break;
		}
		case plugin_uninstall:
		{
			FreeLibraryAndExitThread(g_hModule,0);
			break;
		}
		case pipe_client_connect:
		{
			g_plugin.client_disconnect(PIPE_NAME);
			break;
		}
		case syscallmonitor_init:
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
		case syscallmonitor_uninstall:
		{
			InstrumentationCallback::Remove();
			break;
		}
		case veh_init:
		{
			g_vehdbg.init();
			g_vehdbg.set_callback(vehexception_cb);
			break;
		}
		case veh_uninstall:
		{
			for (int i = 0; i < 4; i++)
			{
				g_vehdbg.unset_hardbreak(i);
				Sleep(500);
			}
			g_vehdbg.close();
			break;
		}
		case pipe_client_close:
		{
			g_plugin.client_connect(PIPE_NAME);
			break;
		}
		case veh_set_dr:
		{
			int i = g_vehdbg.set_break(cmd->dr_index,cmd->hardbread.addr, cmd->hardbread.size, cmd->hardbread.type);
			break;
		}
		case veh_unset_dr:
		{
			g_vehdbg.unset_hardbreak(cmd->dr_index);
			break;
		}
		case veh_enable_dr:
		{
			
			std::pair<vehdbg::DBG_PARAMS, bool> ret{};
			if (g_vehdbg.get_hardBreak_by_index(cmd->dr_index, ret))
			{
				ret.second = true;
			}
			break;
		}
		case veh_disable_dr:
		{
			std::pair<vehdbg::DBG_PARAMS, bool> ret{};
			if (g_vehdbg.get_hardBreak_by_index(cmd->dr_index, ret))
			{
				ret.second = false;
			}
			break;
		}
	}
}

void test()
{
	
	AllocConsole();
	freopen_s((_iobuf**)__acrt_iob_func(0), "conin$", "r", (_iobuf*)__acrt_iob_func(0));
	freopen_s((_iobuf**)__acrt_iob_func(1), "conout$", "w", (_iobuf*)__acrt_iob_func(1));
	freopen_s((_iobuf**)__acrt_iob_func(2), "conout$", "w", (_iobuf*)__acrt_iob_func(2));
	{
		//dbg::init();
		//dbg::set_callback(vehexception_cb);
		//auto base = (uint64_t)GetModuleHandleA(NULL) + 0x5A0D7FF;
		//auto ret = dbg::set_break(0,base, dbg::DBG_SIZE::SIZE_1, dbg::DBG_TYPE::TYPE_EXECUTE);
		//printf("set :%d\n",ret);
	}

}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		g_hModule = hModule;
		
	}
	return TRUE;
}




