#include "Packet.h"

Packet::Packet(PacketType type, MatchedClient* matchedClient)
{
	Type = type;
	this->matchedClient = matchedClient;

	AddByte(static_cast<unsigned char>(Type));
}

void Packet::AddDelimiter(std::vector<unsigned char>* dest)
{
	for (const auto& c : "0xdeadbeef")
		dest->push_back(c);
}

void Packet::AddByte(unsigned char byte, std::vector<unsigned char>* dest)
{
	if (!dest->empty())
		AddDelimiter(dest);
	
	dest->push_back(byte);
}

void Packet::AddString(std::string str, std::vector<unsigned char>* dest)
{
	if (!dest->empty())
		AddDelimiter(dest);
	
	for (const auto& c : str)
		dest->push_back(c);
}

void Packet::AddString(std::vector<unsigned char> str, std::vector<unsigned char>* dest)
{
	if (!dest->empty())
		AddDelimiter(dest);
	
	for (const auto& c : str)
		dest->push_back(c);
}

void Packet::AddByte(unsigned char byte)
{
	AddByte(byte, &Data);
}

void Packet::AddString(std::string str)
{
	AddString(str, &Data);
}

void Packet::AddString(std::vector<unsigned char> str)
{
	AddString(str, &Data);
}
