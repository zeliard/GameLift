#pragma once

#include <aws/gamelift/GameLiftClient.h>

class PlayerSession;
class GameLiftManager
{
public:
	GameLiftManager(const std::string& matchConfig);

	void SetUpAwsClient(const std::string& region);
	
	bool PreparePlayerSessions(int psCount);

	void LaunchPlayerSessions();
	void TerminatePlayerSessions();


	std::shared_ptr<Aws::GameLift::GameLiftClient> GetAwsClient()
	{
		return mGLClient;
	}

	const std::string& GetMatchMakingConfigName() const
	{
		return mMatchConfigName;
	}

private:

	std::shared_ptr<Aws::GameLift::GameLiftClient> mGLClient;

	std::string mMatchConfigName;

	std::vector<PlayerSession*> mPlayerSessions;
};

extern GameLiftManager* GGameLiftManager;