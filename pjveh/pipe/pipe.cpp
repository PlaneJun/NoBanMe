#include "pipe.h"


bool PipeCom::create(const wchar_t* pipe_name)
{
	if (hPipe_ == INVALID_HANDLE_VALUE)
	{
		hPipe_ = CreateNamedPipe(pipe_name, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 1, MAX_PIPE_BUFFER_SIZE, MAX_PIPE_BUFFER_SIZE, 0, NULL);
		if (GetLastError() != NULL)
		{
			return false;
		}
	}
	wcscpy_s(pipe_name_, pipe_name);
	return true;
}

bool PipeCom::close()
{
	if (hPipe_ != INVALID_HANDLE_VALUE)
	{
		DisconnectNamedPipe(hPipe_);
		CloseHandle(hPipe_);
		hPipe_ = INVALID_HANDLE_VALUE;
		return true;
	}

	return false;
}

void PipeCom::client_connect(const wchar_t* pipe_name)
{
	do
	{
		hPipe_ = CreateFile(pipe_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hPipe_ != INVALID_HANDLE_VALUE)
		{
			break;
		}
		Sleep(3000);
	} while (true);
}

bool PipeCom::client_disconnect(const wchar_t* pipe_name)
{
	hPipe_ = CreateFile(pipe_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe_ != INVALID_HANDLE_VALUE)
	{
		return false;
	}
	return CloseHandle(hPipe_);
}

bool PipeCom::write_buffer(char* s, size_t size)
{
	if (size > MAX_PIPE_BUFFER_SIZE)
		size = MAX_PIPE_BUFFER_SIZE;
	return WriteFile(hPipe_, s, size, &last_write_len_, NULL);
}

bool PipeCom::read_buffer(char* buff)
{
	memset(buff, 0, MAX_PIPE_BUFFER_SIZE);
	return ReadFile(hPipe_, buff, MAX_PIPE_BUFFER_SIZE, &last_read_len_, NULL);
}
