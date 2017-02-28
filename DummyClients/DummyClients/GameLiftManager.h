#pragma once

#include <aws/gamelift/GameLiftClient.h>

class GameSession;
class GameLiftManager
{
public:
	GameLiftManager(const std::string& alias);

	void SetUpAwsClient(const std::string& region);
	void PrepareGameSessions(int gsCount);
	void LaunchGameSessions();
	void TerminateGameSessions();

	std::shared_ptr<Aws::GameLift::GameLiftClient> GetAwsClient()
	{
		return mGLClient;
	}

	const std::string& GetAliasId() const
	{
		return mAliasId;
	}

private:
	std::shared_ptr<Aws::GameLift::GameLiftClient> mGLClient;

	std::string mAliasId;

	std::vector<std::shared_ptr<GameSession>> mGameSessions;
};

extern GameLiftManager* GGameLiftManager;