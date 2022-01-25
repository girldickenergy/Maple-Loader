#pragma once

#include <Wrapper/RSADecrypt.h>

#include "MatchedClient.h"
#include "User.h"
#include "TCP/TCP_Client.h"
#include "States.h"

class Communication
{
	static void onIncomingMessage(const char* msg, size_t size);
	static void onDisconnection(const pipe_ret_t& ret);
public:
	static inline States CurrentState = States::Idle;

	static inline RSADecrypt RSA = RSADecrypt();
	static inline TcpClient TCPClient = TcpClient();
	static inline MatchedClient* MatchedClient = nullptr;

	static inline User* CurrentUser = new User();
	
	static bool ConnectToServer();
};
