#include "stdafx.h"
#include "PlayerSession.h"
#include "Log.h"

#include "PacketType.h"

#include "GameLiftManager.h"

#define CLIENT_BUFSIZE	65536

PlayerSession::PlayerSession(SOCKET sock) : Session(sock, CLIENT_BUFSIZE, CLIENT_BUFSIZE)
{
}

PlayerSession::~PlayerSession()
{
}

void PlayerSession::PlayerLogin(const std::string& playerId)
{
	if (GGameLiftManager->AcceptPlayerSession(std::static_pointer_cast<PlayerSession>(shared_from_this()), playerId))
	{
		mPlayerSessionId = playerId;

		LoginResult outPacket;
		strcpy_s(outPacket.mPlayerId, playerId.c_str());

		PostSend((const char*)&outPacket, outPacket.mSize);
		return;
	}

	/// disconnect unauthed player
	Disconnect(DR_UNAUTH);
}

void PlayerSession::PlayerLogout(const std::string& playerId)
{
	GGameLiftManager->RemovePlayerSession(std::static_pointer_cast<PlayerSession>(shared_from_this()), playerId);
	mPlayerSessionId.clear();

	GConsoleLog->PrintOut(true, "[PLAYER] PlayerLogout: %s \n", playerId.c_str());
	Disconnect(DR_LOGOUT);

}

void PlayerSession::OnDisconnect(DisconnectReason dr)
{
	if (IsValid())
	{
		GGameLiftManager->RemovePlayerSession(std::static_pointer_cast<PlayerSession>(shared_from_this()), mPlayerSessionId);
		mPlayerSessionId.clear();
	}

}