#include "DllStreamRequest.h"

DllStreamRequest::DllStreamRequest(int cheatID, std::string releaseStream, MatchedClient* matchedClient) : Request(RequestType::DllStream)
{
	VM_SHARK_BLACK_START
	std::vector<unsigned char> data;

	AddString(std::to_string(cheatID), &data);
	AddString(releaseStream, &data);

	AddString(matchedClient->aes->Encrypt(data));
	VM_SHARK_BLACK_END
}