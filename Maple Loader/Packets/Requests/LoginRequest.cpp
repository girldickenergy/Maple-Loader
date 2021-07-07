#include "LoginRequest.h"

LoginRequest::LoginRequest(std::string hwid, std::string username, std::string password, MatchedClient* matchedClient) : Request(RequestType::Login)
{
	std::vector<unsigned char> data;

	AddString(hwid, &data);
	AddString(username, &data);
	AddString(password, &data);

	std::vector<unsigned char> encrypted = matchedClient->aes->Encrypt(data);

	AddString(std::to_string(encrypted.size()));
	AddString(encrypted);
}
