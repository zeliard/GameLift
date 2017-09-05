#include "stdafx.h"

#include <aws/core/Aws.h>

#include "PacketType.h"
#include "DummyClients.h"
#include "Exception.h"
#include "PlayerSession.h"
#include "IocpManager.h"
#include "GameLiftManager.h"
#include "Log.h"
#include "INIReader.h"


int PLAYER_ACTION_REQUEST = 0;
int PLAYER_ACTION_INTERVAL = 1000;

int _tmain(int argc, _TCHAR* argv[])
{
	INIReader iniReader("config.ini");
	if (iniReader.ParseError() < 0)
	{
		printf_s("config.ini not found\n");
		return 0;
	}

	const std::string& region = iniReader.Get("config", "GAMELIFT_REGION", "ap-northeast-2");
	const std::string& matchconfig = iniReader.Get("config", "MATCHMAKING_CONFIG_NAME", "MatchConfig");

	int maxPlayerSessionCount = iniReader.GetInteger("config", "MAX_PLAYER_SESSIONS", 8);

	PLAYER_ACTION_REQUEST = iniReader.GetInteger("config", "PLAYER_ACTION_REQUEST", 60);
	PLAYER_ACTION_INTERVAL = iniReader.GetInteger("config", "PLAYER_ACTION_INTERVAL", 1000);


	Aws::SDKOptions options;
	Aws::InitAPI(options);

	LThreadType = THREAD_MAIN;

	/// for dump on crash
	SetUnhandledExceptionFilter(ExceptionFilter);

	/// Global Managers
	GConsoleLog = new ConsoleLog(".\\DummyClientsLog.txt");
	GIocpManager = new IocpManager;
	GGameLiftManager = new GameLiftManager(matchconfig);

	GGameLiftManager->SetUpAwsClient(region);

	if (false == GIocpManager->Initialize())
		return -1;

	GGameLiftManager->PreparePlayerSessions(maxPlayerSessionCount);

	if (false == GIocpManager->StartIoThreads())
		return -1;

	printf_s("Start Game Sessions...\n");


	GGameLiftManager->LaunchPlayerSessions();


	/// block here...
	getchar();

	printf_s("Terminating Game Sessions...\n");

	/// Disconnect request here
	GGameLiftManager->TerminatePlayerSessions();

	GIocpManager->StopIoThreads();

	GIocpManager->Finalize();

	Aws::ShutdownAPI(options);

	delete GConsoleLog;
	return 0;
}

