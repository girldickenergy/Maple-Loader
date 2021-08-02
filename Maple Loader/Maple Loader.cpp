#include "Globals.h"

#include "UI/UI.h"

#include "AntiDebug/xorstr.hpp"

#include "Packets/Requests/HandshakeRequest.h"
#include "Packets/Responses/DllStreamResponse.h"

#include "Packets/Responses/FatalErrorResponse.h"
#include "Packets/Responses/HandshakeResponse.h"
#include "Packets/Responses/LoginResponse.h"
#include "Packets/Responses/Response.h"

void OnIncomingMessage(const char* msg, size_t size)
{
	auto* const response = static_cast<Response*>(Response::ConstructResponse(msg, size, Globals::MatchedClient));
	switch (response->Type)
	{
		case ResponseType::FatalError:
		{
			auto* const fatalErrorResponse = static_cast<FatalErrorResponse*>(response);

			std::string str;
			for (const auto& c : "Fatal error occurred: " + fatalErrorResponse->ErrorMessage + "\nThe application will now exit.")
				if (c != '\0') //fuck fuck fuck null terminators
					str.push_back(c);

			MessageBoxA(UI::Window, xor (str.c_str()), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

			Globals::ShutdownAndExit();

			break;
		}
		case ResponseType::Handshake:
		{
			auto* const handshakeResponse = static_cast<HandshakeResponse*>(response);

			Globals::MatchedClient = new MatchedClient(Globals::TCPClient);
			Globals::MatchedClient->aes->SetIV(handshakeResponse->IV);
			Globals::MatchedClient->aes->SetKey(handshakeResponse->Key);
			
			break;
		}
		case ResponseType::Login:
		{
			auto* const loginResponse = static_cast<LoginResponse*>(response);
				
			Globals::LoaderState = LoaderStates::Idle;

			switch (loginResponse->Result)
			{
				case LoginResult::Success:
				{
					Globals::CurrentUser.Session = loginResponse->SessionToken;

					Globals::Games = loginResponse->Games;
					Globals::Cheats = loginResponse->Cheats;

					Globals::CurrentGame = Globals::Games[0];
					Globals::CurrentCheat = Globals::Cheats[0];

					Globals::CurrentUser.ResetSensitiveFields();
						
					Globals::LoaderState = LoaderStates::LoggedIn;

					break;
				}
				case LoginResult::IncorrectCredentials:
				{
					MessageBoxA(UI::Window, xor ("Failed to login!\nPlease make sure that you've entered your username and password correctly and try again."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
						
					break;
				}
				case LoginResult::HWIDMismatch:
				{
					MessageBoxA(UI::Window, xor ("HWID Mismatch!"), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
						
					break;
				}
				case LoginResult::Banned:
				{
					MessageBoxA(UI::Window, xor ("You're banned!"), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

					break;
				}
				case LoginResult::InternalError:
				{
					MessageBoxA(UI::Window, xor ("An internal error occured!\nPlease contact staff."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

					break;
				}
				default:
				{
					MessageBoxA(UI::Window, xor ("An internal error occured: unknown login result."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
						
					break;
				}
			}
				
			break;
		}
		case ResponseType::DllStream:
		{
			auto* const dllStreamResponse = static_cast<DllStreamResponse*>(response);
			switch (dllStreamResponse->Result)
			{
				case DllStreamResult::Success:
					MessageBoxA(UI::Window, xor ("Received DLL Stream"), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

					break;
				case DllStreamResult::NotSubscribed:
					MessageBoxA(UI::Window, xor ("Sorry, you're not subscribed to the Maple membership."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
					Globals::LoaderState = LoaderStates::LoggedIn;

					break;
				case DllStreamResult::InvalidSession:
					MessageBoxA(UI::Window, xor ("Your session has expired.\n\nPlease log in again."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
					Globals::LoaderState = LoaderStates::Idle;

					break;
				case DllStreamResult::InternalError:
					MessageBoxA(UI::Window, xor ("An internal error occured!\nPlease contact staff."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
					Globals::LoaderState = LoaderStates::LoggedIn;

					break;
				default:
					MessageBoxA(UI::Window, xor ("An internal error occured: unknown dllstream result."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
					Globals::LoaderState = LoaderStates::LoggedIn;
				
					break;
			}
			
			break;
		}
	}

	delete response;
}

void OnDisconnection(const pipe_ret_t& ret)
{
	MessageBoxA(UI::Window, xor ("You have been disconnected from the server!\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

	Globals::ShutdownAndExit();
}

bool ConnectToServer()
{
	client_observer_t observer;
	observer.wantedIp = "127.0.0.1";
	observer.incoming_packet_func = OnIncomingMessage;
	observer.disconnected_func = OnDisconnection;
	Globals::TCPClient.subscribe(observer);

	pipe_ret_t connectRet = Globals::TCPClient.connectTo("127.0.0.1", 9999);
	if (connectRet.success)
	{
		// Send initial Handshake, to get RSA Encrypted Client Key and IV
		HandshakeRequest handshakePacket = HandshakeRequest();

		if (const pipe_ret_t sendRet = Globals::TCPClient.sendBytes(handshakePacket.Data); !sendRet.success)
		{
			MessageBoxA(UI::Window, xor ("Failed to communicate with the server!"), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

			return false;
		}
		
		return true;
	}

	return false;
}

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPSTR cmd_args, int show_cmd)
{
	#ifdef _DEBUG
		AllocConsole();
		freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	#endif
	
	if (!ConnectToServer())
	{
		MessageBoxA(UI::Window, xor ("Failed to connect to server!\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

		return 0;
	}
	
	if (!UI::Initialize(inst, show_cmd))
	{
		MessageBoxA(UI::Window, xor ("Internal graphics error occurred!\nPlease make sure your graphics card drivers are up to date."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

		return 0;
	}

	// init curl
	curl_global_init(CURL_GLOBAL_ALL);
	
	MSG msg;
	memset(&msg, 0, sizeof(msg));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		if (!UI::Render())
		{
			MessageBoxA(UI::Window, xor ("Internal graphics error occurred!\nPlease make sure your graphics card drivers are up to date."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
			
			break;
		}
	}

	Globals::ShutdownAndExit();
}
