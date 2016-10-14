#include <aws/gamelift/server/GameLiftServerAPI.h>
#include "Log.h"
#include "../PacketShare/PacketType.h"
#include "GameLiftManager.h"
#include "ClientSession.h"


std::unique_ptr<GameLiftManager> GGameLiftManager(nullptr);

GameLiftManager::GameLiftManager() : mActivated(false), mAcceptedCount(0), mRemovedCount(0)
{
}

bool GameLiftManager::InitializeGameLift(int listenPort)
{
	auto initOutcome = Aws::GameLift::Server::InitSDK();

	if (!initOutcome.IsSuccess())
		return false;

	std::string serverOut("./logs/serverOut.log");
	std::string serverErr("./logs/serverErr.log");
	std::vector<std::string> logPaths;
	logPaths.push_back(serverOut);
	logPaths.push_back(serverErr);

	auto processReadyParameter = Aws::GameLift::Server::ProcessParameters(
		std::bind(&GameLiftManager::OnStartGameSession, this, std::placeholders::_1),
		std::bind(&GameLiftManager::OnProcessTerminate, this),
		std::bind(&GameLiftManager::OnHealthCheck, this),
		listenPort, Aws::GameLift::Server::LogParameters(logPaths)
		);

	auto readyOutcome = Aws::GameLift::Server::ProcessReady(processReadyParameter);

	if (!readyOutcome.IsSuccess())
		return false;

	mActivated = true;

	mAcceptedCount = 0;
	mRemovedCount = 0;

	GConsoleLog->PrintOut(true, "[GAMELIFT] ProcessReady Success\n");
	return true;
}


void GameLiftManager::FinalizeGameLift()
{
	Aws::GameLift::Server::Destroy();
}

bool GameLiftManager::AcceptPlayerSession(std::shared_ptr<ClientSession> psess, const std::string& playerSessionId)
{
	
	auto outcome = Aws::GameLift::Server::AcceptPlayerSession(playerSessionId);

	if (outcome.IsSuccess())
	{
		++mAcceptedCount;
		mPlayerSessions.insert(psess);
		return true;
	}
	
	GConsoleLog->PrintOut(true, "[GAMELIFT] AcceptPlayerSession Fail: %s\n", outcome.GetError().GetErrorMessage().c_str());
	
	return false;
}

void GameLiftManager::RemovePlayerSession(std::shared_ptr<ClientSession> psess, const std::string& playerSessionId)
{
	
	auto outcome = Aws::GameLift::Server::RemovePlayerSession(playerSessionId);
	if (outcome.IsSuccess())
	{
		mPlayerSessions.erase(psess);
		++mRemovedCount;
	}
	else
	{
		GConsoleLog->PrintOut(true, "[GAMELIFT] RemovePlayerSession Fail: %d %s\n", 
			outcome.GetError().GetErrorType(),
			outcome.GetError().GetErrorName().c_str());
	}
	
	CheckTerminateGameSession();
}

void GameLiftManager::OnStartGameSession(Aws::GameLift::Server::Model::GameSession myGameSession)
{
	Aws::GameLift::Server::ActivateGameSession();

	GConsoleLog->PrintOut(true, "[GAMELIFT] OnStartGameSession Success\n");
}

void GameLiftManager::OnProcessTerminate()
{

	// game-specific tasks required to gracefully shut down a game session, 
	// such as notifying players, preserving game state data, and other cleanup

	Aws::GameLift::Server::TerminateGameSession();

	Aws::GameLift::Server::ProcessEnding();

	mActivated = false;

	GConsoleLog->PrintOut(true, "[GAMELIFT] OnProcessTerminate Success\n");
}

void GameLiftManager::CheckTerminateGameSession()
{
	// accepted full, but no ones here
	if (mAcceptedCount >= MAX_PLAYER_PER_GAME && mRemovedCount >= MAX_PLAYER_PER_GAME)
	{
		GConsoleLog->PrintOut(true, "[GAMELIFT] Terminate GameSession\n");

		Aws::GameLift::Server::TerminateGameSession();
			
		Aws::GameLift::Server::ProcessEnding();
		mActivated = false;
		
	}
}

void GameLiftManager::BroadcastMessage(PacketHeader* pkt)
{
	for (auto& it : mPlayerSessions)
	{
		if (false == it->SendRequest(pkt))
		{
			it->Disconnect();
		}
	}

}