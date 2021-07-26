#include "DllStreamRequest.h"

DllStreamRequest::DllStreamRequest(MatchedClient* matchedClient) : Request(RequestType::DllStream)
{
	std::vector<unsigned char> data;

	AddString("osu-lite", &data);

	AddString(matchedClient->aes->Encrypt(data));
}
