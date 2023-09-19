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
	uint8_t stack[1024]; //1mb stack
	uint64_t region_start;
	uint64_t region_size;
	char disassembly[256];
}DbgBreakInfo, * PDbgBreakInfo;

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
	veh_disable_dr
};

typedef struct _TCONTROL_CMD
{
	ECMD cmd;
	uint8_t dr_index;
	struct
	{
		uint64_t addr;
		dbg::DBG_SIZE size;
		dbg::DBG_TYPE type;
	}hardbread;
	bool syscall_state;
}ControlCmd, * PControlCmd;