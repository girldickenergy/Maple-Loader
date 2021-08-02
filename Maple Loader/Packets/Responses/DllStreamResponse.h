#pragma once

#include "Response.h"

enum class DllStreamResult : unsigned char
{
	Success = 0x0,
	NotSubscribed = 0x1,
	InvalidSession = 0x2,
	InternalError = 0x3
};

class DllStreamResponse : public Response
{
public:
	DllStreamResult Result;
	
	DllStreamResponse(const char* msg, size_t size, MatchedClient* matchedClient);
};
