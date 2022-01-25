#pragma once

#include "Request.h"
#include "../../Communication/MatchedClient.h"

class FinalDllStreamRequest : public Request
{
public:
	FinalDllStreamRequest(unsigned int allocationBase, std::vector<unsigned int> imports, MatchedClient* matchedClient);
};