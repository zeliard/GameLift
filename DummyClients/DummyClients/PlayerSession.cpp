#include "stdafx.h"
#include "Exception.h"
#include "Log.h"
#include "DummyClients.h"
#include "OverlappedIOContext.h"
#include "PlayerSession.h"
#include "IocpManager.h"
#include "GameSession.h"

#include "PacketType.h"

#define CLIENT_BUFSIZE	65536

PlayerSession::PlayerSession(GameSession* owner) : Session(CLIENT_BUFSIZE, CLIENT_BUFSIZE)
{
	mOwnerGameSession = owner;
	mPlayerIdx = -1;
	mPosX = 0;
	mPosY = 0;
	mPlayTestCount = 0;

	mMoveStep = 0;
	mCurrDir = 0;
}

PlayerSession::~PlayerSession()
{
}

bool PlayerSession::PrepareSession()
{
	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = 0;

 	if (SOCKET_ERROR == bind(mSocket, (SOCKADDR*)&addr, sizeof(addr)))
 	{
		GConsoleLog->PrintOut(true, "PlayerSession::PrepareSession() bind error: %d\n", GetLastError());
 		return false ;
 	}

	HANDLE handle = CreateIoCompletionPort((HANDLE)mSocket, GIocpManager->GetComletionPort(), (ULONG_PTR)this, 0);
	if (handle != GIocpManager->GetComletionPort())
	{
		GConsoleLog->PrintOut(true, "PlayerSession::PrepareSession() CreateIoCompletionPort error: %d\n", GetLastError());
		return false;
	}

	return true;
}

bool PlayerSession::ConnectRequest(const std::string& playerSessionId, int idx)
{
	if (mConnected)
	{
		CRASH_ASSERT(false);
		return false;
	}
	
	/// Set up our socket address structure
	ZeroMemory(&mConnectAddr, sizeof(mConnectAddr));
	mConnectAddr.sin_port = htons(static_cast<u_short>(mOwnerGameSession->mPort));
	mConnectAddr.sin_family = AF_INET;
	mConnectAddr.sin_addr.s_addr = inet_addr(mOwnerGameSession->mIpAddress.c_str());

	OverlappedConnectContext* context = new OverlappedConnectContext(this);

	if (FALSE == ConnectEx(mSocket, (sockaddr*)&mConnectAddr, sizeof(SOCKADDR_IN), NULL, 0, NULL, (LPWSAOVERLAPPED)context))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(context);
			GConsoleLog->PrintOut(true, "PlayerSession::ConnectRequest Error : %d\n", GetLastError());
		}
	}

	mPlayerSessionId = playerSessionId;
	mPlayerIdx = idx;
	return true;
}

void PlayerSession::ConnectCompletion()
{
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0))
	{
		DWORD errCode = GetLastError();

		if (WSAENOTCONN == errCode)
			GConsoleLog->PrintOut(true, "Connecting a server failed: maybe WSAENOTCONN??\n");
		else
			GConsoleLog->PrintOut(true, "SO_UPDATE_CONNECT_CONTEXT failed: %d\n", errCode);

		return;
	}

	int opt = 1;
	if (SOCKET_ERROR == setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int)))
	{
		GConsoleLog->PrintOut(true, "[DEBUG] TCP_NODELAY error: %d\n", GetLastError());
		CRASH_ASSERT(false);
		return;
	}

	opt = 0;
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int)))
	{
		GConsoleLog->PrintOut(true, "[DEBUG] SO_RCVBUF change error: %d\n", GetLastError());
		CRASH_ASSERT(false);
		return;
	}

	if (1 == InterlockedExchange(&mConnected, 1))
	{
		CRASH_ASSERT(false);
	}

	if (false == PreRecv())
	{
		GConsoleLog->PrintOut(true, "[DEBUG] PreRecv for Server Connection error: %d\n", GetLastError());
		InterlockedExchange(&mConnected, 0);
		return;
	}

	GConsoleLog->PrintOut(true, "[DEBUG] Session established: IP=%s, PORT=%d \n", inet_ntoa(mConnectAddr.sin_addr), ntohs(mConnectAddr.sin_port));

	/// start a test!
	StartTest();
}


void PlayerSession::OnDisconnect(DisconnectReason dr)
{
	GConsoleLog->PrintOut(true, "[DEBUG] Client Disconnected: Reason=%d IP=%s, PORT=%d \n", dr, inet_ntoa(mConnectAddr.sin_addr), ntohs(mConnectAddr.sin_port));
}

void PlayerSession::StartTest()
{
	srand(time(NULL));
	mPosX = 480.f;
	mPosY = 320.f;
	mCurrDir = rand() % 8;

	LoginRequest outPacket;
	sprintf_s(outPacket.mPlayerId, "%s", mPlayerSessionId.c_str());
	 
	PostSend((const char*)&outPacket, outPacket.mSize);
}

void PlayerSession::EndTest()
{
	ExitRequest outPacket;
	sprintf_s(outPacket.mPlayerId, "%s", mPlayerSessionId.c_str());

	PostSend((const char*)&outPacket, outPacket.mSize);

	GConsoleLog->PrintOut(true, "%s requested to end a test\n", outPacket.mPlayerId);
}

void PlayerSession::PlayTest()
{
	/// request for ending this player
	if (PLAYER_ACTION_REQUEST == ++mPlayTestCount)
	{
		EndTest();
		return;
	}
	
	if (rand() % 10 == 0)
	{
		ChatBroadcastRequest outPacket;
		sprintf_s(outPacket.mPlayerId, "%s", mPlayerSessionId.c_str());

		char chat[100];
		chat[99] = '\0';
		strcpy_s(outPacket.mChat, chat);

		PostSend((const char*)&outPacket, outPacket.mSize);
	}
	else
	{
		MoveRequest outPacket;
		outPacket.mPlayerIdx = mPlayerIdx;

		SetNextPos();
		outPacket.mPosX = mPosX;
		outPacket.mPosY = mPosY;

		PostSend((const char*)&outPacket, outPacket.mSize);

	}

	/// after an interval do again
	DoAsyncAfter(PLAYER_ACTION_INTERVAL, &PlayerSession::PlayTest);
}

void PlayerSession::SetNextPos()
{
	if (10 == mMoveStep++)
	{
		mMoveStep = 0;
		
		/// find new dir
		mCurrDir = rand() % 8;
	}

	float vec = static_cast<float>(rand() % 10) + 3.f;
	switch (mCurrDir)
	{
	case 0:
		mPosY += vec;
		break;
	case 1:
		mPosY += vec;
		mPosX += vec + 3.f;
		break;
	case 2:
		mPosX += vec + 3.f;
		break;
	case 3:
		mPosX += vec + 3.f;
		mPosY -= vec;
		break;
	case 4:
		mPosY -= vec;
		break;
	case 5:
		mPosY -= vec;
		mPosX -= (vec + 3.f);
		break;
	case 6:
		mPosX -= (vec + 3.f);
		break;
	case 7:
		mPosX -= (vec + 3.f);
		mPosY += vec;
		break;
	default:
		break;
	}

	
}