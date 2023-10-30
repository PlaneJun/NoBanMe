#include <Windows.h>
#include <winternl.h>
#include <winnt.h>
#include <stdint.h>
#pragma comment (lib, "imagehlp.lib")
#pragma comment( lib, "ntdll.lib" )

#define MAX_INSTRUMENTATION_CALLBACKS 64

EXTERN_C
{
	NTSTATUS NTSYSAPI NtSetInformationProcess(HANDLE ProcessHandle,PROCESS_INFORMATION_CLASS ProcessInformationClass,LPVOID ProcessInformation,DWORD ProcessInformationSize);
	void InstrumentationCallbackEntry();
	void RestoreCallbackContext(PCONTEXT ContextRecord);
	void InstrumentationCallbackHandler(PCONTEXT PreviousContext);
}

typedef struct _INSTRUMENTATION_CALLBACK_INFORMATION
{
	ULONG Version;
	ULONG Reserved;
	PVOID Callback;
}INSTRUMENTATION_CALLBACK_INFORMATION, *PINSTRUMENTATION_CALLBACK_INFORMATION;


namespace InstrumentationCallback
{
	using  VECTORED_INSTRUMENTATION_CALLBACK = VOID(WINAPI*)(PCONTEXT PreviousContext);
	VECTORED_INSTRUMENTATION_CALLBACK Callbacks[MAX_INSTRUMENTATION_CALLBACKS];
	CHAR bCallbackListSpinlock = NULL;
	
	NTSTATUS Initialize()
	{
		RtlZeroMemory(InstrumentationCallback::Callbacks, sizeof(InstrumentationCallback::Callbacks));
		INSTRUMENTATION_CALLBACK_INFORMATION CallbackInfo{};
		CallbackInfo.Callback = InstrumentationCallbackEntry;
		return NtSetInformationProcess(GetCurrentProcess(), (PROCESS_INFORMATION_CLASS)40, &CallbackInfo, sizeof(CallbackInfo));
	}

	bool AddCallback(VECTORED_INSTRUMENTATION_CALLBACK Callback)
	{
		while (_InterlockedExchange8(&bCallbackListSpinlock, TRUE) == TRUE)
		{
			_mm_pause();
		}

		bool Result = FALSE;
		for (uint32_t i = 0; i < MAX_INSTRUMENTATION_CALLBACKS; i++)
		{
			if (InstrumentationCallback::Callbacks[i] == NULL)
			{
				InstrumentationCallback::Callbacks[i] = Callback;
				Result = TRUE;
				break;
			}
		}

		bCallbackListSpinlock = FALSE;
		return Result;
	}

	bool RemoveCallback(VECTORED_INSTRUMENTATION_CALLBACK Callback)
	{
		while (_InterlockedExchange8(&bCallbackListSpinlock, TRUE) == TRUE)
		{
			_mm_pause();
		}

		bool Result = FALSE;
		for (uint32_t i = NULL; i < MAX_INSTRUMENTATION_CALLBACKS; i++)
		{
			if (InstrumentationCallback::Callbacks[i] == Callback)
			{
				InstrumentationCallback::Callbacks[i] = NULL;
				Result = TRUE;
				break;
			}
		}

		bCallbackListSpinlock = FALSE;
		return Result;
	}

	bool Remove()
	{
		INSTRUMENTATION_CALLBACK_INFORMATION CallbackInfo{};
		CallbackInfo.Callback = NULL;
		return NtSetInformationProcess(GetCurrentProcess(), (PROCESS_INFORMATION_CLASS)40, &CallbackInfo, sizeof(CallbackInfo)) == 0;
	}
}

void InstrumentationCallbackHandler(PCONTEXT PreviousContext)
{
	auto InstrumentationCallbackDisabled = reinterpret_cast<PCHAR>(__readgsqword(0x30) + 0x2EC);
	if (_InterlockedExchange8(InstrumentationCallbackDisabled, TRUE) == FALSE)
	{
		for (uint32_t i = NULL; i < MAX_INSTRUMENTATION_CALLBACKS; i++)
		{
			if (InstrumentationCallback::Callbacks[i] != NULL) {
				InstrumentationCallback::Callbacks[i](PreviousContext);
			}
		}

		*InstrumentationCallbackDisabled = FALSE;
	}
	RtlRestoreContext(PreviousContext, NULL);
}



