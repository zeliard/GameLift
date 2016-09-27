#include "stdafx.h"
#include "Exception.h"
#include "FastSpinlock.h"
#include "DummyClients.h"
#include "PlayerSession.h"
#include "GameSession.h"
#include "IocpManager.h"
#include "GameLiftManager.h"

#include <aws/core/utils/Outcome.h>
#include <aws/gamelift/model/CreateGameSessionRequest.h>
#include <aws/gamelift/model/CreatePlayerSessionsRequest.h>

GameSession::~GameSession()
{
	for (auto it : mReadySessionList)
	{
		delete it;
	}
}

bool GameSession::PreparePlayerSessions()
{
	CRASH_ASSERT(LThreadType == THREAD_MAIN);

	for (int i = 0; i < mMaxPlayerCount - TEST_PLAYER_SESSION_EXCEPT; ++i)
	{
		PlayerSession* client = new PlayerSession(this);

		if (false == client->PrepareSession())
			return false;
			
		mReadySessionList.push_back(client);
	}

	return true;
}

bool GameSession::CreateGameSession()
{
	FastSpinlockGuard guard(mLock);

	Aws::GameLift::Model::CreateGameSessionRequest req;
	req.SetAliasId(GGameLiftManager->GetAliasId());
	req.SetName(mGameSessionName);
	req.SetMaximumPlayerSessionCount(mMaxPlayerCount);
	auto outcome = GGameLiftManager->GetAwsClient()->CreateGameSession(req);
	if (outcome.IsSuccess())
	{
		auto gs = outcome.GetResult().GetGameSession();
		mPort = gs.GetPort();
		mIpAddress = gs.GetIpAddress();
		mGameSessionId = gs.GetGameSessionId();
		return true;
	}

	printf_s("%s\n", outcome.GetError().GetMessageA().c_str());

	return false;
}

bool GameSession::CreatePlayerSessions()
{
	FastSpinlockGuard guard(mLock);

	Aws::GameLift::Model::CreatePlayerSessionsRequest req;
	req.SetGameSessionId(mGameSessionId);
	std::vector<std::string> playerIds;

	for (int i = 0; i < mMaxPlayerCount - TEST_PLAYER_SESSION_EXCEPT; ++i) ///< must be less than 25
	{
		std::string pid = "DummyPlayer" + std::to_string(mStartPlayerId + i);
		playerIds.push_back(pid);
	}
	req.SetPlayerIds(playerIds);

	auto outcome = GGameLiftManager->GetAwsClient()->CreatePlayerSessions(req);
	if (outcome.IsSuccess())
	{
		mPlayerSessionList = outcome.GetResult().GetPlayerSessions();
		return true;
	}

	printf_s("%s\n", outcome.GetError().GetMessageA().c_str());
	return false;
}

bool GameSession::ConnectPlayerSessions()
{
	FastSpinlockGuard guard(mLock);

	auto it = mReadySessionList.begin();
	int idx = mStartPlayerId;
	for (auto& playerSessionItem : mPlayerSessionList)
	{
		(*it)->AddRef();
		if (false == (*it)->ConnectRequest(playerSessionItem.GetPlayerSessionId(), idx++))
			return false;

		Sleep(10);

		++it;
	}
	
	return true;
}

void GameSession::DisconnectPlayerSessions()
{
	FastSpinlockGuard guard(mLock);

	for (auto session : mReadySessionList)
	{
		if (session->IsConnected())
			session->DisconnectRequest(DR_ACTIVE);
	}
}

