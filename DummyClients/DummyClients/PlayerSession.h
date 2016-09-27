#pragma once

#include "Session.h"

class GameSession;

class PlayerSession : public Session, public ObjectPool<PlayerSession>
{
public:
	PlayerSession(GameSession* owner);
	virtual ~PlayerSession();

	bool PrepareSession();

	bool ConnectRequest(const std::string& playerSessionId, int idx);
	void ConnectCompletion();

	virtual void OnRead(size_t len);

	virtual void OnDisconnect(DisconnectReason dr);

	template <class PKT_TYPE>
	bool ParsePacket(PKT_TYPE& pkt)
	{
		return mRecvBuffer.Read((char*)&pkt, pkt.mSize);
	}


	void StartTest();
	void EndTest();

	void PlayTest();


private:
	void SetNextPos();


	SOCKADDR_IN mConnectAddr;

	std::string mPlayerSessionId;
	GameSession* mOwnerGameSession;

	int mPlayerIdx;
	float mPosX;
	float mPosY;

	int mCurrDir; ///< 0~7
	int mMoveStep;

	int mPlayTestCount;
} ;



