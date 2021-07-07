#pragma once

#include "Response.h"

class HandshakeResponse : public Response
{
public:
	std::vector<unsigned char> IV;
	std::vector<unsigned char> Key;

	HandshakeResponse(const char* msg, size_t size);
};
