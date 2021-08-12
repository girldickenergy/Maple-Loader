#include "LoginRequest.h"

LoginRequest::LoginRequest(std::string hwid, std::string hash, std::string username, std::string password, MatchedClient* matchedClient) : Request(RequestType::Login)
{
	std::vector<unsigned char> data;

	AddString(hwid, &data);
	AddString("D36220CABCAE43EFE8257902D3F712648A15D6D143D865F1951EFBDCD01A2B9A", &data);
	AddString(username, &data);
	AddString(password, &data);

	AddString(matchedClient->aes->Encrypt(data));
	matchedClient->username = username;
}
