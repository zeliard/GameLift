#pragma once
#include "FastSpinlock.h"

class PlayerSession;
struct PacketHeader;
class GameLiftManager;

class GameSession
{
private:
	void PlayerEnter(std::shared_ptr<PlayerSession> psess);
	void PlayerLeave(std::shared_ptr<PlayerSession> psess);

	void BroadcastMessage(PacketHeader* pkt);

	std::set<std::shared_ptr<PlayerSession>> mPlayerSessions;

	friend class GameLiftManager;
};