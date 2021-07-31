#pragma once

#include "Response.h"

#include "../../Cheat.h"
#include "../../Game.h"

enum class LoginResult : unsigned char
{
	Success = 0x0,
	IncorrectCredentials = 0x1,
	HWIDMismatch = 0x2,
	Banned = 0x3,
	InternalError = 0x4
};

class LoginResponse : public Response
{
public:
	LoginResult Result;
	std::string SessionToken;
	std::vector<Game*> Games;
	std::vector<Cheat*> Cheats;

	LoginResponse(const char* msg, size_t size, MatchedClient* matchedClient);
};
