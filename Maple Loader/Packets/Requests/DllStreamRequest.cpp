#include "DllStreamRequest.h"

DllStreamRequest::DllStreamRequest(int cheatID, std::string releaseStream, MatchedClient* matchedClient) : Request(RequestType::DllStream)
{
	std::vector<unsigned char> data;

	AddString(std::to_string(cheatID), &data);
	AddString(releaseStream, &data);

	AddString(matchedClient->aes->Encrypt(data));
}
