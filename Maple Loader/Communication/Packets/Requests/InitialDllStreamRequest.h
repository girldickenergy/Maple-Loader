#pragma once

#include "Request.h"
#include "../../Communication/MatchedClient.h"

class InitialDllStreamRequest : public Request
{
public:
	InitialDllStreamRequest(int cheatID, std::string releaseStream, MatchedClient* matchedClient);
};
