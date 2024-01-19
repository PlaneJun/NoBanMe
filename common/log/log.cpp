#include "log.h"

Log* Log::s_instance_ = NULL;

Log * Log::get_instance()
{
	if (!s_instance_)
		s_instance_ = new Log();
	return s_instance_;
}

Log::~Log()
{
	if (hLogFile)
		CloseHandle(hLogFile);
}

Log::Log()
{
	InitializeCriticalSection(&cs_); 
}


