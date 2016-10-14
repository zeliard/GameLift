#include "Log.h"
#include "../PacketShare/PacketType.h"
#include "ClientSession.h"
#include "GameLiftManager.h"


//@{ Handler Helper

typedef std::shared_ptr<ClientSession> ClientSessionPtr;

typedef void(*HandlerFunc)(ClientSessionPtr session);

static HandlerFunc HandlerTable[PKT_MAX];

static void DefaultHandler(ClientSessionPtr session)
{
	GConsoleLog->PrintOut(true, "Invalid packet handler");
	session->Disconnect();
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
	static void Handler_##PKT_TYPE(ClientSessionPtr session); \
	static RegisterHandler _register_##PKT_TYPE(PKT_TYPE, Handler_##PKT_TYPE); \
	static void Handler_##PKT_TYPE(ClientSessionPtr session)

//@}

///////////////////////////////////////////////////////////

void ClientSession::DispatchPacket()
{
	/// packet parsing
	while (true)
	{
		/// read packet header
		PacketHeader header;
		if (false == mRecvBuffer.Peek((char*)&header, sizeof(PacketHeader)))
			return;

		/// packet completed? 
		if (mRecvBuffer.GetStoredSize() < (size_t)header.mSize)
			return;

		if (header.mType >= PKT_MAX || header.mType <= PKT_NONE)
		{
			GConsoleLog->PrintOut(true, "Invalid packet type\n");
			Disconnect();
			return;
		}

		/// packet dispatch...
		HandlerTable[header.mType](shared_from_this());
	}
}

/////////////////////////////////////////////////////////
REGISTER_HANDLER(PKT_CS_LOGIN)
{
	LoginRequest inPacket;
	if (false == session->ParsePacket(inPacket))
	{
		GConsoleLog->PrintOut(true, "packet parsing error, Type: %d\n", inPacket.mType);
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
		GConsoleLog->PrintOut(true, "[DEBUG] packet parsing error, Type: %d\n", inPacket.mType);
		return;
	}
	
	/// direct response in case of chatting
	ChatBroadcastResult outPacket;

	strcpy(outPacket.mPlayerId, inPacket.mPlayerId);
	strcpy(outPacket.mChat, inPacket.mChat);
	
	GGameLiftManager->BroadcastMessage(&outPacket);
}


REGISTER_HANDLER(PKT_CS_MOVE)
{
	MoveRequest inPacket;
	if (false == session->ParsePacket(inPacket))
	{
		GConsoleLog->PrintOut(true, "[DEBUG] packet parsing error, Type: %d\n", inPacket.mType);
		return;
	}

	/// just broadcast for now
	MoveBroadcastResult outPacket;
	outPacket.mPlayerIdx = inPacket.mPlayerIdx;
	outPacket.mPosX = inPacket.mPosX;
	outPacket.mPosY = inPacket.mPosY;

	GGameLiftManager->BroadcastMessage(&outPacket);
}






