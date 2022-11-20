#include "Communication.h"

#include <random>

#include "ThemidaSDK.h"

#include "../Utilities/Autofill/AutofillUtilities.h"
#include "../Utilities/Hardware/HardwareUtilities.h"
#include "../Utilities/Security/xorstr.hpp"
#include "Packets/PacketType.h"
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
void Communication::onReceive(const std::vector<unsigned char>& data)
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

	const auto type = static_cast<PacketType>(data[0]);
	const std::vector payload(data.begin() + 1, data.end());

	VM_SHARK_BLACK_END

	switch (type)
	{
		case PacketType::Handshake:
		{
			VM_SHARK_BLACK_START
			STR_ENCRYPT_START

			HandshakeResponse handshakeResponse = HandshakeResponse::Deserialize(payload);

			CryptoProvider::GetInstance()->InitializeAES(handshakeResponse.GetKey(), handshakeResponse.GetIV());

			const std::pair<std::string, std::string> credentials = AutofillUtilities::GetCredentials();

			memcpy(LoginUsername, credentials.first.c_str(), credentials.first.length());
			memcpy(LoginPassword, credentials.second.c_str(), credentials.second.length());

			STR_ENCRYPT_END
			VM_SHARK_BLACK_END
		}
			break;

		case PacketType::Login:
		{
			LoginResponse loginResponse = LoginResponse::Deserialize(payload);

			switch (loginResponse.GetResult())
			{
				case LoginResult::Success:
				{
					VM_SHARK_BLACK_START

					delete user;

					for (Cheat* cheat : cheats)
						delete cheat;
					cheats.clear();

					for (Game* game : games)
						delete game;
					games.clear();

					user = new User(LoginUsername, loginResponse.GetSessionToken(), loginResponse.GetDiscordID(), loginResponse.GetDiscordAvatarHash());
					games = loginResponse.GetGames();
					cheats = loginResponse.GetCheats();

					selectedGame = games[0];
					for (Cheat* cheat : cheats)
					{
						if (cheat->GetGameID() == selectedGame->GetID())
						{
							selectedCheat = cheat;

							break;
						}
					}

					state = States::LoggedIn;

					AutofillUtilities::RememberCredentials(LoginUsername, LoginPassword);

					memset(LoginPassword, 0, sizeof(LoginPassword));
					memset(LoginUsername, 0, sizeof(LoginUsername));
						
					VM_SHARK_BLACK_END
				}
					break;
				case LoginResult::IncorrectCredentials:
					VM_SHARK_BLACK_START
					STR_ENCRYPT_START

					state = States::Idle;
					MessageBoxA(UI::NativeWindow, xorstr_("Please make sure that you've entered your username and password correctly and try again."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);
					
					STR_ENCRYPT_END
					VM_SHARK_BLACK_END

					break;
				case LoginResult::VersionMismatch:
					VM_SHARK_BLACK_START
					STR_ENCRYPT_START

					state = States::Idle;
					MessageBoxA(UI::NativeWindow, xorstr_("A newer version of the loader is available.\nPlease re-download it from your dashboard."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);

					STR_ENCRYPT_END
					VM_SHARK_BLACK_END

					break;
				case LoginResult::HWIDMismatch:
					VM_SHARK_BLACK_START
					STR_ENCRYPT_START

					state = States::Idle;
					MessageBoxA(UI::NativeWindow, xorstr_("Hardware changed too fast."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);

					STR_ENCRYPT_END
					VM_SHARK_BLACK_END

					break;
				case LoginResult::Banned:
					VM_SHARK_BLACK_START
					STR_ENCRYPT_START

					state = States::Idle;
					MessageBoxA(UI::NativeWindow, xorstr_("You're banned.\nFor more information please visit your dashboard."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);

					STR_ENCRYPT_END
					VM_SHARK_BLACK_END

					break;
				default:
					VM_SHARK_BLACK_START
					STR_ENCRYPT_START

					state = States::Idle;
					MessageBoxA(UI::NativeWindow, xorstr_("Unknown error occurred. Please contact staff."), xorstr_("Failed to log in"), MB_ICONERROR | MB_OK);

					STR_ENCRYPT_END
					VM_SHARK_BLACK_END

					break;
			}
		}
			break;
		case PacketType::LoaderStream:
		{
			LoaderStreamResponse loaderStreamResponse = LoaderStreamResponse::Deserialize(payload);

			switch (loaderStreamResponse.GetResult())
			{
				case LoaderStreamResult::Success:
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

							if (DataWriter::GetInstance()->Initialize(pid))
							{
								if (DataWriter::GetInstance()->WriteUserData(user->GetUsername(), user->GetSessionToken(), user->GetDiscordID(), user->GetAvatarHash(), selectedCheat->GetID(), selectedCheat->GetReleaseStreams()[selectedCheat->CurrentStream]))
								{
									tcpClient.Disconnect();

									MessageBoxA(UI::NativeWindow, xorstr_("Injection process has started. Please launch the game and wait for injection to finish.\nOnce Maple is injected you can toggle the in-game menu with DELETE button.\n\nThank you for choosing Maple and have fun!"), xorstr_("Success"), MB_ICONINFORMATION | MB_OK);

									DataWriter::GetInstance()->Finish();

									glfwSetWindowShouldClose(UI::GLFWWindow, 1);

									break;
								}
							}
						}
					}
					
					MessageBoxA(UI::NativeWindow, xorstr_("Injection failed.\nPlease try again later."), xorstr_("Failed to load"), MB_ICONERROR | MB_OK);
					state = States::LoggedIn;

					STR_ENCRYPT_END
					VM_SHARK_BLACK_END
				}
					break;
				case LoaderStreamResult::InvalidSession:
					VM_SHARK_BLACK_START
					STR_ENCRYPT_START

					MessageBoxA(UI::NativeWindow, xorstr_("Your session has expired.\nPlease log in again."), xorstr_("Failed to load"), MB_ICONERROR | MB_OK);
					state = States::Idle;

					STR_ENCRYPT_END
					VM_SHARK_BLACK_END

					break;
				case LoaderStreamResult::NotSubscribed:
					VM_SHARK_BLACK_START
					STR_ENCRYPT_START

					MessageBoxA(UI::NativeWindow, xorstr_("Sorry, but you have to be subscribed to use this software."), xorstr_("Failed to load"), MB_ICONERROR | MB_OK);
					state = States::LoggedIn;
					STR_ENCRYPT_END
					VM_SHARK_BLACK_END

					break;
				default:
					VM_SHARK_BLACK_START
					STR_ENCRYPT_START

					MessageBoxA(UI::NativeWindow, xorstr_("Unknown error occurred. Please contact staff."), xorstr_("Failed to load"), MB_ICONERROR | MB_OK);
					state = States::LoggedIn;

					STR_ENCRYPT_END
					VM_SHARK_BLACK_END

					break;
			}
		}
			break;
	}
}

void Communication::onDisconnect()
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	MessageBoxA(UI::NativeWindow, xorstr_("You have been disconnected from the server.\nThe application will now exit."), xorstr_("Fatal error"), MB_ICONERROR | MB_OK);

	glfwSetWindowShouldClose(UI::GLFWWindow, 1);

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END
}

bool Communication::Connect()
{
	VM_FISH_RED_START
	STR_ENCRYPT_START

	tcpClient = TCPClient(&onReceive, &onDisconnect);
	if (!tcpClient.Connect(xorstr_("127.0.0.1"), xorstr_("9999")))
		return false;

	HandshakeRequest handshakeRequest = HandshakeRequest();
	tcpClient.Send(handshakeRequest.Serialize());

	STR_ENCRYPT_END
	VM_FISH_RED_END

	return true;
}

void Communication::Disconnect()
{
	VM_SHARK_BLACK_START

	tcpClient.Disconnect();

	VM_SHARK_BLACK_END
}

void Communication::LogIn()
{
	state = States::LoggingIn;

	LoginRequest loginRequest = LoginRequest(LoginUsername, LoginPassword, "l-20112022", HardwareUtilities::GetHWID());

	tcpClient.Send(loginRequest.Serialize());
}

void Communication::RequestLoader()
{
	VM_SHARK_BLACK_START

	state = States::LoadingPayload;

	LoaderStreamRequest r = LoaderStreamRequest(user->GetSessionToken(), selectedCheat->GetID());
	tcpClient.Send(r.Serialize());

	VM_SHARK_BLACK_END
}

States Communication::GetState()
{
	return state;
}

User* Communication::GetUser()
{
	return user;
}

const std::vector<Game*>& Communication::GetAllGames()
{
	return games;
}

void Communication::SelectGame(unsigned int gameID)
{
	for (Game* game : games)
	{
		if (game->GetID() == gameID)
		{
			selectedGame = game;

			break;
		}
	}
}

Game* Communication::GetSelectedGame()
{
	return selectedGame;
}

const std::vector<Cheat*>& Communication::GetAllCheats()
{
	return cheats;
}

void Communication::SelectCheat(unsigned int cheatID)
{
	for (Cheat* cheat : cheats)
	{
		if (cheat->GetID() == cheatID)
		{
			selectedCheat = cheat;

			break;
		}
	}
}

Cheat* Communication::GetSelectedCheat()
{
	return selectedCheat;
}
#pragma optimize("", on)
