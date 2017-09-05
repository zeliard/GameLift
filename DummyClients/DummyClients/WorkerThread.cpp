#include "stdafx.h"
#include "Exception.h"
#include "ThreadLocal.h"
#include "Timer.h"
#include "DummyClients.h"
#include "WorkerThread.h"
#include "PlayerSession.h"
#include "IocpManager.h"
#include "Log.h"

WorkerThread::WorkerThread(int workerThreadId, HANDLE hThread, HANDLE hCompletionPort)
: mWorkerThreadId(workerThreadId), mThreadHandle(hThread), mCompletionPort(hCompletionPort)
{
}


WorkerThread::~WorkerThread()
{
	CloseHandle(mThreadHandle);
}

DWORD WorkerThread::Run()
{

	while (true)
	{
		if ( false == DoIocpJob() )
			break;

		/// Do Dimer Tasks
		LTimer->DoTimerJob();
	
	}

	return 1;
}

bool WorkerThread::DoIocpJob()
{
	DWORD dwTransferred = 0;
	OverlappedIOContext* context = nullptr;
	ULONG_PTR completionKey = 0;

	int ret = GetQueuedCompletionStatus(mCompletionPort, &dwTransferred, (PULONG_PTR)&completionKey, (LPOVERLAPPED*)&context, GQCS_TIMEOUT);

	if (CK_IOCP_TASK == completionKey)
	{
		PlayerSession* session = reinterpret_cast<PlayerSession*>(context);
		session->TrackMatchMaking();
		return true;
	}

	Session* remote = context ? context->mSessionObject : nullptr;

	if (ret == 0 || dwTransferred == 0)
	{
		/// stop requested
		if (completionKey == CK_STOP_NOW)
			return false; 

		/// check time out first 
		if ( context == nullptr && GetLastError() == WAIT_TIMEOUT)
			return true;

		if (context->mIoType == IO_RECV || context->mIoType == IO_SEND)
		{
			CRASH_ASSERT(nullptr != remote);

			/// In most cases in here: ERROR_NETNAME_DELETED(64)

			remote->DisconnectRequest(DR_COMPLETION_ERROR);

			DeleteIoContext(context);

			return true;
		}
	}

	CRASH_ASSERT(nullptr != remote);

	bool completionOk = false;
	switch (context->mIoType)
	{
	case IO_CONNECT:
		dynamic_cast<PlayerSession*>(remote)->ConnectCompletion();
		completionOk = true;
		break;

	case IO_DISCONNECT:
		remote->DisconnectCompletion(static_cast<OverlappedDisconnectContext*>(context)->mDisconnectReason);
		completionOk = true;
		break;

	case IO_RECV_ZERO:
		completionOk = remote->PostRecv();
		break;

	case IO_SEND:
		remote->SendCompletion(dwTransferred);

		if (context->mWsaBuf.len != dwTransferred)
			GConsoleLog->PrintOut(false, "Partial SendCompletion requested [%d], sent [%d]\n", context->mWsaBuf.len, dwTransferred);
		else
			completionOk = true;
		
		break;

	case IO_RECV:
		remote->RecvCompletion(dwTransferred);
	
		
		completionOk = remote->PreRecv();

		break;

	default:
		GConsoleLog->PrintOut(false, "Unknown I/O Type: %d\n", context->mIoType);
		CRASH_ASSERT(false);
		break;
	}

	if (!completionOk)
	{
		/// connection closing
		remote->DisconnectRequest(DR_IO_REQUEST_ERROR);
	}

	DeleteIoContext(context);
	
	return true;
}


