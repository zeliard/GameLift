#include "stdafx.h"
#include "Log.h"
#include "PacketType.h"
#include "PlayerSession.h"
#include "GameLiftManager.h"

//@{ Handler Helper

typedef std::shared_ptr<PlayerSession> PlayerSessionPtr;

typedef void(*HandlerFunc)(PlayerSessionPtr session);

static HandlerFunc HandlerTable[PKT_MAX];

static void DefaultHandler(PlayerSessionPtr session)
{
	session->Disconnect(DR_ACTIVE);
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
	static void Handler_##PKT_TYPE(PlayerSessionPtr session); \
	static RegisterHandler _register_##PKT_TYPE(PKT_TYPE, Handler_##PKT_TYPE); \
	static void Handler_##PKT_TYPE(PlayerSessionPtr session)

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

		/// check a packet completed on not
		if (mRecvBuffer.GetStoredSize() < header.mSize)
			return;


		if (header.mType >= PKT_MAX || header.mType <= PKT_NONE)
		{
			Disconnect(DR_ACTIVE);
			return;
		}

		/// packet dispatch...
		HandlerTable[header.mType](std::static_pointer_cast<PlayerSession>(shared_from_this()));
	}
}

/////////////////////////////////////////////////////////
REGISTER_HANDLER(PKT_CS_LOGIN)
{
	LoginRequest inPacket;
	if (false == session->ParsePacket(inPacket))
	{
		GConsoleLog->PrintOut(true, "packet parsing error: %d\n", inPacket.mType);
		return;
	}
	
	session->PlayerLogin(std::string(inPacket.mPlayerId));

}

REGISTER_HANDLER(PKT_CS_EXIT)
{
	ExitRequest inPacket;
	if (false == session->ParsePacket(inPacket))
	{
		GConsoleLog->PrintOut(true, "packet parsing error: %d\n", inPacket.mType);
		return;
	}

	session->PlayerLogout(std::string(inPacket.mPlayerId));

}

REGISTER_HANDLER(PKT_CS_CHAT)
{
	ChatBroadcastRequest inPacket;
	if (false == session->ParsePacket(inPacket))
	{
		GConsoleLog->PrintOut(true, "packet parsing error: %d\n", inPacket.mType);
		return;
	}

	/// direct response in case of chatting
	ChatBroadcastResult outPacket;
		
	strcpy_s(outPacket.mPlayerId, inPacket.mPlayerId);
	strcpy_s(outPacket.mChat, inPacket.mChat);

	//session->PostSend((const char*)&outPacket, outPacket.mSize);

	GGameLiftManager->BroadcastMessage(&outPacket);
	
}


REGISTER_HANDLER(PKT_CS_MOVE)
{
	MoveRequest inPacket;
	if (false == session->ParsePacket(inPacket))
	{
		GConsoleLog->PrintOut(true, "packet parsing error: %d\n", inPacket.mType);
		return;
	}

	/// just broadcast for now
	MoveBroadcastResult outPacket;
	outPacket.mPlayerIdx = inPacket.mPlayerIdx;
	outPacket.mPosX = inPacket.mPosX;
	outPacket.mPosY = inPacket.mPosY;

	GGameLiftManager->BroadcastMessage(&outPacket);
}

