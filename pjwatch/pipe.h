#pragma once

class pipe
{
public:

	bool initPipe()
	{
		hPipe_ = CreateNamedPipe(pipename_, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 1, maxlen_, maxlen_, 0, NULL);
		if (hPipe_ == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		return true;
	}

	void clientConnPipe()
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

	bool write(void* s, size_t size)
	{
		return WriteFile(hPipe_, s, size, &WriteLen_, NULL);
	}

	bool read(char* buff) 
	{
		memset(buff, NULL, maxlen_);
		return ReadFile(hPipe_, buff, maxlen_, &ReadLen_, NULL);
	}

private:
	const wchar_t* pipename_ = L"\\\\.\\pipe\\pjark";
	const uint32_t maxlen_ = 8192;
	HANDLE hPipe_ = INVALID_HANDLE_VALUE;
	DWORD WriteLen_;
	DWORD ReadLen_;

};
