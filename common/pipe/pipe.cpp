#include "pipe.h"
#include <windows.h>
#include "../log/log.h"


bool PipeCom::create()
{
	hPipe_ = CreateNamedPipeW(pipe_name_, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 1, MAX_PIPE_BUFFER_SIZE, MAX_PIPE_BUFFER_SIZE, 0, NULL);
	if (hPipe_ == INVALID_HANDLE_VALUE)
	{
		MSG_LOG("create pipe fail: err %d", GetLastError());
		return false;
	}

	return true;
}

bool PipeCom::close()
{
	bool state = true;
	if (hPipe_ != INVALID_HANDLE_VALUE)
	{
		DisconnectNamedPipe(hPipe_);
		CloseHandle(hPipe_);
		state = false;
	}

	return state;
}

bool PipeCom::client_connect()
{
	MSG_LOG("enter");

	bool state = true;
	do
	{
		if (!WaitNamedPipeW(pipe_name_, 100))
		{
			MSG_LOG("wait fail.");
			state = false;
			break;
		}

		hPipe_ = CreateFileW(pipe_name_, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
		if (hPipe_ == INVALID_HANDLE_VALUE)
		{
			MSG_LOG("open pipe fail.");
			state = false;
			break;
		}

	} while (FALSE);

	MSG_LOG("end");
	return state;
}

bool PipeCom::client_disconnect()
{
	return CloseHandle(hPipe_);
}

bool PipeCom::write_buffer(char* s, size_t size)
{
	if (size > MAX_PIPE_BUFFER_SIZE)
		size = MAX_PIPE_BUFFER_SIZE;

	BOOL ret = WriteFile(hPipe_, s, size, &last_write_len_, NULL);

	return ret;
}

bool PipeCom::read_buffer(char* buff)
{
	MSG_LOG("wait");
	memset(buff, 0, MAX_PIPE_BUFFER_SIZE);
	BOOL ret = ReadFile(hPipe_, buff, MAX_PIPE_BUFFER_SIZE, &last_read_len_, NULL);
	return ret;
}
