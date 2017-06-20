#pragma once

/// If you want test GameLift Match Queue Functionality, define USE_MATCH_QUEUE
//#define USE_MATCH_QUEUE
#define MATCH_QUEUE_NAME	"GameLiftSeoulQueue"

///FYI: GameLift Local Mode does not support Match Queue 


/// request sending interval (ms)
#define	PLAYER_ACTION_INTERVAL	100

/// How many times to send a request
extern int PLAYER_ACTION_REQUEST;

/// Test Player Session Count == mMaxPlayerCount - TEST_PLAYER_SESSION_EXCEPT
extern int TEST_PLAYER_SESSION_EXCEPT;

