#pragma once

#include "Response.h"
class DllStreamResponse : public Response
{
public:
	   DllStreamResponse(const char* msg, size_t size, MatchedClient* matchedClient);
};
