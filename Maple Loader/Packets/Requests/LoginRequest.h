#pragma once

#include "Request.h"
#include "../../Communication/MatchedClient.h"

class LoginRequest : public Request
{
public:
	LoginRequest(std::string hwid, std::string username, std::string password, MatchedClient* matchedClient);
};
