#include "stdafx.h"
#include "Exception.h"
#include "Log.h"
#include "DummyClients.h"
#include "OverlappedIOContext.h"
#include "PlayerSession.h"
#include "IocpManager.h"
#include "GameLiftManager.h"

#include "PacketType.h"

#include <aws/core/utils/Outcome.h>
#include <aws/gamelift/model/StartMatchmakingRequest.h>
#include <aws/gamelift/model/StartMatchmakingResult.h>
#include <aws/gamelift/model/DescribeMatchmakingRequest.h>
#include <aws/gamelift/model/DescribeMatchmakingResult.h>

#define CLIENT_BUFSIZE	65536

PlayerSession::PlayerSession(const std::string& playerId) : Session(CLIENT_BUFSIZE, CLIENT_BUFSIZE), mPlayerId(playerId)
{
	mPort = -1;
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
		GConsoleLog->PrintOut(false, "PlayerSession::PrepareSession() bind error: %d\n", GetLastError());
 		return false ;
 	}

	HANDLE handle = CreateIoCompletionPort((HANDLE)mSocket, GIocpManager->GetComletionPort(), (ULONG_PTR)this, 0);
	if (handle != GIocpManager->GetComletionPort())
	{
		GConsoleLog->PrintOut(false, "PlayerSession::PrepareSession() CreateIoCompletionPort error: %d\n", GetLastError());
		return false;
	}

	return true;
}

bool PlayerSession::StartMatchMaking()
{
	Aws::GameLift::Model::StartMatchmakingRequest req;
	req.SetConfigurationName(GGameLiftManager->GetMatchMakingConfigName());
	
	Aws::GameLift::Model::Player player;
	player.SetPlayerId(mPlayerId);
	std::vector<Aws::GameLift::Model::Player> players;
	players.push_back(player);
	req.SetPlayers(players);

	auto outcome = GGameLiftManager->GetAwsClient()->StartMatchmaking(req);
	if (!outcome.IsSuccess())
	{
		GConsoleLog->PrintOut(true, "%s\n", outcome.GetError().GetMessageA().c_str());
		return false;
	}

	mTicketId = outcome.GetResult().GetMatchmakingTicket().GetTicketId();

	/// Post to IOCP queue -> TrachMatchMaking...
	GIocpManager->PostIocpTask(this);

	return true;
}

void PlayerSession::TrackMatchMaking()
{
	Aws::GameLift::Model::DescribeMatchmakingRequest req;
	std::vector<std::string> ticketIds;
	ticketIds.push_back(mTicketId);
	req.SetTicketIds(ticketIds);

	auto outcome = GGameLiftManager->GetAwsClient()->DescribeMatchmaking(req);
	if (!outcome.IsSuccess())
	{
		GConsoleLog->PrintOut(true, "%s\n", outcome.GetError().GetMessageA().c_str());
		return;
	}

	auto result = outcome.GetResult().GetTicketList()[0];

	if (result.GetStatus() == Aws::GameLift::Model::MatchmakingConfigurationStatus::COMPLETED)
	{
		auto info = result.GetGameSessionConnectionInfo();
		mIpAddress = info.GetIpAddress();
		mPort = info.GetPort();

		for (auto& psess : info.GetMatchedPlayerSessions())
		{
			/// Find Me
			if (psess.GetPlayerId() == mPlayerId)
			{
				mPlayerSessionId = psess.GetPlayerSessionId();

				/// Connect to a Server
				ConnectRequest();
				
				return;
			}
		}
	}

	DoAsyncAfter(1000, &PlayerSession::TrackMatchMaking);
}

bool PlayerSession::ConnectRequest()
{
	if (mConnected)
	{
		CRASH_ASSERT(false);
		return false;
	}
	
	/// Set up our socket address structure
	ZeroMemory(&mConnectAddr, sizeof(mConnectAddr));
	mConnectAddr.sin_port = htons(static_cast<u_short>(mPort));
	mConnectAddr.sin_family = AF_INET;
	mConnectAddr.sin_addr.s_addr = inet_addr(mIpAddress.c_str());

	OverlappedConnectContext* context = new OverlappedConnectContext(this);

	if (FALSE == ConnectEx(mSocket, (sockaddr*)&mConnectAddr, sizeof(SOCKADDR_IN), NULL, 0, NULL, (LPWSAOVERLAPPED)context))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(context);
			GConsoleLog->PrintOut(false, "PlayerSession::ConnectRequest Error : %d\n", GetLastError());
		}
	}
	
	return true;
}

void PlayerSession::ConnectCompletion()
{
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0))
	{
		DWORD errCode = GetLastError();

		if (WSAENOTCONN == errCode)
			GConsoleLog->PrintOut(false, "Connecting a server failed: maybe WSAENOTCONN??\n");
		else
			GConsoleLog->PrintOut(false, "SO_UPDATE_CONNECT_CONTEXT failed: %d\n", errCode);

		return;
	}

	int opt = 1;
	if (SOCKET_ERROR == setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int)))
	{
		GConsoleLog->PrintOut(false, "[DEBUG] TCP_NODELAY error: %d\n", GetLastError());
		CRASH_ASSERT(false);
		return;
	}

	opt = 0;
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int)))
	{
		GConsoleLog->PrintOut(false, "[DEBUG] SO_RCVBUF change error: %d\n", GetLastError());
		CRASH_ASSERT(false);
		return;
	}

	if (1 == InterlockedExchange(&mConnected, 1))
	{
		CRASH_ASSERT(false);
	}

	if (false == PreRecv())
	{
		GConsoleLog->PrintOut(false, "[DEBUG] PreRecv for Server Connection error: %d\n", GetLastError());
		InterlockedExchange(&mConnected, 0);
		return;
	}

	GConsoleLog->PrintOut(false, "[DEBUG] Session established: IP=%s, PORT=%d \n", inet_ntoa(mConnectAddr.sin_addr), ntohs(mConnectAddr.sin_port));

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
		outPacket.mPlayerIdx = rand();

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