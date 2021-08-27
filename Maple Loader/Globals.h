#pragma once

#include "TCP/TCP_Client.h"
#include "Communication/MatchedClient.h"

#include "Crypto/Wrapper/RSADecrypt.h"

#include "Cheat.h"
#include "Game.h"
#include "User.h"

#include "UI/UI.h"

enum class LoaderStates : int
{
	Idle = 0,
	LoggingIn = 1,
	LoggedIn = 2,
	LoadingDLL = 3
};

inline bool operator<(LoaderStates a, LoaderStates b)
{
	return static_cast<int>(a) < static_cast<int>(b);
}

class Globals
{
public:
	static inline LoaderStates LoaderState = LoaderStates::Idle;
	
	static inline RSADecrypt RSA = RSADecrypt();
	
	static inline TcpClient TCPClient = TcpClient();
	static inline MatchedClient* MatchedClient = nullptr;

	static inline User CurrentUser;

	static inline std::vector<Game*> Games = {};
	static inline std::vector<Cheat*> Cheats = {};

	static inline Game* CurrentGame = nullptr;
	static inline Cheat* CurrentCheat = nullptr;

	static void ShutdownAndExit(int exitCode = 0)
	{
		TCPClient.finish();
		UI::Shutdown();

		ExitProcess(exitCode);
	}
};
