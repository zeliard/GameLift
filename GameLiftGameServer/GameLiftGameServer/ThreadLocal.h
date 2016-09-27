#pragma once

#define MAX_WORKER_THREAD	8

enum THREAD_TYPE
{
	THREAD_MAIN,
	THREAD_IO_WORKER
};

class Timer;

extern __declspec(thread) int LThreadType;
extern __declspec(thread) int LWorkerThreadId;

extern __declspec(thread) int64_t LTickCount;
extern __declspec(thread) Timer* LTimer;

