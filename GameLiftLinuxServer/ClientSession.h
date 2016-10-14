#pragma once

#include <map>
#include <sys/socket.h>
#include <netdb.h>
#include <memory>

#include "Config.h"
#include "../PacketShare/PacketType.h"
#include "CircularBuffer.h"


typedef int SOCKET;

class ClientSession;
class ClientManager;


class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:
	ClientSession(SOCKET sock)
		: mConnected(false), mLogon(false), mSocket(sock), mSendBuffer(BUFSIZE), mRecvBuffer(BUFSIZE)
	{
		memset(&mClientAddr, 0, sizeof(sockaddr_in));
	}

	virtual ~ClientSession();

	void	OnConnect(sockaddr_in* addr);
	void	OnDisconnect();
	void	Disconnect();

	void	OnReceive();

	bool	SendRequest(PacketHeader* pkt);
	bool	SendFlush(); 

public:

	bool IsValid() { return mPlayerSessionId.length() > 0; }

	void PlayerLogin(const std::string& playerId);
	void PlayerLogout(const std::string& playerId);
	

public: 
	bool	IsConnected() const { return mConnected; }
	void	OnTick();


	template <class PKT_TYPE>
	bool ParsePacket(PKT_TYPE& pkt)
	{
		return mRecvBuffer.Read((char*)&pkt, pkt.mSize);
	}

	void DispatchPacket();
	void EchoBack();

private:

	std::string	mPlayerSessionId;

	bool			mConnected;
	bool			mLogon;
	SOCKET			mSocket;

	sockaddr_in		mClientAddr;

	CircularBuffer	mSendBuffer;
	CircularBuffer	mRecvBuffer;

	friend class ClientManager;
};


