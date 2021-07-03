#pragma once

#include "Packet.h"

class Login : public Packet
{
	void constructPacket(std::string hwid, std::string username, std::string password);
public:
	Login(std::string hwid, std::string username, std::string password, MatchedClient* matchedClient);
};
