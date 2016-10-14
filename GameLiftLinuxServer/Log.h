#pragma once

#include <fstream>
#include <memory>
#include "Utils.h"

class ConsoleLog
{
public:
	ConsoleLog(const char* filename);
	~ConsoleLog();

	void PrintOut(bool fileOut, const char* fmt, ...);

private:

	std::ofstream mLogFileStream;
};

extern std::unique_ptr<ConsoleLog> GConsoleLog;
