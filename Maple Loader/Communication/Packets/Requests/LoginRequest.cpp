#include "LoginRequest.h"

LoginRequest::LoginRequest(std::string hwid, std::string hash, std::string username, std::string password, MatchedClient* matchedClient) : Request(RequestType::Login)
{
	VM_SHARK_BLACK_START
	std::vector<unsigned char> data;

	AddString(hwid, &data);
	AddString(hash, &data);
	AddString(username, &data);
	AddString(password, &data);

	AddString(matchedClient->aes->Encrypt(data));
	VM_SHARK_BLACK_END
}
