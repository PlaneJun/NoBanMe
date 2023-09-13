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
	uint64_t region_start;
	uint64_t region_size;
	char disassembly[256];
}DbgBreakInfo, * PDbgBreakInfo;

typedef struct _THARD_BREAK
{
	uint64_t addr;
	dbg::DBG_SIZE size;
	dbg::DBG_TYPE type;
}HardBreak;

enum ECMD
{
	Pipe,
	SyscallMonitor,
	InitDbg,
	SetDrBreak,
	UnSetDrBreak,
	EnableDrBreak,
	DisableDrBreak
};

typedef struct _TCONTROL_CMD
{
	ECMD cmd;
	uint8_t dr_index;
	HardBreak hardbread;
	uint32_t threadid;
	bool syscall_state;
}ControlCmd, * PControlCmd;