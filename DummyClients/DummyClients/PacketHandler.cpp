#include "stdafx.h"
#include "Log.h"
#include "PacketType.h"
#include "PlayerSession.h"
#include "DummyClients.h"

//@{ Handler Helper

typedef void(*HandlerFunc)(PlayerSession* session);

static HandlerFunc HandlerTable[PKT_MAX];

static void DefaultHandler(PlayerSession* session)
{
	session->DisconnectRequest(DR_ACTIVE);
}

struct InitializeHandlers
{
	InitializeHandlers()
	{
		for (int i = 0; i < PKT_MAX; ++i)
			HandlerTable[i] = DefaultHandler;
	}
} _init_handlers_;

struct RegisterHandler
{
	RegisterHandler(int pktType, HandlerFunc handler)
	{
		HandlerTable[pktType] = handler;
	}
};

#define REGISTER_HANDLER(PKT_TYPE)	\
	static void Handler_##PKT_TYPE(PlayerSession* session); \
	static RegisterHandler _register_##PKT_TYPE(PKT_TYPE, Handler_##PKT_TYPE); \
	static void Handler_##PKT_TYPE(PlayerSession* session)

//@}

void PlayerSession::OnRead(size_t len)
{
	/// packet parsing
	while (true)
	{
		/// read a packet header
		PacketHeader header;
		if (false == mRecvBuffer.Peek((char*)&header, sizeof(PacketHeader)))
			return;

		/// packet completed? 
		if (mRecvBuffer.GetStoredSize() < header.mSize)
			return;


		if (header.mType >= PKT_MAX || header.mType <= PKT_NONE)
		{
			DisconnectRequest(DR_ACTIVE);
			return;
		}

		/// packet dispatch...
		HandlerTable[header.mType](this);
	}
}

REGISTER_HANDLER(PKT_SC_LOGIN)
{
	LoginResult inPacket;
	if (false == session->ParsePacket(inPacket))
	{
		LoggerUtil::EventLog("packet parsing error: %d\n", inPacket.mType);
		return;
	}

	printf_s("Login OK: PlayerSessionId - %s\n", inPacket.mPlayerId);

	/// after 100ms, send a request
	session->DoAsyncAfter(PLAYER_ACTION_INTERVAL, &PlayerSession::PlayTest);
}



REGISTER_HANDLER(PKT_SC_CHAT)
{
	ChatBroadcastResult inPacket;
	if (false == session->ParsePacket(inPacket))
	{
		LoggerUtil::EventLog("packet parsing error: %d\n", inPacket.mType);
		return;
	}

	/// just for visualizing :(
	//printf_s("."); 
}


REGISTER_HANDLER(PKT_SC_MOVE)
{
	MoveBroadcastResult inPacket;
	if (false == session->ParsePacket(inPacket))
	{
		LoggerUtil::EventLog("packet parsing error: %d\n", inPacket.mType);
		return;
	}

	/// just for visualizing :(
	//printf_s("M"); 
}