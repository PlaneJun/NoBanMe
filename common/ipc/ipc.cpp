#include "ipc.h"
#include <windows.h>
#include "../log/log.h"


IPCCom::IPCCom()
{

}

IPCCom::~IPCCom()
{
	close();
}

bool IPCCom::create_sharemem()
{
	hShareMem_ = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_EXECUTE_READWRITE, 0, MAX_IPC_BUFFER_SIZE, sharemem_name_);;
	if (hShareMem_ == NULL)
	{
		MSG_LOG("create sharemem fail: err %d", GetLastError());
		return false;
	}

	ShareMemBuffer_ = MapViewOfFile(hShareMem_, FILE_MAP_ALL_ACCESS, 0, 0, MAX_IPC_BUFFER_SIZE);
	if (ShareMemBuffer_ == NULL)
	{
		MSG_LOG("map sharemem fail: err %d", GetLastError());
		CloseHandle(hShareMem_);
		return false;
	}
	MSG_LOG("shareMem ptr:%p", ShareMemBuffer_);
	return true;
}

bool IPCCom::create_semaphore()
{
	hSemaphore_ = CreateSemaphoreW(NULL, 0, 1, semaphore_name_);
	if (hSemaphore_ == NULL)
	{
		MSG_LOG("create semaphore fail: err %d", GetLastError());
		return false;
	}
	return true;
}

bool IPCCom::create()
{
	if (create_sharemem())
	{
		if (!create_semaphore())
		{
			close();
			return false;
		}
		return true;
	}
	return false;
}

void IPCCom::close()
{
	if (hSemaphore_ != nullptr) {
		ReleaseSemaphore(hSemaphore_, 1, NULL);
		CloseHandle(hSemaphore_);
	}

	if (ShareMemBuffer_) {
		UnmapViewOfFile(ShareMemBuffer_);
	}

	if (hShareMem_) {
		UnmapViewOfFile(hShareMem_);
	}

}

bool IPCCom::client_connect()
{
	MSG_LOG("enter");

	bool ret = create_sharemem() && create_semaphore();
			
	MSG_LOG("end");
	return ret;
}

bool IPCCom::client_disconnect()
{
	return true;
}

bool IPCCom::write_buffer(char* s, size_t size)
{
	if (!hSemaphore_ || !ShareMemBuffer_)
		return false;

	if (size > MAX_IPC_BUFFER_SIZE)
		size = MAX_IPC_BUFFER_SIZE;

	BOOL ret = TRUE;
	RtlCopyMemory(ShareMemBuffer_, s, MAX_IPC_BUFFER_SIZE);
	ReleaseSemaphore(hSemaphore_, 1, NULL); // 写数据之后通知读操作,i done
	return ret;
}

bool IPCCom::read_buffer(char* buff)
{
	if (!hSemaphore_ || !ShareMemBuffer_)
		return false;

	memset(buff, 0, MAX_IPC_BUFFER_SIZE);
	BOOL ret = TRUE;
	
	// 堵塞等待写操作
	if (WaitForSingleObject(hSemaphore_, INFINITE) == WAIT_OBJECT_0)
	{
		RtlCopyMemory(buff, ShareMemBuffer_, MAX_IPC_BUFFER_SIZE);
	}
	
	return ret;
}
