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


typedef struct _THARD_BREAK
{
    uint64_t addr;
    int size;
    int type;
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


typedef NTSTATUS(WINAPI* NTQUERYINFORMATIONTHREAD)(
    HANDLE ThreadHandle,
    ULONG ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength);

typedef enum _THREADINFOCLASS
{
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair_Reusable,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,   // Obsolete
    ThreadIsIoPending,
    ThreadHideFromDebugger,
    ThreadBreakOnTermination,
    ThreadSwitchLegacyState,
    ThreadIsTerminated,
    ThreadLastSystemCall,
    ThreadIoPriority,
    ThreadCycleTime,
    ThreadPagePriority,
    ThreadActualBasePriority,
    ThreadTebInformation,
    ThreadCSwitchMon,          // Obsolete
    ThreadCSwitchPmu,
    ThreadWow64Context,
    ThreadGroupInformation,
    ThreadUmsInformation,      // UMS
    ThreadCounterProfiling,
    ThreadIdealProcessorEx,
    MaxThreadInfoClass
} THREADINFOCLASS;
