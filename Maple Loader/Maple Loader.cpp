#pragma once
#include <iostream>
#include "TCP/TCP_Client.h"

TcpClient client;

void onIncomingMsg(const char* msg, size_t size)
{
	std::cout << "Got msg from server: " << msg << std::endl;
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
	client_observer_t observer;
	observer.wantedIp = "127.0.0.1";
	observer.incoming_packet_func = onIncomingMsg;
	observer.disconnected_func = onDisconnection;
	client.subscribe(observer);

	pipe_ret_t connectRet = client.connectTo("195.58.39.35", 9999);
	if (connectRet.success) {
		std::cout << "Client connected successfully" << std::endl;
	} else {
		std::cout << "Client failed to connect: " << connectRet.msg << std::endl;
		return EXIT_FAILURE;
	}

	std::string msg = "[=>] mcf is living\n";
	pipe_ret_t sendRet = client.sendMsg(msg.c_str(), msg.size());
	if (!sendRet.success)
		std::cout << "Failed to send msg: " << sendRet.msg << std::endl;

	return 0;
}