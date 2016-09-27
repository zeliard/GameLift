/*
The MIT License (MIT)

Copyright (c) 2016 Seungmo Koo (@teralode)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/


#include "stdafx.h"

#include <aws/core/Aws.h>

#include "DummyClients.h"
#include "Exception.h"
#include "PlayerSession.h"
#include "GameSession.h"
#include "IocpManager.h"
#include "GameLiftManager.h"



int _tmain(int argc, _TCHAR* argv[])
{
	Aws::SDKOptions options;
	Aws::InitAPI(options);

	LThreadType = THREAD_MAIN;

	/// for dump on crash
	SetUnhandledExceptionFilter(ExceptionFilter);

	/// Global Managers
	GIocpManager = new IocpManager;
	GGameLiftManager = new GameLiftManager(ALIAS_ID);

	GGameLiftManager->SetUpAwsClient(GAMELIFT_REGION);

	if (false == GIocpManager->Initialize())
		return -1;

	GGameLiftManager->PrepareGameSessions(MAX_GAME_SESSIONS);

	if (false == GIocpManager->StartIoThreads())
		return -1;

	printf_s("Start Game Sessions...\n");

	GGameLiftManager->LaunchGameSessions();
	
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

