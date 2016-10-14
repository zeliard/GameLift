#include <iostream>
#include <thread>

#include "Config.h"
#include "Log.h"
#include "Scheduler.h"
#include "ClientManager.h"
#include "GameLiftManager.h"

int main(int argc, char* argv[])
{
	int portNum = 9001;
	if (argc < 2)
		printf("Specify Listen Port Number\n");
	else
		portNum = atoi(argv[1]);


	GConsoleLog.reset(new ConsoleLog("./logs/serverLog.txt"));

	/// GameLift
	GGameLiftManager.reset(new GameLiftManager);

	/// Managers
	GScheduler.reset(new Scheduler);
	GClientManager.reset(new ClientManager);

	/// Gamelift init/start!
#ifndef ECHO_MODE
	if (false == GGameLiftManager->InitializeGameLift(portNum))
		return -1;
#endif // !ECHO_MODE
	
	/// bind and listen
	if (false == GClientManager->Initialize(portNum))
		return -1;

	printf("Server Started...\n");

	GClientManager->EventLoop(); ///< block here

	GGameLiftManager->FinalizeGameLift();

    return 0;
}