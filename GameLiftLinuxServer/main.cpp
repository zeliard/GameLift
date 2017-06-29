#include <iostream>
#include <thread>

#include "Config.h"
#include "Log.h"
#include "Scheduler.h"
#include "ClientManager.h"
#include "GameLiftManager.h"

int main(int argc, char* argv[])
{
	int portNum = 0;
	/// listen port override rather than dynamic port by OS
	if (argc < 2)
		printf("Specify Listen Port Number. Or, it will be used dynamic port by OS\n");
	else
		portNum = atoi(argv[1]);



	GConsoleLog.reset(new ConsoleLog("./logs/serverLog.txt"));

	/// GameLift
	GGameLiftManager.reset(new GameLiftManager);

	/// Managers
	GScheduler.reset(new Scheduler);
	GClientManager.reset(new ClientManager);

	/// bind and listen
	if (false == GClientManager->Initialize(portNum))
		return -1;

	/// Gamelift init/start!
#ifndef ECHO_MODE
	if (false == GGameLiftManager->InitializeGameLift(portNum))
		return -1;
#endif // !ECHO_MODE

	printf("Server Started...\n");

	GClientManager->EventLoop(); ///< block here

	GGameLiftManager->FinalizeGameLift();

	return 0;
}