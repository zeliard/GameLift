#include "stdafx.h"
#include "ThreadLocal.h"
#include "Log.h"
#include "Timer.h"

__declspec(thread) int LThreadType = -1;
__declspec(thread) int LWorkerThreadId = -1;
__declspec(thread) ThreadCallHistory* LThreadCallHistory = nullptr;
__declspec(thread) void* LRecentThisPointer = nullptr;

__declspec(thread) Timer* LTimer = nullptr;
__declspec(thread) int64_t LTickCount = 0;

__declspec(thread) ExecuterListType* LExecuterList = nullptr;
__declspec(thread) AsyncExecutable*	LCurrentExecuterOccupyingThisThread = nullptr;

ThreadCallHistory* GThreadCallHistory[MAX_IOTHREAD] = {0, };

