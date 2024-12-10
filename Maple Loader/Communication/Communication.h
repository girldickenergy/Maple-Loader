#pragma once

#include <vector>

#include "Singleton.h"

#include "States.h"
#include "User.h"
#include "Cheat.h"
#include "Game.h"
#include "Packets/PacketSerializer.h"
#include "TCP/TCPClient.h"

class Communication : public Singleton<Communication>
{
	States m_State = States::Idle;

	User* m_User = nullptr;
	std::vector<Game> m_Games;
	Game* m_SelectedGame = nullptr;
	std::vector<Cheat> m_Cheats;
	Cheat* m_SelectedCheat;
	bool m_HandshakeComplete = false;

	TCPClient m_TcpClient;

	std::unordered_map<uint32_t, std::function<void(entt::meta_any)>> m_PacketHandlers;

	void OnReceive(const std::vector<unsigned char>& data);
	void OnDisconnect();
	void OnHandshake(const entt::meta_any& packet);
	void OnLogin(const entt::meta_any& packet);
	void OnLoaderStream(const entt::meta_any& packet);
public:
	static inline unsigned int IntegritySignature1 = 0xdeadbeef;
	static inline unsigned int IntegritySignature2 = 0xefbeadde;
	static inline unsigned int IntegritySignature3 = 0xbeefdead;

	static inline char LoginUsername[24];
	static inline char LoginPassword[256];

	Communication(singletonLock);

	bool Connect();
	void Disconnect();
	void LogIn();
	void RequestLoader();

	States GetState();
	User* GetUser();
	std::vector<Game>* GetAllGames();
	void SelectGame(unsigned int gameID);
	Game* GetSelectedGame();
	std::vector<Cheat>* GetAllCheats();
	void SelectCheat(unsigned int cheatID);
	Cheat* GetSelectedCheat();
};
