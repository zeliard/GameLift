#pragma once


class IOThread
{
public:
	IOThread(HANDLE hThread, HANDLE hCompletionPort);
	~IOThread();

	DWORD Run();

	void DoIocpJob();
	void DoTimerJob();

	HANDLE GetHandle() { return mThreadHandle;  }
	
private:

	HANDLE mThreadHandle;
	HANDLE mCompletionPort;
};

