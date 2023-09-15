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
        int size;
        int type;
    }hardbread;
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
