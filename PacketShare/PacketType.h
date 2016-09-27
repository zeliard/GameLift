#pragma once

#ifndef WIN32
#include <string.h>
#endif // !WIN32

#define MAX_CHAT_LEN	128

#define MAX_SESSION_LEN	128

#define MAX_PLAYER_PER_GAME	8

enum PacketTypes
{
	PKT_NONE	= 0,
	
	PKT_CS_LOGIN	= 1,
	PKT_SC_LOGIN	= 2,
	
	PKT_CS_CHAT		= 11,
	PKT_SC_CHAT		= 12,

	PKT_CS_MOVE		= 21,
	PKT_SC_MOVE		= 22,

	PKT_CS_EXIT		= 31,

	PKT_MAX	= 1024
} ;

#pragma pack(push, 1)

struct PacketHeader
{
	PacketHeader() : mSize(0), mType(PKT_NONE) 	{}
	short mSize ;
	short mType ;
} ;



struct LoginRequest : public PacketHeader
{
	LoginRequest()
	{
		mSize = sizeof(LoginRequest) ;
		mType = PKT_CS_LOGIN ;
		memset(mPlayerId, 0, sizeof(mPlayerId));
	}

	char mPlayerId[MAX_SESSION_LEN];
} ;

struct LoginResult : public PacketHeader
{
	LoginResult()
	{
		mSize = sizeof(LoginResult) ;
		mType = PKT_SC_LOGIN ;
		memset(mPlayerId, 0, sizeof(mPlayerId));
	}

	char	mPlayerId[MAX_SESSION_LEN];

} ;

struct ChatBroadcastRequest : public PacketHeader
{
	ChatBroadcastRequest()
	{
		mSize = sizeof(ChatBroadcastRequest) ;
		mType = PKT_CS_CHAT ;

		memset(mPlayerId, 0, sizeof(mPlayerId));
		memset(mChat, 0, sizeof(mChat));
	}

	char mPlayerId[MAX_SESSION_LEN];
	char mChat[MAX_CHAT_LEN];
} ;

struct ChatBroadcastResult : public PacketHeader
{
	ChatBroadcastResult()
	{
		mSize = sizeof(ChatBroadcastResult) ;
		mType = PKT_SC_CHAT ;

		memset(mPlayerId, 0, sizeof(mPlayerId));
		memset(mChat, 0, sizeof(mChat));
	}
	
	char mPlayerId[MAX_SESSION_LEN];
	char mChat[MAX_CHAT_LEN];
} ;

struct MoveRequest : public PacketHeader
{
	MoveRequest()
	{
		mSize = sizeof(MoveRequest);
		mType = PKT_CS_MOVE;
		mPlayerIdx = -1;
		mPosX = 0;
		mPosY = 0;
	}

	int		mPlayerIdx;
	float	mPosX;
	float	mPosY;
};

struct MoveBroadcastResult : public PacketHeader
{
	MoveBroadcastResult()
	{
		mSize = sizeof(MoveBroadcastResult);
		mType = PKT_SC_MOVE;
		mPlayerIdx = -1;
		mPosX = 0;
		mPosY = 0;
	}

	int		mPlayerIdx;
	float	mPosX;
	float	mPosY;
};




struct ExitRequest : public PacketHeader
{
	ExitRequest()
	{
		mSize = sizeof(ExitRequest);
		mType = PKT_CS_EXIT;

		memset(mPlayerId, 0, sizeof(mPlayerId));
	}

	char mPlayerId[MAX_SESSION_LEN];

};




#pragma pack(pop)