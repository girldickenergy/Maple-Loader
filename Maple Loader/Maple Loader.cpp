#pragma once
#include <iostream>
#include "TCP/TCP_Client.h"
#include "TCP/Utils/SendByteVector.hpp"
#include "Utils/RSAKeygen.hpp"
#include "Utils/StringUtilities.cpp"

TcpClient client;
static inline RSADecrypt* RSAA = new RSADecrypt();

void onIncomingMsg(const char* msg, size_t size)
{
	const std::regex re(R"(0xdeadbeef)");
	const std::vector<std::string> tokenized = Split(std::string(msg, size), re);

	std::vector<unsigned char> encrypted = std::vector<unsigned char>();
	for (const auto& byte : tokenized[1])
		encrypted.push_back(byte);

	encrypted.erase(encrypted.begin());
	
	std::vector<unsigned char> s = RSAA->Decode(encrypted, std::stoi(tokenized[0]));
	//std::cout << "Got msg from server: " << s << std::endl;
}

void onDisconnection(const pipe_ret_t& ret)
{
	std::cout << "Server disconnected: " << ret.msg << std::endl;
	std::cout << "Closing client..." << std::endl;
	pipe_ret_t finishRet = client.finish();
	if (finishRet.success) {
		std::cout << "Client closed." << std::endl;
	} else {
		std::cout << "Failed to close client: " << finishRet.msg << std::endl;
	}
}

int main()
{
	// uncomment to generate and test new key, if leaked!
	//GenerateKey();
	/*BM2();
	return 0;*/
	client_observer_t observer;
	observer.wantedIp = "127.0.0.1";
	observer.incoming_packet_func = onIncomingMsg;
	observer.disconnected_func = onDisconnection;
	client.subscribe(observer);

	pipe_ret_t connectRet = client.connectTo("127.0.0.1", 9999);
	if (connectRet.success) {
		std::cout << "Client connected successfully" << std::endl;
	} else {
		std::cout << "Client failed to connect: " << connectRet.msg << std::endl;
		return EXIT_FAILURE;
	}

	// construct some unit tests.,.. writing this at 1am... been awake for 20 hours, kill me please
	message constructOne = message();
	constructOne.push_back(0x33); // request type -> heartbeat

	//message constructTwo = message();
	//constructTwo.push_back(0x01); // request type -> login

	pipe_ret_t sendRet = sendBytes(&client, constructOne);
	if (!sendRet.success)
		std::cout << "Failed to send msg: " << sendRet.msg << std::endl;

	while (1)
	{
		// wait for messages
		Sleep(1);
	}
	
	/*std::string msg = "[=>] mcf is living\n";
	int i = 0xFF;
	pipe_ret_t sendRet = client.sendMsg(reinterpret_cast<char*>(i), msg.size());
	if (!sendRet.success)
		std::cout << "Failed to send msg: " << sendRet.msg << std::endl;*/

	return 0;
}