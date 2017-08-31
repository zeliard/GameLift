#pragma once

#include <aws/gamelift/GameLiftClient.h>

class GameSession;
class GameLiftManager
{
public:
	GameLiftManager(const std::string& alias, const std::string& region, const std::string& matchQueue);

	void SetUpAwsClient(const std::string& region);
	void PrepareGameSessions(int gsCount);
	void LaunchGameSessions();
	void TerminateGameSessions();

	/// New API Test: StartGameSessionPlacement
	void LaunchGameSessionPlacement();

	/// FlexMatch Test
	void TestFlexMatch();

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

	const std::string& GetMatchQueue() const
	{
		return mMatchQueueName;
	}


private:
	std::shared_ptr<Aws::GameLift::GameLiftClient> mGLClient;

	std::string mRegion;

	std::string mAliasId; ///< only used if MatchQueue mode is not enabled.
	std::string mMatchQueueName; ///< only used when MatchQueue enabled

	std::vector<std::shared_ptr<GameSession>> mGameSessions;
};

extern GameLiftManager* GGameLiftManager;