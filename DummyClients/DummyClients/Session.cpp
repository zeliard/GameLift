#include "stdafx.h"
#include "Exception.h"
#include "ThreadLocal.h"
#include "Log.h"
#include "DummyClients.h"
#include "OverlappedIOContext.h"
#include "Session.h"
#include "IocpManager.h"


Session::Session(size_t sendBufSize, size_t recvBufSize) 
: mSendBuffer(sendBufSize), mRecvBuffer(recvBufSize), mConnected(0), mSendRequestedCount(0)
{
	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}



void Session::DisconnectRequest(DisconnectReason dr)
{
	TRACE_THIS;

	/// already disconnected?
	if (0 == InterlockedExchange(&mConnected, 0))
		return;

	OverlappedDisconnectContext* context = new OverlappedDisconnectContext(this, dr);

	if (FALSE == DisconnectEx(mSocket, (LPWSAOVERLAPPED)context, TF_REUSE_SOCKET, 0))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(context);
			GConsoleLog->PrintOut(false, "Session::DisconnectRequest Error : %d\n", GetLastError());
		}
	}

}


bool Session::PreRecv()
{
	TRACE_THIS;

	if (!IsConnected())
		return false;

	OverlappedPreRecvContext* recvContext = new OverlappedPreRecvContext(this);

	DWORD recvbytes = 0;
	DWORD flags = 0;
	recvContext->mWsaBuf.len = 0;
	recvContext->mWsaBuf.buf = nullptr;

	/// start async recv
	if (SOCKET_ERROR == WSARecv(mSocket, &recvContext->mWsaBuf, 1, &recvbytes, &flags, (LPWSAOVERLAPPED)recvContext, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(recvContext);
			GConsoleLog->PrintOut(false, "Session::PreRecv Error : %d\n", GetLastError());
			return false;
		}
	}

	return true;
}

bool Session::PostRecv()
{
	TRACE_THIS;

	if (!IsConnected())
		return false;

	if (0 == mRecvBuffer.GetFreeSpaceSize())
		return false;

	OverlappedRecvContext* recvContext = new OverlappedRecvContext(this);

	DWORD recvbytes = 0;
	DWORD flags = 0;
	recvContext->mWsaBuf.len = (ULONG)mRecvBuffer.GetFreeSpaceSize();
	recvContext->mWsaBuf.buf = mRecvBuffer.GetBuffer();


	/// start real recv
	if (SOCKET_ERROR == WSARecv(mSocket, &recvContext->mWsaBuf, 1, &recvbytes, &flags, (LPWSAOVERLAPPED)recvContext, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(recvContext);
			GConsoleLog->PrintOut(false, "Session::PostRecv Error : %d\n", GetLastError());
			return false;
		}

	}

	return true;
}


bool Session::PostSend(const char* data, size_t len)
{
	TRACE_THIS;

	if (!IsConnected())
		return false;

	FastSpinlockGuard criticalSection(mSendBufferLock);

	if (false == mSendBuffer.Write(data, len))
		return false;

	if (mSendRequestedCount > 0) ///< previous send request - not completed
		return true; 

	return FlushSend();
}

bool Session::FlushSend()
{
	TRACE_THIS;

	OverlappedSendContext* sendContext = new OverlappedSendContext(this);

	DWORD sendbytes = 0;
	DWORD flags = 0;
	sendContext->mWsaBuf.len = (ULONG)mSendBuffer.GetContiguiousBytes();
	sendContext->mWsaBuf.buf = mSendBuffer.GetBufferStart();

	/// start async send
	if (SOCKET_ERROR == WSASend(mSocket, &sendContext->mWsaBuf, 1, &sendbytes, flags, (LPWSAOVERLAPPED)sendContext, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(sendContext);
			GConsoleLog->PrintOut(false, "Session::PostSend Error : %d\n", GetLastError());

			DisconnectRequest(DR_IO_REQUEST_ERROR);
			return false;
		}
	}

	++mSendRequestedCount;

	return true;
}


void Session::SendCompletion(DWORD transferred)
{
	TRACE_THIS;

	FastSpinlockGuard criticalSection(mSendBufferLock);

	CRASH_ASSERT(mSendRequestedCount > 0);

	mSendBuffer.Remove(transferred);
	--mSendRequestedCount;

	/// remains to send
	if (mSendBuffer.GetContiguiousBytes() > 0)
	{
		FlushSend();
	}

}

void Session::RecvCompletion(DWORD transferred)
{
	TRACE_THIS;

	mRecvBuffer.Commit(transferred);

	OnRead(transferred);
}

void Session::DisconnectCompletion(DisconnectReason dr)
{
	TRACE_THIS;

	OnDisconnect(dr);

	/// release refcount when added at issuing a session
	ReleaseRef();
}

void Session::AddRef()
{
	int ret = AddRefForThis();
	CRASH_ASSERT(ret >= 0);
}

void Session::ReleaseRef()
{
	int ret = ReleaseRefForThis();
	CRASH_ASSERT(ret > 0);

	if (ret == 0)
	{
		OnRelease();
	}
}

void Session::EchoBack()
{
	TRACE_THIS;

	size_t len = mRecvBuffer.GetContiguiousBytes();

	if (len == 0)
		return;

	if (false == PostSend(mRecvBuffer.GetBufferStart(), len))
		return;

	mRecvBuffer.Remove(len);

}

