#pragma once

#include "Request.h"
#include "../../Communication/MatchedClient.h"

class DllStreamRequest : public Request
{
public:
	DllStreamRequest(MatchedClient* matchedClient);
};
