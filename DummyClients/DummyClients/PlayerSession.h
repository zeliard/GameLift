#pragma once

#include "Session.h"


class PlayerSession : public Session, public ObjectPool<PlayerSession>
{
public:
	PlayerSession(const std::string& playerId);
	virtual ~PlayerSession();

	bool PrepareSession();
	bool StartMatchMaking();
	void TrackMatchMaking();

	bool ConnectRequest();
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

	/// retrieved info
	int mPort;
	std::string mIpAddress;
	std::string mPlayerSessionId;

	std::string mTicketId; ///< MatchMaking Ticket Id for tracking

	std::string mPlayerId;

	float mPosX;
	float mPosY;

	int mCurrDir; ///< 0~7
	int mMoveStep;

	int mPlayTestCount;
} ;



