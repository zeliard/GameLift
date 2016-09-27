#include "stdafx.h"
#include "PacketType.h"
#include "PlayerSession.h"
#include "GameSession.h"

void GameSession::PlayerEnter(std::shared_ptr<PlayerSession> psess)
{
	mPlayerSessions.insert(psess);
}

void GameSession::PlayerLeave(std::shared_ptr<PlayerSession> psess)
{
	mPlayerSessions.erase(psess);
}

void GameSession::BroadcastMessage(PacketHeader* pkt)
{
	for (auto& it : mPlayerSessions)
	{
		if (false == it->PostSend((const char*)pkt, pkt->mSize))
		{
			it->Disconnect(DR_SENDBUFFER_ERROR);
		}
	}
}