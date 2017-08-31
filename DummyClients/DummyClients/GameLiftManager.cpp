#include "stdafx.h"
#include "GameLiftManager.h"
#include "GameSession.h"
#include "PacketType.h"
#include "Log.h"
#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>

#include <aws/core/utils/Outcome.h>
#include <aws/gamelift/model/StartMatchmakingRequest.h>
#include <aws/gamelift/model/StartMatchmakingResult.h>
#include <aws/gamelift/model/DescribeMatchmakingRequest.h>
#include <aws/gamelift/model/DescribeMatchmakingResult.h>

GameLiftManager* GGameLiftManager = nullptr;



GameLiftManager::GameLiftManager(const std::string& alias, const std::string& region, const std::string& matchQueue)
	: mAliasId(alias), mRegion(region), mMatchQueueName(matchQueue)
{
}

void GameLiftManager::SetUpAwsClient(const std::string& region)
{
	Aws::Client::ClientConfiguration config;
	config.scheme = Aws::Http::Scheme::HTTPS;
	config.connectTimeoutMs = 10000;
	config.requestTimeoutMs = 10000;

	config.region = region;

	/// In case of GameLift Local
	if (mAliasId == "TEST_LOCAL")
	{
		config.scheme = Aws::Http::Scheme::HTTP;
		config.endpointOverride = region;
	}
	
	mGLClient = Aws::MakeShared<Aws::GameLift::GameLiftClient>("GameLiftTest", config);
}

void GameLiftManager::PrepareGameSessions(int gsCount)
{
	for (int i = 0; i < gsCount; ++i)
	{
		std::string gsName = "MyTestGameSession" + std::to_string(i+1);

		auto newGs = std::make_shared<GameSession>(i*MAX_PLAYER_PER_GAME, MAX_PLAYER_PER_GAME, gsName);

		newGs->PreparePlayerSessions();

		mGameSessions.push_back(newGs);
	}
	
}

void GameLiftManager::LaunchGameSessions()
{
	for (auto& gs : mGameSessions)
	{
		if (gs->CreateGameSession())
		{
			if (gs->CreatePlayerSessions())
			{
				gs->ConnectPlayerSessions();
			}
		}
		Sleep(1000);
	}
}

void GameLiftManager::TerminateGameSessions()
{
	for (auto& gs : mGameSessions)
	{
		gs->DisconnectPlayerSessions();
	}
}

void GameLiftManager::LaunchGameSessionPlacement()
{
	for (auto& gs : mGameSessions)
	{
		if (gs->StartGameSessionPlacement())
		{
			if (gs->CreatePlayerSessions())
			{
				gs->ConnectPlayerSessions();
			}
		}
		Sleep(1000);
	}
}

void GameLiftManager::TestFlexMatch()
{
	std::set<std::string> tickets;

	for (int i = 0; i < 20; ++i)
	{
		Aws::GameLift::Model::StartMatchmakingRequest req;
		req.SetConfigurationName("DummyMatchMakingConfig");
		Aws::GameLift::Model::Player player;
		std::string pid = "DummyPlayer" + std::to_string(1 + i);
		player.SetPlayerId(pid);
		std::vector<Aws::GameLift::Model::Player> players;
		players.push_back(player);
		req.SetPlayers(players);

		auto outcome = mGLClient->StartMatchmaking(req);
		if (!outcome.IsSuccess())
		{
			GConsoleLog->PrintOut(true, "%s\n", outcome.GetError().GetMessageA().c_str());
			return;
		}

		auto ticketId = outcome.GetResult().GetMatchmakingTicket().GetTicketId();
		tickets.insert(ticketId);
	}
	
	Sleep(5000);

	for (auto& tkt : tickets)
	{
		Aws::GameLift::Model::DescribeMatchmakingRequest req;
		std::vector<std::string> tkts;
		tkts.push_back(tkt);
		req.SetTicketIds(tkts);
		auto outcome = mGLClient->DescribeMatchmaking(req);
		if (!outcome.IsSuccess())
		{
			GConsoleLog->PrintOut(true, "%s\n", outcome.GetError().GetMessageA().c_str());
			return;
		}

		auto result = outcome.GetResult().GetTicketList()[0];
		
		if (result.GetStatus() == Aws::GameLift::Model::MatchmakingConfigurationStatus::COMPLETED)
		{
			auto info = result.GetGameSessionConnectionInfo();
			std::cout << info.GetIpAddress() << " " << info.GetPort() << std::endl;
		}
		

	}
	
	Sleep(1000);
	


}