#pragma once

#include <vector>

#include "PacketType.h"
#include "../Communication/Client.cpp"

class Packet
{
protected:
	MatchedClient* matchedClient;
public:
	PacketType Type;
	std::vector<unsigned char> Data; //type | data size | data

	Packet(PacketType type, MatchedClient* matchedClient);

	void AddDelimiter(std::vector<unsigned char>* dest);
	void AddByte(unsigned char byte, std::vector<unsigned char>* dest);
	void AddString(std::string str, std::vector<unsigned char>* dest);
	void AddString(std::vector<unsigned char> str, std::vector<unsigned char>* dest);
	void AddByte(unsigned char byte);
	void AddString(std::string str);
	void AddString(std::vector<unsigned char> str);
};
