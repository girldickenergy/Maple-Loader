/*
	made by azuki for maple.software and the maple project / the mcf
			2021
*/

#pragma once
#include "../pipe_ret_t.h"
#include <vector>
#include "../TCP_Client.h"

typedef unsigned char byte;
typedef std::vector<byte> message;

/*
	the message typedef:
	[0x00] -> request type
*/

/*
	request type:
	0x30: heartbeat
		-> hwid, session
	0x31: login
		-> hwid, username, password
			-> returns session
	0x32: requestmaple
		-> hwid, session
			-> outputs maple dll
*/

// TODO: organize everything

static pipe_ret_t sendBytes(TcpClient *tcpC, message bytes)
{
	std::string msg;
	for (int i = 0; i < bytes.size(); i++) {
		byte b = bytes[i];
		msg += (char)b;
	}
	return tcpC->sendMsg(msg.c_str(), bytes.size());
}

static void addStringToBytes(std::string str, message* bytes)
{
	for (char& c : str)
		bytes->push_back((char)c);
	// end of string deliminator
	bytes->push_back('|');
}