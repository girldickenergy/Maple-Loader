#include "Communication.h"

#include <random>

#include "ThemidaSDK.h"

#include "../Utilities/Autofill/AutofillUtilities.h"
#include "../Utilities/Hardware/HardwareUtilities.h"
#include "../Utilities/Security/xorstr.hpp"
#include "Packets/Requests/HandshakeRequest.h"
#include "Packets/Responses/HandshakeResponse.h"
#include "Crypto/CryptoProvider.h"
#include "Packets/Requests/LoginRequest.h"
#include "Packets/Responses/LoginResponse.h"
#include "../UI/UI.h"
#include "Packets/Responses/LoaderStreamResponse.h"
#include "Packets/Requests/LoaderStreamRequest.h"
#include "../Injection/ProcessHollowing/ProcessHollowing.h"
#include "../Injection/DataWriter.h"

#pragma optimize("", off)
void Communication::OnReceive(const std::vector<unsigned char>& data)
{
	VM_SHARK_BLACK_START

	int codeIntegrityVar = 0x671863E2;
	CHECK_CODE_INTEGRITY(codeIntegrityVar, 0x40CD69D0)
	if (codeIntegrityVar != 0x40CD69D0)
	{
		IntegritySignature1 -= 0x1;
		IntegritySignature2 -= 0x1;
		IntegritySignature3 -= 0x1;
	}

	int debuggerVar = 0xD0A7E6;
	CHECK_DEBUGGER(debuggerVar, 0x3E839EE3)
	if (debuggerVar != 0x3E839EE3)
	{
		IntegritySignature1 -= 0x1;
		IntegritySignature2 -= 0x1;
		IntegritySignature3 -= 0x1;
	}

	std::vector<uint8_t> decryptedData;

	if (!m_HandshakeComplete)
	{
		auto key1 = *reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(data.data()));
		auto key2 = *reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(data.data()) + sizeof(uint32_t));
		auto key3 = *reinterpret_cast<uint32_t*>(const_cast<uint8_t*>(data.data()) + sizeof(uint32_t) * 2);

		decryptedData = CryptoProvider::Get().ApplyCryptoTransformations(std::vector(data.begin() + sizeof(uint32_t) * 3, data.end()), key1, key2, key3, true);
	}
	else
		decryptedData = CryptoProvider::Get().AESDecrypt(data);

	const auto& packet = PacketSerializer::Get().Deserialize(decryptedData);

	if (!packet.has_value())
	{
		Disconnect();

		return;
	}

	if (!m_PacketHandlers.contains(packet->second))
	{
		Disconnect();

		return;
	}

	m_PacketHandlers[packet.value().second](packet->first);

	VM_SHARK_BLACK_END
}

void Communication::OnDisconnect()
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START
		
	MessageBoxA(UI::NativeWindow, xorstr_("You have been disconnected from the server.\nThe application will now exit."), xorstr_("Fatal error"), MB_ICONERROR | MB_OK);

	glfwSetWindowShouldClose(UI::GLFWWindow, 1);

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END
}

void Communication::OnHandshake(const entt::meta_any& packet)
{
	VM_SHARK_BLACK_START

	auto handshakeResponse = packet.cast<HandshakeResponse>();

	auto decryptionArray = CryptoProvider::Get().ComputeHashMha256(handshakeResponse.GetKey());
	auto decryptionKey = std::vector(decryptionArray.begin(), decryptionArray.end());

	auto decryptedKey = CryptoProvider::Get().ApplyRollingXor(handshakeResponse.GetEncryptedKey(), decryptionKey);
	auto decryptedIv = CryptoProvider::Get().ApplyRollingXor(handshakeResponse.GetEncryptedIV(), decryptionKey);

	CryptoProvider::Get().InitializeAES(decryptedKey, decryptedIv);

	const std::pair<std::string, std::string> credentials = AutofillUtilities::GetCredentials();

	memcpy(LoginUsername, credentials.first.c_str(), credentials.first.length());
	memcpy(LoginPassword, credentials.second.c_str(), credentials.second.length());

	m_HandshakeComplete = true;

	VM_SHARK_BLACK_END
}

void Communication::OnLogin(const entt::meta_any& packet)
{
	auto loginResponse = packet.cast<LoginResponse>();

	switch (loginResponse.GetResult())
	{
		case RequestResult::Success:
		{
			VM_SHARK_BLACK_START

			delete m_User;

			m_Cheats.clear();
			m_Games.clear();

			m_User = new User(LoginUsername, loginResponse.GetSessionToken(), loginResponse.GetDiscordID(), loginResponse.GetDiscordAvatarHash());
			m_Games = loginResponse.GetGames();
			m_Cheats = loginResponse.GetCheats();

			m_SelectedGame = &m_Games[0];
			for (auto& cheat : m_Cheats)
			{
				if (cheat.GetGameID() == m_SelectedGame->GetID())
				{
					m_SelectedCheat = &cheat;

					break;
				}
			}

			m_State = States::LoggedIn;

			AutofillUtilities::RememberCredentials(LoginUsername, LoginPassword);

			memset(LoginPassword, 0, sizeof(LoginPassword));
			memset(LoginUsername, 0, sizeof(LoginUsername));
				
			VM_SHARK_BLACK_END
		}
			break;
		case RequestResult::InvalidCredentials:
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			m_State = States::Idle;
			MessageBoxA(UI::NativeWindow, xorstr_("Please make sure that you've entered your username and password correctly and try again."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);
			
			STR_ENCRYPT_END
			VM_SHARK_BLACK_END

			break;
		case RequestResult::VersionMismatch:
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			m_State = States::Idle;
			MessageBoxA(UI::NativeWindow, xorstr_("A newer version of the loader is available.\nPlease re-download it from your dashboard."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END

			break;
		case RequestResult::HWIDMismatch:
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			m_State = States::Idle;
			MessageBoxA(UI::NativeWindow, xorstr_("Hardware changed too fast."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END

			break;
		case RequestResult::UserBanned:
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			m_State = States::Idle;
			MessageBoxA(UI::NativeWindow, xorstr_("You're banned.\nFor more information please visit your dashboard."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END

			break;
		default:
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			m_State = States::Idle;
			MessageBoxA(UI::NativeWindow, xorstr_("Unknown error occurred. Please contact staff."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END

			break;
	}
}

void Communication::OnLoaderStream(const entt::meta_any& packet)
{
	auto loaderStreamResponse = packet.cast<LoaderStreamResponse>();

	switch (loaderStreamResponse.GetResult())
	{
		case RequestResult::Success:
		{
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			bool integrityViolated = IntegritySignature1 != 0xdeadbeef || IntegritySignature2 != 0xefbeadde || IntegritySignature3 != 0xbeefdead;

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> behaviorRNG(1, 3);

			// 1 - swap 32 and 64 bit svchosts, 2 - skip injection process and fail, 3 - corrupt loader image
			const int behavior = integrityViolated ? behaviorRNG(gen) : 0;

			char windowsDirectory[MAX_PATH];
			GetWindowsDirectoryA(windowsDirectory, sizeof(windowsDirectory));
			std::string svchostFilePath32 = std::string(windowsDirectory) + (behavior == 1 ? xorstr_("\\System32\\svchost.exe") : xorstr_("\\SysWOW64\\svchost.exe"));
			std::string svchostFilePath64 = std::string(windowsDirectory) + (behavior == 1 ? xorstr_("\\System32\\svchost.exe") : xorstr_("\\System32\\svchost.exe"));
			
			if (behavior != 2)
			{
				std::vector<unsigned char> loaderData = loaderStreamResponse.GetLoaderData();
				if (behavior == 3)
				{
					std::uniform_int_distribution<> beginOffsetRNG(0, loaderData.size() - 1);
					int beginOffset = beginOffsetRNG(gen);
					std::uniform_int_distribution<> endOffsetRNG(beginOffset, loaderData.size() - 1);

					loaderData.erase(loaderData.begin() + beginOffset, loaderData.end() - endOffsetRNG(gen));
				}
				
				if (const DWORD pid = ProcessHollowing::RunPE(loaderData.data(), std::wstring(svchostFilePath32.begin(), svchostFilePath32.end()).c_str(), std::wstring(svchostFilePath64.begin(), svchostFilePath64.end()).c_str()))
				{
					while (true)
					{
						HANDLE mtx = OpenMutexA(SYNCHRONIZE, FALSE, xorstr_("QVPj0LSOL81Lko4d"));
						if (mtx != NULL)
						{
							CloseHandle(mtx);

							break;
						}

						Sleep(100);
					}

					m_TcpClient.Disconnect();

					MessageBoxA(UI::NativeWindow, xorstr_("Injection process has started. Please launch the game and wait for injection to finish.\nOnce Maple is injected you can toggle the in-game menu with DELETE button.\n\nThank you for choosing Maple and have fun!"), xorstr_("Success"), MB_ICONINFORMATION | MB_OK);

					DataWriter::GetInstance()->Finish();

					glfwSetWindowShouldClose(UI::GLFWWindow, 1);

					break;
				}
			}
			
			MessageBoxA(UI::NativeWindow, xorstr_("Injection failed.\nPlease try again later."), xorstr_("Failed to load"), MB_ICONERROR | MB_OK);
			m_State = States::LoggedIn;

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END
		}
			break;
		case RequestResult::InvalidSession:
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START
		MessageBoxA(UI::NativeWindow, xorstr_("Your session has expired.\nPlease log in again."), xorstr_("Failed to load"), MB_ICONERROR | MB_OK);
			m_State = States::Idle;

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END

			break;
		case RequestResult::NotSubscribed:
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			MessageBoxA(UI::NativeWindow, xorstr_("Sorry, but you have to be subscribed to use this software."), xorstr_("Failed to load"), MB_ICONERROR | MB_OK);
			m_State = States::LoggedIn;
			STR_ENCRYPT_END
			VM_SHARK_BLACK_END

			break;
		default:
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			MessageBoxA(UI::NativeWindow, xorstr_("Unknown error occurred. Please contact staff."), xorstr_("Failed to load"), MB_ICONERROR | MB_OK);
			m_State = States::LoggedIn;

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END

			break;
	}
}

Communication::Communication(singletonLock)
{
	m_PacketHandlers =
	{
		{ entt::resolve<HandshakeResponse>().id(), [this](const entt::meta_any& packet) { OnHandshake(packet); } },
		{ entt::resolve<LoginResponse>().id(), [this](const entt::meta_any& packet) { OnLogin(packet); } },
		{ entt::resolve<LoaderStreamResponse>().id(), [this](const entt::meta_any& packet) { OnLoaderStream(packet); } }
	};
}

bool Communication::Connect()
{
	VM_FISH_RED_START
	STR_ENCRYPT_START

	auto receiveBinding = [this]<typename T>(T && data) { OnReceive(std::forward<T>(data)); };
	auto disconnectBinding = [this] { OnDisconnect(); };

	m_TcpClient = TCPClient(receiveBinding, disconnectBinding);

	if (!m_TcpClient.Connect(xorstr_("127.0.0.1"), xorstr_("9999")))
		return false;

	std::mt19937 random(std::random_device{}());
	std::uniform_int_distribution<> byteDistribution(0x03, 0xFF);
	std::uniform_int_distribution<> intDistribution(INT_MIN, INT_MAX);

	// Random amount of bytes, but at least 0x03
	auto length = byteDistribution(random);
	auto data = std::vector<uint32_t>(length);

	for (size_t i = 0; i < length; i++)
		data[i] = intDistribution(random);

	data[1] = data[0] ^ data[2] ^ 0xdeadbeef;

	auto handshakeRequest = HandshakeRequest(data);
	auto handshakeRequestSerialized = PacketSerializer::Get().Serialize(handshakeRequest);

	if (!handshakeRequestSerialized.has_value())
		return false;

	uint32_t key1 = intDistribution(random);
	uint32_t key2 = intDistribution(random);
	uint32_t key3 = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() >> 10) ^ 0xdeadbeef;

	std::vector<uint8_t> encryptedHandshake = CryptoProvider::Get().ApplyCryptoTransformations(handshakeRequestSerialized.value(), key1, key2, key3);

	encryptedHandshake.insert(encryptedHandshake.begin(), reinterpret_cast<uint8_t*>(&key3), reinterpret_cast<uint8_t*>(&key3) + sizeof(uint32_t));
	encryptedHandshake.insert(encryptedHandshake.begin(), reinterpret_cast<uint8_t*>(&key2), reinterpret_cast<uint8_t*>(&key2) + sizeof(uint32_t));
	encryptedHandshake.insert(encryptedHandshake.begin(), reinterpret_cast<uint8_t*>(&key1), reinterpret_cast<uint8_t*>(&key1) + sizeof(uint32_t));

	m_TcpClient.Send(encryptedHandshake);

	STR_ENCRYPT_END
	VM_FISH_RED_END

	return true;
}

void Communication::Disconnect()
{
	VM_SHARK_BLACK_START

	m_TcpClient.Disconnect();

	VM_SHARK_BLACK_END
}

void Communication::LogIn()
{
	VM_SHARK_BLACK_START

	m_State = States::LoggingIn;

	auto loginRequest = LoginRequest(LoginUsername, LoginPassword, xorstr_("l-03122023"), HardwareUtilities::GetHWID());
	auto loginRequestSerialized = PacketSerializer::Get().Serialize(loginRequest);

	if (!loginRequestSerialized.has_value())
	{
		Disconnect();

		return;
	}

	m_TcpClient.Send(CryptoProvider::Get().AESEncrypt(loginRequestSerialized.value()));

	VM_SHARK_BLACK_END
}

void Communication::RequestLoader()
{
	VM_SHARK_BLACK_START

	m_State = States::LoadingPayload;

	auto loaderStreamRequest = LoaderStreamRequest(m_User->GetSessionToken(), m_SelectedCheat->GetID(), m_SelectedCheat->GetReleaseStreams()[m_SelectedCheat->CurrentStream]);
	auto loaderStreamRequestSerialized = PacketSerializer::Get().Serialize(loaderStreamRequest);

	if (!loaderStreamRequestSerialized.has_value())
	{
		Disconnect();

		return;
	}

	m_TcpClient.Send(CryptoProvider::Get().AESEncrypt(loaderStreamRequestSerialized.value()));

	VM_SHARK_BLACK_END
}

States Communication::GetState()
{
	return m_State;
}

User* Communication::GetUser()
{
	return m_User;
}

std::vector<Game>* Communication::GetAllGames()
{
	return &m_Games;
}

void Communication::SelectGame(unsigned int gameID)
{
	for (auto& game : m_Games)
	{
		if (game.GetID() == gameID)
		{
			m_SelectedGame = &game;

			break;
		}
	}
}

Game* Communication::GetSelectedGame()
{
	return m_SelectedGame;
}

std::vector<Cheat>* Communication::GetAllCheats()
{
	return &m_Cheats;
}

void Communication::SelectCheat(unsigned int cheatID)
{
	for (auto& cheat : m_Cheats)
	{
		if (cheat.GetID() == cheatID)
		{
			m_SelectedCheat = &cheat;

			break;
		}
	}
}

Cheat* Communication::GetSelectedCheat()
{
	return m_SelectedCheat;
}
#pragma optimize("", on)
