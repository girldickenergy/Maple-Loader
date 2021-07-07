#pragma once

#include "Response.h"

enum class LoginResult : unsigned char
{
	Success = 0x0,
	IncorrectCredentials = 0x1,
	HWIDMismatch = 0x2
};

class LoginResponse : public Response
{
public:
	LoginResult Result;
	std::string SessionToken;
	std::string ExpiresAt;

	LoginResponse(const char* msg, size_t size, MatchedClient* matchedClient);
};
