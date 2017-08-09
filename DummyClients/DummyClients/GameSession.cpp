#include "stdafx.h"
#include "Exception.h"
#include "FastSpinlock.h"
#include "DummyClients.h"
#include "PlayerSession.h"
#include "GameSession.h"
#include "IocpManager.h"
#include "GameLiftManager.h"
#include "PacketType.h"
#include "DummyClients.h"

#include <Rpc.h>
#include <aws/core/utils/Outcome.h>
#include <aws/gamelift/model/CreateGameSessionRequest.h>
#include <aws/gamelift/model/CreatePlayerSessionsRequest.h>
#include <aws/gamelift/model/StartGameSessionPlacementRequest.h>
#include <aws/gamelift/model/DescribeGameSessionPlacementRequest.h>
#include <aws/gamelift/model/DescribeGameSessionDetailsRequest.h>

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
	
	auto aliasId = GGameLiftManager->GetAliasId();
	if (aliasId == "TEST_LOCAL")
	{
		req.SetFleetId(std::string("fleet-") + mGameSessionName);
	}
	else
	{
		req.SetAliasId(aliasId);
	}
	
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

bool GameSession::StartGameSessionPlacement()
{
	FastSpinlockGuard guard(mLock);

	/// region reset for a match queue...
	GGameLiftManager->SetUpAwsClient(GGameLiftManager->GetRegion());

	GeneratePlacementId();

	Aws::GameLift::Model::StartGameSessionPlacementRequest req;
	req.SetGameSessionQueueName(GGameLiftManager->GetMatchQueue());
	req.SetMaximumPlayerSessionCount(MAX_PLAYER_PER_GAME);
	req.SetPlacementId(mPlacementId);

	auto outcome = GGameLiftManager->GetAwsClient()->StartGameSessionPlacement(req);
	if (outcome.IsSuccess())
	{
		auto status = outcome.GetResult().GetGameSessionPlacement().GetStatus();

		if (status == Aws::GameLift::Model::GameSessionPlacementState::PENDING)
		{
			return CheckGameSessionPlacement();
		}

		if (status == Aws::GameLift::Model::GameSessionPlacementState::FULFILLED)
			return true;
	}

	printf_s("%s\n", outcome.GetError().GetMessageA().c_str());

	return false;
}

bool GameSession::CheckGameSessionPlacement()
{
	while (true)
	{
		Aws::GameLift::Model::DescribeGameSessionPlacementRequest req;
		req.SetPlacementId(mPlacementId);
		auto outcome = GGameLiftManager->GetAwsClient()->DescribeGameSessionPlacement(req);
		if (outcome.IsSuccess())
		{
			auto gs = outcome.GetResult().GetGameSessionPlacement();

			if (gs.GetStatus() == Aws::GameLift::Model::GameSessionPlacementState::FULFILLED)
			{
				auto arn = gs.GetGameSessionArn();
				
				std::string delim ="::gamesession";
				std::string region = arn.substr(0, arn.find(delim));
				region = region.substr(17);
			
				/// change region...
				GGameLiftManager->SetUpAwsClient(region);
				
				Aws::GameLift::Model::DescribeGameSessionDetailsRequest request;
				
				request.SetGameSessionId(arn);
				auto response = GGameLiftManager->GetAwsClient()->DescribeGameSessionDetails(request);
				if (response.IsSuccess())
				{
					// GameSession Fulfill..
					auto result = response.GetResult().GetGameSessionDetails();
					mIpAddress = result[0].GetGameSession().GetIpAddress();
					mPort = result[0].GetGameSession().GetPort();
					mGameSessionId = arn;
				}
				else
				{
					printf_s("%s\n", response.GetError().GetMessageA().c_str());
					return false;
				}
				
				return true;
			}

		}
		else
		{
			break;
		}

		Sleep(500);
	}

	return false;

}

void GameSession::GeneratePlacementId()
{
	UUID uuid;
	UuidCreate(&uuid);

	unsigned char* str = nullptr;
	UuidToStringA(&uuid, &str);

	mPlacementId.clear();
	mPlacementId = std::string((char*)str);

	RpcStringFreeA(&str);
}