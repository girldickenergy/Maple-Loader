#include "Login.h"

void Login::constructPacket(std::string hwid, std::string username, std::string password)
{
	std::vector<unsigned char> data;

	AddString(hwid, &data);
	AddString(username, &data);
	AddString(password, &data);

	std::vector<unsigned char> encrypted = matchedClient->aes->Encrypt(data);

	AddString(std::to_string(encrypted.size()));
	AddString(encrypted);
}

Login::Login(std::string hwid, std::string username, std::string password, MatchedClient* matchedClient) : Packet(PacketType::Login, matchedClient)
{
	constructPacket(hwid, username, password);
}
