#pragma once

#include <aws/gamelift/server/model/GameSession.h>
#include "FastSpinLock.h"

struct PacketHeader;
class GameSession;
class PlayerSession;

class GameLiftManager
{
public:
	GameLiftManager();

	bool InitializeGameLift(int listenPort);
	void FinalizeGameLift();

	bool AcceptPlayerSession(std::shared_ptr<PlayerSession> psess, const std::string& playerSessionId);
	void RemovePlayerSession(std::shared_ptr<PlayerSession> psess, const std::string& playerSessionId);

	void OnStartGameSession(Aws::GameLift::Server::Model::GameSession myGameSession);

	void OnProcessTerminate();

	bool OnHealthCheck() { return mActivated; }

	void BroadcastMessage(PacketHeader* pkt); ///< proxy for a game session
	
private:
	void CheckTerminateGameSession();

private:
	FastSpinlock mLock;

	bool mActivated;
	int mAcceptedCount; ///< player accepted count after game-session started
	int mRemovedCount; ///< player removed count

	std::shared_ptr<GameSession> mGameSession; ///< 1:1 relationship, currently
};

extern std::unique_ptr<GameLiftManager> GGameLiftManager;