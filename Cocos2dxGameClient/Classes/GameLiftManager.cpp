#include "GameLiftManager.h"
#include "GameLiftManager.h"
#include "TcpClient.h"
#include "platform/CCFileUtils.h"

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/Outcome.h>
#include <aws/gamelift/model/SearchGameSessionsRequest.h>
#include <aws/gamelift/model/CreatePlayerSessionRequest.h>



GameLiftManager* GGameLiftManager = nullptr;

GameLiftManager::GameLiftManager(const std::string& alias) : mAliasId(alias)
{
}

void GameLiftManager::SetUpAwsClient(AWs::Region region)
{

	Aws::Client::ClientConfiguration config;
	config.scheme = Aws::Http::Scheme::HTTPS;
	config.connectTimeoutMs = 30000;
	config.requestTimeoutMs = 30000;

	config.region = region;

	mGLClient = Aws::MakeShared<Aws::GameLift::GameLiftClient>("GameLiftTest", config);

}

bool GameLiftManager::SearchGameSessions()
{
	Aws::GameLift::Model::SearchGameSessionsRequest req;
	req.SetAliasId(mAliasId);
	req.SetFilterExpression("hasAvailablePlayerSessions=true");

	auto outcome = mGLClient->SearchGameSessions(req);
	if (outcome.IsSuccess())
	{
		auto& games = outcome.GetResult().GetGameSessions();
		auto& gs = games[0];

		mPort = gs.GetPort();
		mServerIpAddress = gs.GetIpAddress();
		mGameSessionId = gs.GetGameSessionId();
		return true;
	}

	//CCLOG("%s\n", outcome.GetError().GetMessage().c_str());

	return false;
	
}

bool GameLiftManager::CreatePlayerSession()
{
	Aws::GameLift::Model::CreatePlayerSessionRequest req;
	req.SetGameSessionId(mGameSessionId);
	req.SetPlayerId("PlayerCoCos");

	auto outcome = mGLClient->CreatePlayerSession(req);
	if (outcome.IsSuccess())
	{
		auto& psess = outcome.GetResult().GetPlayerSession();
		mPlayerSessionId = psess.GetPlayerSessionId();

		return true;
	}

	//printf_s("%s\n", outcome.GetError().GetMessage().c_str());
	return false;
	
}

bool GameLiftManager::ConnectPlayerSession()
{
	if (TcpClient::getInstance()->connect(mServerIpAddress.c_str(), mPort))
	{
		TcpClient::getInstance()->loginRequest(mPlayerSessionId);
		return true;
	}

	/// conn error
	return false;
}

void GameLiftManager::DoTestNow()
{
	if (SearchGameSessions())
	{
		if (CreatePlayerSession())
		{
			if (ConnectPlayerSession())
			{
				/// ok gogo
			}
		}
	}
}

