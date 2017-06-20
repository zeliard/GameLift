#include "stdafx.h"

#include <aws/core/Aws.h>

#include "PacketType.h"
#include "DummyClients.h"
#include "Exception.h"
#include "PlayerSession.h"
#include "GameSession.h"
#include "IocpManager.h"
#include "GameLiftManager.h"
#include "INIReader.h"

int PLAYER_ACTION_REQUEST = 0;
int TEST_PLAYER_SESSION_EXCEPT = 0;
int PLAYER_ACTION_INTERVAL = 1000;

int _tmain(int argc, _TCHAR* argv[])
{
	INIReader iniReader("config.ini");
	if (iniReader.ParseError() < 0)
	{
		printf_s("config.ini not found\n");
		return 0;
	}

	const std::string& aliasId = iniReader.Get("config", "ALIAS_ID", "TEST_LOCAL");
	const std::string& region = iniReader.Get("config", "GAMELIFT_REGION", "127.0.0.1:9080");
	int maxGameSessionCount = iniReader.GetInteger("config", "MAX_GAME_SESSIONS", 1);

	PLAYER_ACTION_REQUEST = iniReader.GetInteger("config", "PLAYER_ACTION_REQUEST", 60);
	PLAYER_ACTION_INTERVAL = iniReader.GetInteger("config", "PLAYER_ACTION_INTERVAL", 1000);

	int dummySessionCountPerGameSession = iniReader.GetInteger("config", "DUMMY_PLAYER_SESSION_PER_GAME_SESSION", 8);
	TEST_PLAYER_SESSION_EXCEPT = MAX_PLAYER_PER_GAME - dummySessionCountPerGameSession;

	Aws::SDKOptions options;
	Aws::InitAPI(options);

	LThreadType = THREAD_MAIN;

	/// for dump on crash
	SetUnhandledExceptionFilter(ExceptionFilter);

	/// Global Managers
	GIocpManager = new IocpManager;
	GGameLiftManager = new GameLiftManager(aliasId, region);

	GGameLiftManager->SetUpAwsClient(region);

	if (false == GIocpManager->Initialize())
		return -1;

	GGameLiftManager->PrepareGameSessions(maxGameSessionCount);

	if (false == GIocpManager->StartIoThreads())
		return -1;

	printf_s("Start Game Sessions...\n");

#ifdef USE_MATCH_QUEUE
	GGameLiftManager->LaunchGameSessionPlacement();
#else
	GGameLiftManager->LaunchGameSessions();
#endif
	
	
	/// block here...
	getchar();

	printf_s("Terminating Game Sessions...\n");

	/// Disconnect request here
	GGameLiftManager->TerminateGameSessions();

	GIocpManager->StopIoThreads();

	GIocpManager->Finalize();


	Aws::ShutdownAPI(options);
	return 0;
}

