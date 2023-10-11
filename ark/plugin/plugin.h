#pragma once
#include <Windows.h>
#include <stdint.h>

class plugin
{
public:

	bool init_pipe();

	bool close_pipe();

	void client_connect_pipe();

	void client_disconnect_pipe();

	bool write_pipe(void* s, size_t size);

	bool read_pipe(char* buff);

private:
	const wchar_t* pipename_ = L"\\\\.\\pipe\\pjark";
	const uint32_t maxlen_ = 8192;
	HANDLE hPipe_ = INVALID_HANDLE_VALUE;
	DWORD WriteLen_;
	DWORD ReadLen_;

};
