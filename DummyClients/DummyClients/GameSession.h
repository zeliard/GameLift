#pragma once
#include <list>
#include <WinSock2.h>
#include "FastSpinlock.h"
#include "DummyClients.h"
#include <aws/gamelift/model/PlayerSession.h>

class PlayerSession;

class GameSession
{
public:
	GameSession(int startPlayerId, int maxPlayerCount, const std::string& name) 
		: mStartPlayerId(startPlayerId), mMaxPlayerCount(maxPlayerCount), mGameSessionName(name)
		, mPort(-1)
	{}
	
	~GameSession();

	bool PreparePlayerSessions();

	bool CreateGameSession();

	bool CreatePlayerSessions();
	bool ConnectPlayerSessions();
	void DisconnectPlayerSessions();

	bool StartGameSessionPlacement();
	bool CheckGameSessionPlacement();

	void GeneratePlacementId();

private:
	typedef std::list<PlayerSession*> ClientList;
	ClientList	mReadySessionList;

	FastSpinlock mLock;

	int mStartPlayerId;
	int mMaxPlayerCount;
	std::string mGameSessionName;

	/// retrieved info
	int mPort;
	std::string mIpAddress;
	std::string mGameSessionId;

	std::vector<Aws::GameLift::Model::PlayerSession> mPlayerSessionList;

	std::string mPlacementId;

	friend class PlayerSession;
};


