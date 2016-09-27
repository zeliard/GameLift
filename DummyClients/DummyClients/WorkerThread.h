#pragma once


class WorkerThread
{
public:
	WorkerThread(int workerThreadId, HANDLE hThread, HANDLE hCompletionPort);
	~WorkerThread();

	DWORD Run();

	bool DoIocpJob(); ///< false means End 

	HANDLE GetHandle() { return mThreadHandle;  }
	
private:

	HANDLE mThreadHandle;
	HANDLE mCompletionPort;
	int mWorkerThreadId;
};

