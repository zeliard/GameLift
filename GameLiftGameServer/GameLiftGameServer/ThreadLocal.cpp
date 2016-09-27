#include "stdafx.h"
#include "ThreadLocal.h"
#include "Timer.h"

__declspec(thread) int LThreadType = -1;
__declspec(thread) int LWorkerThreadId = -1;

__declspec(thread) int64_t LTickCount = 0;
__declspec(thread) Timer* LTimer = nullptr;
