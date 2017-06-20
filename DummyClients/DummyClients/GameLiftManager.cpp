#include "stdafx.h"
#include "GameLiftManager.h"
#include "GameSession.h"
#include "PacketType.h"
#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>



GameLiftManager* GGameLiftManager = nullptr;



GameLiftManager::GameLiftManager(const std::string& alias, const std::string& region) : mAliasId(alias), mRegion(region)
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

