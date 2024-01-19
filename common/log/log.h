#pragma once
#include <Windows.h>
#include <time.h>
#include <stdio.h>


#define MSG_LOG(...) Log::get_instance()->WriteLog(__FUNCTION__,__VA_ARGS__)

class Log
{
public:
	Log();
	~Log();
	static Log* get_instance();

	void enable_file_log() { isFile_ = TRUE; }

	template <typename... Args>
	void WriteLog(const char* function, const char* format, Args... args)
	{
		if (!hLogFile && isFile_)
		{
			char strLogFilePath[MAX_PATH] = { 0 };
			time_t tNow = time(nullptr);
			struct tm tmNow;
			localtime_s(&tmNow, &tNow);
			sprintf_s(strLogFilePath, MAX_PATH, "%04d-%02d-%02dT%02d-%02d-%02d_out.log", tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);

			hLogFile = CreateFileA(strLogFilePath,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				CREATE_ALWAYS,
				NULL,
				NULL);

			if (hLogFile == INVALID_HANDLE_VALUE)
			{
				char strError[MAX_PATH] = { 0 };
				sprintf_s(strError, MAX_PATH, "[%04d-%02d-%02dT%02d:%02d:%02d]<%u>  create log file failed,err:%d", tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec,
					GetCurrentThreadId(), GetLastError());
				OutputDebugStringA(strError);
			}
		}


		time_t  now = time(0);

		struct tm p;
		localtime_s(&p, &now);
		char buffer[2500] = { '\0' };
		sprintf_s(buffer, 2500, "[%04d-%02d-%02dT%02d:%02d:%02d]<%u> ", p.tm_year + 1900, p.tm_mon + 1, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec, GetCurrentThreadId());
		sprintf_s(buffer + strlen(buffer), 2500 - strlen(buffer), "%s: ", function);
		sprintf_s(buffer + strlen(buffer), 2500 - strlen(buffer), format, args...);
		sprintf_s(buffer + strlen(buffer), 2500 - strlen(buffer), "\n");
		EnterCriticalSection(&cs_);

		OutputDebugStringA(buffer);

		if (hLogFile && isFile_)
		{
			WriteFile(hLogFile, buffer, strlen(buffer),NULL, NULL);
		}

		LeaveCriticalSection(&cs_);
	}

private:
	static Log* s_instance_;

	BOOL isFile_=FALSE;
	HANDLE hLogFile;
	CRITICAL_SECTION cs_;

};



