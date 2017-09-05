#include "stdafx.h"
#include "PlayerSession.h"
#include "GameLiftManager.h"
#include "PacketType.h"
#include "Log.h"
#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>

GameLiftManager* GGameLiftManager = nullptr;


GameLiftManager::GameLiftManager(const std::string& matchConfig) : mMatchConfigName(matchConfig)
{
}

void GameLiftManager::SetUpAwsClient(const std::string& region)
{
	Aws::Client::ClientConfiguration config;
	config.scheme = Aws::Http::Scheme::HTTPS;
	config.connectTimeoutMs = 10000;
	config.requestTimeoutMs = 10000;

	config.region = region;

	mGLClient = Aws::MakeShared<Aws::GameLift::GameLiftClient>("GameLiftTest", config);
}

bool GameLiftManager::PreparePlayerSessions(int psCount)
{
	for (int i = 0; i < psCount; ++i)
	{
		std::string psName = "DummyPlayer" + std::to_string(i+1);

		PlayerSession* client = new PlayerSession(psName);

		if (false == client->PrepareSession())
			return false;

		client->AddRef();

		mPlayerSessions.push_back(client);
	}

	return true;
}

void GameLiftManager::LaunchPlayerSessions()
{
	for (auto session : mPlayerSessions)
	{
		session->StartMatchMaking();
	}
}

void GameLiftManager::TerminatePlayerSessions()
{
	for (auto session : mPlayerSessions)
	{
		if (session->IsConnected())
			session->DisconnectRequest(DR_ACTIVE);
	}

	mPlayerSessions.clear();
}
