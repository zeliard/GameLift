#pragma once
#include <aws/gamelift/GameLiftClient.h>

class GameLiftManager
{
public:
	GameLiftManager(const std::string& alias);

	void SetUpAwsClient(Aws::Region region);
	
	bool SearchGameSessions();
	bool CreatePlayerSession();
	bool ConnectPlayerSession();
	
	std::shared_ptr<Aws::GameLift::GameLiftClient> GetAwsClient()
	{
		return mGLClient;
	}

	const std::string& GetAliasId() const
	{
		return mAliasId;
	}

	void DoTestNow();

private:
	std::shared_ptr<Aws::GameLift::GameLiftClient> mGLClient;

	std::string mAliasId;

	// retrieved info
	int mPort = 0 ;
	std::string mServerIpAddress;
	std::string mGameSessionId;
	std::string mPlayerSessionId;


	
};

extern GameLiftManager* GGameLiftManager;