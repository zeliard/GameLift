#pragma once
#include "Session.h"

class PlayerSession : public Session
{
public:
	PlayerSession(SOCKET sock);
	virtual ~PlayerSession();

	bool IsValid() { return mPlayerSessionId.length() > 0; }

	void PlayerLogin(const std::string& playerId);
	void PlayerLogout(const std::string& playerId);
	
	virtual void OnRead(size_t len);
	virtual void OnDisconnect(DisconnectReason dr);

private:

	std::string	mPlayerSessionId;

};


