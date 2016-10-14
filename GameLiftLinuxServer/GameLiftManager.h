#pragma once

#include <set>
#include <aws/gamelift/server/model/GameSession.h>


struct PacketHeader;
class GameSession;
class ClientSession;

class GameLiftManager
{
public:
	GameLiftManager();

	bool InitializeGameLift(int listenPort);
	void FinalizeGameLift();

	bool AcceptPlayerSession(std::shared_ptr<ClientSession> psess, const std::string& playerSessionId);
	void RemovePlayerSession(std::shared_ptr<ClientSession> psess, const std::string& playerSessionId);

	void OnStartGameSession(Aws::GameLift::Server::Model::GameSession myGameSession);

	void OnProcessTerminate();

	bool OnHealthCheck() { return mActivated; }

	void BroadcastMessage(PacketHeader* pkt); ///< proxy for a game session
	
private:
	void CheckTerminateGameSession();

private:

	bool mActivated;
	int mAcceptedCount; ///< player accepted count after game-session started
	int mRemovedCount; ///< player removed count

	std::set<std::shared_ptr<ClientSession>> mPlayerSessions;
};

extern std::unique_ptr<GameLiftManager> GGameLiftManager;