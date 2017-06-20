#pragma once

#include <aws/gamelift/GameLiftClient.h>

class GameSession;
class GameLiftManager
{
public:
	GameLiftManager(const std::string& alias, const std::string& region);

	void SetUpAwsClient(const std::string& region);
	void PrepareGameSessions(int gsCount);
	void LaunchGameSessions();
	void TerminateGameSessions();

	/// New API Test: StartGameSessionPlacement
	void LaunchGameSessionPlacement();

	std::shared_ptr<Aws::GameLift::GameLiftClient> GetAwsClient()
	{
		return mGLClient;
	}

	const std::string& GetAliasId() const
	{
		return mAliasId;
	}

	const std::string& GetRegion() const
	{
		return mRegion;
	}


private:
	std::shared_ptr<Aws::GameLift::GameLiftClient> mGLClient;

	std::string mAliasId;
	std::string mRegion;

	std::vector<std::shared_ptr<GameSession>> mGameSessions;
};

extern GameLiftManager* GGameLiftManager;