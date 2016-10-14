#pragma once

#include "ClientSession.h"

class ClientManager
{
public:
	ClientManager() : mEpollFd(-1), mListenSocket(-1)
	{}

	bool Initialize(int listenPort);

	std::shared_ptr<ClientSession> CreateClient(SOCKET sock);
	void DeleteClient(std::shared_ptr<ClientSession> client);

	void EventLoop();

	void FlushClientSend();


private:
	typedef std::map<SOCKET, std::shared_ptr<ClientSession>> ClientList;
	ClientList	mClientList;

	SOCKET		mEpollFd;
	SOCKET		mListenSocket;
} ;

extern std::unique_ptr<ClientManager> GClientManager;
