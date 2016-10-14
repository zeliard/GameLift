#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <typeinfo>

#include "Config.h"
#include "Log.h"
#include "Scheduler.h"
#include "ClientSession.h"
#include "../PacketShare/PacketType.h"
#include "ClientManager.h"
#include "GameLiftManager.h"

ClientSession::~ClientSession()
{
	/// automatically remove from epoll-list
	close(mSocket);
}

void ClientSession::OnConnect(sockaddr_in* addr)
{
	memcpy(&mClientAddr, addr, sizeof(sockaddr_in));

	/// make socket non-blocking
	int flag = fcntl(mSocket, F_GETFL, 0);
	fcntl(mSocket, F_SETFL, flag | O_NONBLOCK);

	/// nagle off
	int opt = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int));

	GConsoleLog->PrintOut(true, "Client Connected: IP=%s, PORT=%d\n", inet_ntoa(mClientAddr.sin_addr), ntohs(mClientAddr.sin_port));
	
	mConnected = true;
}

void ClientSession::Disconnect()
{
	if (!IsConnected())
		return;
	
	linger lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;

	/// no TCP TIME_WAIT
	if (setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(linger)) < 0)
	{
		GConsoleLog->PrintOut(true, "setsockopt linger option error\n");
		return;
	}

	mConnected = false;

	OnDisconnect();

	GConsoleLog->PrintOut(true, "Client Disconnected: IP=%s, PORT=%d\n", inet_ntoa(mClientAddr.sin_addr), ntohs(mClientAddr.sin_port));

	GClientManager->DeleteClient(shared_from_this());
}



void ClientSession::OnReceive()
{
	CRASH_ASSERT(IsConnected());

	while (true)
	{
		int nread = read(mSocket, mRecvBuffer.GetBuffer(), mRecvBuffer.GetFreeSpaceSize());
		if (nread < 0)
		{
			/// no more data
			if (errno == EAGAIN)
				break;

			/// if error
			Disconnect();
			return;
		}

		mRecvBuffer.Commit(nread);

		
#ifdef ECHO_MODE
		EchoBack();
#else
		DispatchPacket();
#endif

	}
}

void ClientSession::EchoBack()
{

	size_t len = mRecvBuffer.GetContiguiousBytes();

	if (len == 0)
		return;

	/// buffering first, then flushing
	if (false == mSendBuffer.Write((char*)mRecvBuffer.GetBufferStart(), len))
	{
		Disconnect();
		return;
	}

	mRecvBuffer.Remove(len);

}

bool ClientSession::SendRequest(PacketHeader* pkt)
{
	if ( !IsConnected() )
		return false;

	/// buffering first, then flushing
	if ( false == mSendBuffer.Write((char*)pkt, pkt->mSize) )
	{
		/// insufficient capacity
		Disconnect();
		return false;
	}

	return true;
}

bool ClientSession::SendFlush()
{
	if (!IsConnected())
		return false;

	/// no more send-data
	if (mSendBuffer.GetContiguiousBytes() == 0)
		return true;

	while (mSendBuffer.GetContiguiousBytes() > 0)
	{
		int sent = write(mSocket, mSendBuffer.GetBufferStart(), mSendBuffer.GetContiguiousBytes());
		if (sent < 0)
		{
			if (errno == EAGAIN)
				return true;

			return false;
		}

		/// remove data sent
		mSendBuffer.Remove(sent);
	}

	return true;
}



void ClientSession::OnTick()
{
	if (!IsConnected())
		return;

	/// Periodic work here
	// ...

	
	CallFuncAfter(PLAYER_HEART_BEAT, shared_from_this(), &ClientSession::OnTick);
}



void ClientSession::PlayerLogin(const std::string& playerId)
{
	if (GGameLiftManager->AcceptPlayerSession(shared_from_this(), playerId))
	{
		mPlayerSessionId = playerId;

		GConsoleLog->PrintOut(true, "[PLAYER] PlayerLogin: %s \n", playerId.c_str());

		LoginResult outPacket;
		strcpy(outPacket.mPlayerId, playerId.c_str());

		if (SendRequest(&outPacket))
			return;

		GConsoleLog->PrintOut(true, "send request error: %s \n", playerId.c_str());
	}

	/// disconnect: unauthed player or send rrror
	Disconnect();
}

void ClientSession::PlayerLogout(const std::string& playerId)
{
	GGameLiftManager->RemovePlayerSession(shared_from_this(), playerId);
	mPlayerSessionId.clear();

	GConsoleLog->PrintOut(true, "[PLAYER] PlayerLogout: %s \n", playerId.c_str());
	Disconnect();

}

void ClientSession::OnDisconnect()
{
	if (IsValid())
	{
		GGameLiftManager->RemovePlayerSession(shared_from_this(), mPlayerSessionId);
		mPlayerSessionId.clear();
	}
}