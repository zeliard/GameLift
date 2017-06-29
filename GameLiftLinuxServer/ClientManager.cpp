#include <stdio.h>
#include <typeinfo>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <algorithm>

#include "Log.h"
#include "../PacketShare/PacketType.h"
#include "Utils.h"
#include "ClientSession.h"
#include "ClientManager.h"

#include "Scheduler.h"

std::unique_ptr<ClientManager> GClientManager(nullptr);


bool ClientManager::Initialize(int& listenPort)
{
	mListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mListenSocket < 0)
		return false;

	int opt = 1;
	setsockopt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));

	/// bind
	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(sockaddr_in));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(listenPort);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = bind(mListenSocket, (sockaddr*)&serveraddr, sizeof(serveraddr));
	if (ret < 0)
		return false;

	/// listen
	ret = listen(mListenSocket, SOMAXCONN);
	if (ret < 0)
		return false;

	mEpollFd = epoll_create(MAX_CONNECTION);
	if (mEpollFd < 0)
		return false;

	epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = mListenSocket;
	ret = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mListenSocket, &ev);
	if (ret < 0)
		return false;

	memset(&serveraddr, 0, sizeof(sockaddr_in));
	socklen_t len = sizeof(serveraddr);
	ret = getsockname(mListenSocket, (sockaddr*)&serveraddr, &len);
	if (ret < 0)
		return false;

	listenPort = ntohs(serveraddr.sin_port);

	return true;

}

void ClientManager::EventLoop()
{
	epoll_event events[MAX_CONNECTION];

	while (true)
	{
		int nfd = epoll_wait(mEpollFd, events, MAX_CONNECTION, POLL_INTERVAL);

		for (int i = 0; i < nfd; ++i)
		{
			if (events[i].data.fd == mListenSocket)
			{
				sockaddr_in clientAddr;
				socklen_t clientAddrLen = sizeof(sockaddr_in);

				SOCKET client = accept(mListenSocket, (sockaddr *)&clientAddr, &clientAddrLen);
				if (client < 0) 
				{
					GConsoleLog->PrintOut(true, "accept error: IP=%s, PORT=%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
					continue;
				}

				auto newClient = CreateClient(client);
				newClient->OnConnect(&clientAddr);

				epoll_event ev;
				memset(&ev, 0, sizeof(ev));
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = client;
				epoll_ctl(mEpollFd, EPOLL_CTL_ADD, client, &ev);
			}
			else
			{
				SOCKET client = events[i].data.fd;
				auto session = mClientList[client];
				session->OnReceive();
			}
		}

		FlushClientSend();

		/// scheduled jobs
		GScheduler->DoTasks();
	 
	}

}

std::shared_ptr<ClientSession> ClientManager::CreateClient(SOCKET sock)
{
	auto client = std::make_shared<ClientSession>(sock);
	mClientList.insert(ClientList::value_type(sock, client)) ;

	return client ;
}

void ClientManager::DeleteClient(std::shared_ptr<ClientSession> client)
{
	epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	epoll_ctl(mEpollFd, EPOLL_CTL_DEL, client->mSocket, &ev);

	mClientList.erase(client->mSocket);
}


void ClientManager::FlushClientSend()
{
	for (auto& it : mClientList)
	{
		auto client = it.second;
		if (false == client->SendFlush())
		{
			client->Disconnect();
		}
	}
}

