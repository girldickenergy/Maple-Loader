#pragma once

#include <vector>

#include "States.h"
#include "User.h"
#include "Cheat.h"
#include "Game.h"
#include "TCP/TCPClient.h"

class Communication
{
	static inline States state = States::Idle;

	static inline User* user = nullptr;
	static inline std::vector<Game*> games;
	static inline Game* selectedGame = nullptr;
	static inline std::vector<Cheat*> cheats;
	static inline Cheat* selectedCheat;

	static inline TCPClient tcpClient;

	static void onReceive(const std::vector<unsigned char>& data);
	static void onDisconnect();
public:
	static inline unsigned int IntegritySignature1 = 0xdeadbeef;
	static inline unsigned int IntegritySignature2 = 0xefbeadde;
	static inline unsigned int IntegritySignature3 = 0xbeefdead;

	static inline char LoginUsername[24];
	static inline char LoginPassword[256];

	static bool Connect();
	static void Disconnect();
	static void LogIn();
	static void RequestLoader();

	static States GetState();
	static User* GetUser();
	static const std::vector<Game*>& GetAllGames();
	static void SelectGame(unsigned int gameID);
	static Game* GetSelectedGame();
	static const std::vector<Cheat*>& GetAllCheats();
	static void SelectCheat(unsigned int cheatID);
	static Cheat* GetSelectedCheat();
};
