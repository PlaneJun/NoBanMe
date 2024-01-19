#pragma once
#include <Windows.h>
#include <stdint.h>

#define MAX_PIPE_BUFFER_SIZE 8192
class PipeCom
{
public:

	bool create();

	bool close();

	bool client_connect();

	bool client_disconnect();

	bool write_buffer(char* s, size_t size);

	bool read_buffer(char* buff);

	template<typename T>
	void write(T data)
	{
		write_buffer(&data, sizeof(T));
	}

	template<typename T>
	T read()
	{
		char ret[MAX_PIPE_BUFFER_SIZE]{};
		read_buffer(&ret);
		return *(T*)&ret[0];
	}

	DWORD get_last_write_len() const
	{
		return last_write_len_;
	}

	DWORD get_last_read_len() const
	{
		return last_read_len_;
	}

private:
	wchar_t pipe_name_[MAX_PATH]= L"\\\\.\\pipe\\NoBanMe";
	HANDLE hPipe_ = INVALID_HANDLE_VALUE;
	DWORD last_write_len_;
	DWORD last_read_len_;
};
