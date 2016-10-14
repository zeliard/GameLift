#pragma once

/// 타이머 실행 간격 (ms)
#define POLL_INTERVAL	100

/// 플레이어의 HeartBeat 주기
#define PLAYER_HEART_BEAT	1000

/// 세션별 send/recv 버퍼 크기
#define BUFSIZE	(1024*10)

/// 최대 동접 제한
#define MAX_CONNECTION	1024


/// No GameLift Mode (just for a packet test)
//#define ECHO_MODE
