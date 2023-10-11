#include "plugin.h"

bool plugin::init_pipe()
{
	if (hPipe_ == INVALID_HANDLE_VALUE)
	{
		hPipe_ = CreateNamedPipe(pipename_, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 1, maxlen_, maxlen_, 0, NULL);
		if (hPipe_ == INVALID_HANDLE_VALUE)
		{
			return false;
		}
	}
	return true;
}

bool plugin::close_pipe()
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

void plugin::client_connect_pipe()
{
	do
	{
		hPipe_ = CreateFile(pipename_, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hPipe_ != INVALID_HANDLE_VALUE)
		{
			break;
		}
	} while (true);
}

void plugin::client_disconnect_pipe()
{
	if (hPipe_)
		CloseHandle(hPipe_);
}

bool plugin::write_pipe(void* s, size_t size)
{
	return WriteFile(hPipe_, s, size, &WriteLen_, NULL);
}

bool plugin::read_pipe(char* buff) {
	memset(buff, NULL, maxlen_);
	return ReadFile(hPipe_, buff, maxlen_, &ReadLen_, NULL);
}
