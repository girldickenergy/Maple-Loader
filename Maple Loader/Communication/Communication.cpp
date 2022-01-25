#include "Communication.h"

#include <ThemidaSDK.h>

#include "Packets/Requests/HandshakeRequest.h"
#include "Packets/Requests/FinalDllStreamRequest.h"

#include "Packets/Responses/FatalErrorResponse.h"
#include "Packets/Responses/HandshakeResponse.h"
#include "Packets/Responses/LoginResponse.h"
#include "Packets/Responses/InitialDllStreamResponse.h"

#include "../UI/UI.h"
#include "../Utilities/Security/xorstr.hpp"
#include "../Utilities/GeneralHelper.h"
#include "../UI/StyleProvider.h"
#include "../UI/Textures.h"
#include "../Utilities/Strings/StringUtilities.h"
#include "../Utilities/Crypto/CryptoHelper.h"
#include "../Injection/ProcessHollowing.h"
#include "../Injection/Data.h"
#include "../Injection/DataWriter.h"
#include "../Utilities/Autofill/AutofillHelper.h"

#pragma optimize("", off)
void Communication::onIncomingMessage(const char* msg, size_t size)
{
	auto* const response = static_cast<Response*>(Response::ConstructResponse(msg, size, MatchedClient));
	switch (response->Type)
	{
		case ResponseType::FatalError:
		{
			auto* const fatalErrorResponse = static_cast<FatalErrorResponse*>(response);

			std::string str;
			for (const auto& c : "Fatal error occurred: " + fatalErrorResponse->ErrorMessage + "\nThe application will now exit.")
				if (c != '\0') //fuck fuck fuck null terminators
					str.push_back(c);

			MessageBoxA(UI::Window, xor (str.c_str()), xor ("Discord"), MB_ICONERROR | MB_OK);

			GeneralHelper::ShutdownAndExit();

			break;
		}
		case ResponseType::Handshake:
		{
			auto* const handshakeResponse = static_cast<HandshakeResponse*>(response);

			switch (handshakeResponse->Result)
			{
				case HandshakeResult::Success:
				{
					VM_SHARK_BLACK_START

					MatchedClient = new ::MatchedClient(TCPClient);
					MatchedClient->aes->SetIV(handshakeResponse->IV);
					MatchedClient->aes->SetKey(handshakeResponse->Key);

					AutofillHelper::Fill();

					VM_SHARK_BLACK_END

					break;
				}
				case HandshakeResult::EpochTimedOut:
				case HandshakeResult::InternalError:
				{
					VM_SHARK_BLACK_START

					// Have both in a switch case, let's not tell anybody trying to crack that the epoch is wrong.
					MessageBoxA(UI::Window, xor ("Fatal error occured: your time is out of sync!\nThe application will now exit."), xor ("Discord"), MB_ICONERROR | MB_OK);

					VM_SHARK_BLACK_END

					GeneralHelper::ShutdownAndExit();

					break;
				}
			}

			break;
		}
		case ResponseType::Login:
		{
			auto* const loginResponse = static_cast<LoginResponse*>(response);

			CurrentState = States::Idle;

			switch (loginResponse->Result)
			{
				case LoginResult::Success:
				{
					VM_SHARK_BLACK_START

					CurrentUser->SessionID = loginResponse->SessionToken;
					CurrentUser->DiscordID = loginResponse->DiscordID;
					CurrentUser->AvatarHash = loginResponse->AvatarHash;

					CurrentUser->Games = loginResponse->Games;
					CurrentUser->Cheats = loginResponse->Cheats;

					CurrentUser->CurrentGame = CurrentUser->Games[0];
					CurrentUser->CurrentCheat = CurrentUser->Cheats[0];

					if (Communication::CurrentUser->DiscordID != "-1" && Communication::CurrentUser->AvatarHash != "-1")
					{
						std::string avatarURL = "https://cdn.discordapp.com/avatars/" + Communication::CurrentUser->DiscordID + "/" + Communication::CurrentUser->AvatarHash + ".png?size=64";

						StyleProvider::AvatarTexture = TextureHelper::CreateTextureFromURL(avatarURL);
					}
					else StyleProvider::AvatarTexture = TextureHelper::CreateTexture(Textures::DefaultAvatar, Textures::DefaultAvatarSize);

					if (!StyleProvider::AvatarTexture)
						StyleProvider::AvatarTexture = TextureHelper::CreateTexture(Textures::DefaultAvatar, Textures::DefaultAvatarSize);

					CurrentState = States::LoggedIn;

					AutofillHelper::Remember();

					CurrentUser->ResetSensitiveFields();

					VM_SHARK_BLACK_END

					break;
				}
				case LoginResult::IncorrectCredentials:
				{
					MessageBoxA(UI::Window, xor ("Failed to login!\nPlease make sure that you've entered your username and password correctly and try again."), xor ("Discord"), MB_ICONERROR | MB_OK);

					break;
				}
				case LoginResult::HashMismatch:
				{
					MessageBoxA(UI::Window, xor ("A newer version of the loader is available.\nPlease download it from https://maple.software/dashboard"), xor ("Discord"), MB_ICONERROR | MB_OK);

					break;
				}
				case LoginResult::HWIDMismatch:
				{
					MessageBoxA(UI::Window, xor ("HWID Mismatch!"), xor ("Discord"), MB_ICONERROR | MB_OK);

					break;
				}
				case LoginResult::Banned:
				{
					MessageBoxA(UI::Window, xor ("You're banned!"), xor ("Discord"), MB_ICONERROR | MB_OK);

					break;
				}
				case LoginResult::InternalError:
				{
					MessageBoxA(UI::Window, xor ("An internal error occured!\nPlease contact staff."), xor ("Discord"), MB_ICONERROR | MB_OK);

					break;
				}
				default:
				{
					MessageBoxA(UI::Window, xor ("An internal error occured: unknown login result."), xor ("Discord"), MB_ICONERROR | MB_OK);

					break;
				}
			}

			break;
		}
		case ResponseType::InitialDllStream:
		{
			auto* const dllStreamResponse = static_cast<InitialDllStreamResponse*>(response);
			switch (dllStreamResponse->Result)
			{
			case InitialDllStreamResult::Success:
			{
				//todo: allocate shit
				//todo: resolve imports
				//todo: do injection magic
				//TODO: MOVE THIS TO INJECTOR

				std::vector<unsigned int> imports;
				for (int i = 0; i < dllStreamResponse->Imports.size(); i++)
					imports.push_back(0xdeadbeef);

				FinalDllStreamRequest dllStream = FinalDllStreamRequest(0x1337, imports, Communication::MatchedClient);

				pipe_ret_t sendRet = Communication::TCPClient.sendBytes(dllStream.Data);
				if (!sendRet.success)
				{
					MessageBoxA(UI::Window, xor ("Failed to communicate with the server!\nThe application will now exit."), xor ("Discord"), MB_ICONERROR | MB_OK);

					GeneralHelper::ShutdownAndExit();
				}

				break;
			}
			case InitialDllStreamResult::NotSubscribed:
				MessageBoxA(UI::Window, xor ("Sorry, you're not subscribed to the Maple membership."), xor ("Discord"), MB_ICONERROR | MB_OK);
				CurrentState = States::LoggedIn;

				break;
			case InitialDllStreamResult::InvalidSession:
				MessageBoxA(UI::Window, xor ("Your session has expired.\n\nPlease log in again."), xor ("Discord"), MB_ICONERROR | MB_OK);
				CurrentState = States::Idle;

				AutofillHelper::Fill();

				break;
			case InitialDllStreamResult::InternalError:
				MessageBoxA(UI::Window, xor ("An internal error occured!\nPlease contact staff."), xor ("Discord"), MB_ICONERROR | MB_OK);
				CurrentState = States::LoggedIn;

				break;
			default:
				MessageBoxA(UI::Window, xor ("An internal error occured: unknown dllstream result."), xor ("Discord"), MB_ICONERROR | MB_OK);
				CurrentState = States::LoggedIn;

				break;
			}

			break;
		}
	}

	delete response;
}

void Communication::onDisconnection(const pipe_ret_t& ret)
{
	VM_SHARK_BLACK_START

	MessageBoxA(UI::Window, xor ("You have been disconnected from the server!\nThe application will now exit."), xor ("Discord"), MB_ICONERROR | MB_OK);
	GeneralHelper::ShutdownAndExit(false);

	VM_SHARK_BLACK_END
}

bool Communication::ConnectToServer()
{
	VM_FISH_RED_START
	STR_ENCRYPT_START

	client_observer_t observer;
	observer.wantedIp = xor("127.0.0.1");
	observer.incoming_packet_func = onIncomingMessage;
	observer.disconnected_func = onDisconnection;
	TCPClient.subscribe(observer);

	pipe_ret_t connectRet = TCPClient.connectTo(xor("127.0.0.1"), 9999);
	if (connectRet.success)
	{
		// Send initial Handshake, to get RSA Encrypted Client Key and IV
		HandshakeRequest handshakePacket = HandshakeRequest();

		if (const pipe_ret_t sendRet = TCPClient.sendBytes(handshakePacket.Data); !sendRet.success)
		{
			MessageBoxA(UI::Window, xor ("Failed to communicate with the server!\nThe application will now exit."), xor ("Discord"), MB_ICONERROR | MB_OK);

			GeneralHelper::ShutdownAndExit();

			return false;
		}
	}
	else
		return false;

	return true;

	STR_ENCRYPT_END
	VM_FISH_RED_END
}
#pragma optimize("", on)
