#include "Response.h"

#include "FatalErrorResponse.h"
#include "HandshakeResponse.h"
#include "LoginResponse.h"
#include "../../Utils/StringUtilities.h"

Response::Response(const char* msg, size_t size)
{
	RawResponse = msg;
	ResponseSize = size;
	Type = static_cast<ResponseType>(RawResponse[0]);
	
	RawData = StringUtilities::Split(std::string(RawResponse, ResponseSize));
	RawData.erase(RawData.begin());
}

void* Response::ConstructResponse(const char* msg, size_t size, MatchedClient* matchedClient)
{
	auto type = static_cast<ResponseType>(msg[0]);
	switch (type)
	{
		case ResponseType::FatalError:
			return new FatalErrorResponse(msg, size);
		case ResponseType::Handshake:
			return new HandshakeResponse(msg, size);
		case ResponseType::Login:
			return new LoginResponse(msg, size, matchedClient);
	}

	return new Response(msg, size);
}
