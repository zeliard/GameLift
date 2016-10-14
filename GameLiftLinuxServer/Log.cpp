#include "Log.h"

#include <iostream>
#include <stdarg.h>  

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

	char buf[128] = {};

	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	printf("%s", buf);

	if (fileOut)
	{
		mLogFileStream << buf;
		mLogFileStream.flush();
	}
}

std::unique_ptr<ConsoleLog> GConsoleLog(nullptr);

