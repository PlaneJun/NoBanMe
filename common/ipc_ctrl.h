#pragma once


enum EDataType : uint8_t
{
	API,
	DEBG
};

typedef struct _TAPI_MONITOR_INFO
{
	EDataType type;
	char function[256];
	char modulename[256];
	uint64_t retvale;
}ApiMonitorInfo, * PApiMonitorInfo;

typedef struct _THARD_BREAK_INFO
{
	EDataType type;
	uint8_t id;
	uint64_t addr;
	CONTEXT ctx;
	uint8_t stack[4096]; //4kb stack
	char disassembly[256];
}DbgBreakInfo, * PDbgBreakInfo;


typedef struct _THOOK_RESPONE
{
	uint64_t hook_addr;
	char ctx[208];
}HookRespone;

enum ECMD
{
	plugin_init,
	plugin_uninstall,
	pipe_client_connect,
	pipe_client_close,
	syscallmonitor_init,
	syscallmonitor_uninstall,
	veh_init,
	veh_uninstall,
	veh_set_dr,
	veh_unset_dr,
	veh_enable_dr,
	veh_disable_dr,
	hook_install,
	hook_uninstall
};

typedef struct _TCONTROL_CMD
{
	ECMD cmd;

	// syscall
	bool syscall_state;

	// veh
	uint8_t dr_index;
	struct
	{
		uint64_t addr;
		uint8_t size;
		uint8_t type;
	}hardbread;

	// hook
	struct
	{
		bool running;
		uint64_t address;
	}hookmsg;

}ControlCmd, * PControlCmd;