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
#include "Exception.h"
#include "Log.h"
#include "Session.h"
#include "IocpManager.h"
#include "GameLiftManager.h"


int main(int argc, char* argv[])
{
	int portNum = 9001;
	if (argc < 2)
		printf_s("Specify Listen Port Number\n");
	else
		portNum = atoi(argv[1]);


	LThreadType = THREAD_MAIN;

	/// for dump on crash
	SetUnhandledExceptionFilter(ExceptionFilter);

	GConsoleLog.reset(new ConsoleLog(".\\logs\\serverLog.txt"));

	/// GameLift
	GGameLiftManager.reset(new GameLiftManager);

	/// Global Managers
	GIocpManager.reset(new IocpManager);

	/// Gamelift init/start!
	if (false == GGameLiftManager->InitializeGameLift(portNum))
		return -1;

	if (false == GIocpManager->Initialize(static_cast<u_short>(portNum)))
		return -1;

	if (false == GIocpManager->StartIoThreads())
		return -1;

	GConsoleLog->PrintOut(true, "Start Server\n");

	GIocpManager->StartAccept(); ///< block here...

	GIocpManager->Finalize();

	/// Gamelift end!
	GGameLiftManager->FinalizeGameLift();

	GConsoleLog->PrintOut(true, "End Server\n");

	return 0;
}


