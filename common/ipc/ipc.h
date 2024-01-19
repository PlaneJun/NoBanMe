#pragma once
#include <Windows.h>
#include <stdint.h>

#define MAX_IPC_BUFFER_SIZE 8192
class IPCCom
{
public:
	IPCCom();
	~IPCCom();

	bool create();

	void close();

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
		char ret[MAX_IPC_BUFFER_SIZE]{};
		read_buffer(&ret);
		return *(T*)&ret[0];
	}


private:

	bool create_sharemem();

	bool create_semaphore();
private:
	wchar_t semaphore_name_[MAX_PATH] = L"Semaphore_NoBanMe";
	wchar_t sharemem_name_[MAX_PATH] = L"ShareMem_NoBanMe";
	HANDLE hSemaphore_;
	HANDLE hShareMem_ = NULL;
	LPVOID ShareMemBuffer_;
};
