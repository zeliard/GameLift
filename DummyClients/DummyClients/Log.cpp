#include "stdafx.h"
#include "Log.h"

#include <iostream>

void ThreadCallHistory::DumpOut(std::ostream& ost)
{
	uint64_t count = mCounter < MAX_HISTORY ? mCounter : MAX_HISTORY;

	ost << "===== Recent Call History [Thread:" << mThreadId << "]" << std::endl ;

	for (int i = 1; i <= count; ++i)
	{
		ost << "   " << mHistory[(mCounter-i) % MAX_HISTORY] << std::endl;
	}
	ost << "===== End of Recent Call History" << std::endl << std::endl;
}
	
	
namespace LoggerUtil
{
	LogEvent gLogEvents[MAX_LOG_SIZE];
	__int64 gCurrentLogIndex = 0;
}

ConsoleLog::ConsoleLog(const char* filename)
{
	mLogFileStream.open(filename, std::ofstream::out);
}

ConsoleLog::~ConsoleLog()
{
	mLogFileStream.close();
}

void ConsoleLog::PrintOut(bool fileOut, const char* fmt, ...)
{
	FastSpinlockGuard lock(mLock);

	char buf[512] = {};

	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, fmt, args);
	va_end(args);

	printf_s("%s", buf);

	if (fileOut)
	{
		mLogFileStream << buf;
		mLogFileStream.flush();
	}
}

ConsoleLog* GConsoleLog = nullptr;

