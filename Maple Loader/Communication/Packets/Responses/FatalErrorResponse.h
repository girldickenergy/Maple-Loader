#pragma once

#include "Response.h"

class FatalErrorResponse : public Response
{
public:
	std::string ErrorMessage;

	FatalErrorResponse(const char* msg, size_t size);
};
