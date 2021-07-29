#define _WINSOCKAPI_
#include <windows.h>

#include "AntiDebug/xorstr.hpp"
#include "TCP/TCP_Client.h"

#include "UI/UI.h"
#include "UI/ImGui/imgui_impl_dx9.h"
#include "UI/ImGui/imgui_impl_win32.h"
#include "UI/StyleProvider.h"

#include "Utils/HWID.h"

#include "Packets/Requests/HandshakeRequest.h"
#include "Packets/Requests/LoginRequest.h"
#include "Packets/Requests/DllStreamRequest.h"

#include "Packets/Responses/FatalErrorResponse.h"
#include "Packets/Responses/HandshakeResponse.h"
#include "Packets/Responses/LoginResponse.h"
#include "Packets/Responses/Response.h"

TcpClient client;
MatchedClient* mClient = new MatchedClient(TcpClient());

enum LoaderState
{
	Idle,
	LoggingIn,
	LoggedIn,
	DownloadingDLL,
	Injecting
};

int State = Idle;

std::string sessionToken;
std::string expiresAt;

void ShutdownAndExit(int exitCode = 0)
{
	client.finish();
	UI::Shutdown();

	exit(exitCode);
}

static inline int dragOffsetX = 0;
static inline int dragOffsetY = 0;
void HandleWindowDrag()
{
	if (ImGui::IsMouseClicked(0))
	{
		POINT point;
		RECT rect;

		GetCursorPos(&point);
		GetWindowRect(UI::Window, &rect);

		dragOffsetX = point.x - rect.left;
		dragOffsetY = point.y - rect.top;
	}

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !ImGui::GetIO().WantTextInput)
	{
		POINT point;
		GetCursorPos(&point);

		SetWindowPos(UI::Window, nullptr, point.x - dragOffsetX, point.y - dragOffsetY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

char usernameBuf[24];
char passwordBuf[256];
int currItem = 0;
bool DrawMenu()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	HandleWindowDrag();

	ImGui::SetNextWindowSize(ImVec2{ 400, 250 }, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2{ 0, 0 }, ImGuiCond_Always);

	ImGui::Begin(xor ("Loader"), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
	{
		ImVec2 windowSize = ImGui::GetWindowSize();

		//title bar
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(windowSize.x, StyleProvider::TitleBarHeight), ImColor(StyleProvider::TitleBarColour));

		//window controls
		ImGui::PushFont(StyleProvider::FontDefault);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
		ImGui::PushStyleColor(ImGuiCol_Text, StyleProvider::TitleColour);
		ImGui::PushStyleColor(ImGuiCol_Button, StyleProvider::WindowControl);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, StyleProvider::WindowControlHovered);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, StyleProvider::WindowControlActive);
		ImGui::SetCursorPos(ImVec2(5, 5));

		if (ImGui::Button(xor ("x"), ImVec2(20, 20)))
			ShutdownAndExit();

		ImGui::SameLine();

		if (ImGui::Button(xor ("-"), ImVec2(20, 20)))
			ShowWindow(UI::Window, SW_MINIMIZE);

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);
		ImGui::PopFont();

		//title
		ImGui::PushFont(StyleProvider::FontTitle);

		auto titleSize = ImGui::CalcTextSize(xor ("Maple Loader"));
		ImGui::SetCursorPos(ImVec2(windowSize.x - titleSize.x - 5, (StyleProvider::TitleBarHeight / 2) - (titleSize.y / 2)));
		ImGui::Text(xor ("Maple Loader"));

		ImGui::PopStyleColor();
		ImGui::PopFont();

		ImGui::SetCursorPos(ImVec2(StyleProvider::WindowPadding.x, StyleProvider::TitleBarHeight + StyleProvider::WindowPadding.y));

		if ((State & LoggedIn) != LoggedIn)
		{
			ImGui::BeginChild(xor ("LoginArea"), ImVec2(340, 180));
			{
				ImVec2 loginAreaSize = ImGui::GetWindowSize();

				ImGui::PushFont(StyleProvider::FontDefault);
				auto usernameSize = ImGui::CalcTextSize(xor ("Username"));
				ImGui::SetCursorPosX(loginAreaSize.x / 2 - usernameSize.x / 2);
				ImGui::Text(xor ("Username"));

				ImGui::PushItemWidth(loginAreaSize.x);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, StyleProvider::TextBoxColour);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				ImGui::InputText(xor ("username"), usernameBuf, 24);
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();

				ImGui::Spacing();
				ImGui::Spacing();

				auto passwordSize = ImGui::CalcTextSize(xor ("Password"));
				ImGui::SetCursorPosX(loginAreaSize.x / 2 - passwordSize.x / 2);
				ImGui::Text(xor ("Password"));

				ImGui::PushItemWidth(loginAreaSize.x);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, StyleProvider::TextBoxColour);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				ImGui::InputText(xor ("password"), passwordBuf, 256, ImGuiInputTextFlags_Password);
				ImGui::PopStyleVar();
				ImGui::PopStyleColor();

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::SetCursorPosX(loginAreaSize.x / 2 - 50);
				if (ImGui::Button(xor (State == LoggingIn ? "Logging in..." : "Login"), ImVec2(100, ImGui::GetFrameHeight())))
				{
					//construct login packet here later =w=
					std::string hwid = HWID::GetHWID();
					if (hwid.empty())
					{
						MessageBoxA(UI::Window, xor ("Failed to gather hardware information!\nPlease report this.\n\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

						return 0;
					}

					// Construct Login Packet
					LoginRequest loginPacket = LoginRequest(hwid, std::string(usernameBuf), std::string(passwordBuf), mClient);
					
					pipe_ret_t sendRet = client.sendBytes(loginPacket.Data);
					if (!sendRet.success)
					{
						MessageBoxA(UI::Window, xor ("Failed to communicate with the server!\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

						ShutdownAndExit();
					}

					State = State | LoggingIn;
				}

				ImGui::PopFont();
			}
			ImGui::EndChild();
		}
		else
		{
			ImGui::BeginChild(xor("LoaderArea"), ImVec2(340, 180));
			{
				ImVec2 loaderAreaSize = ImGui::GetWindowSize();

				ImGui::PushFont(StyleProvider::FontDefault);
				std::string usernameText(std::string(xor("Logged in as ")) + usernameBuf);
				auto usernameSize = ImGui::CalcTextSize(usernameText.c_str());
				ImGui::SetCursorPosX(loaderAreaSize.x / 2 - usernameSize.x / 2);
				ImGui::TextColored(StyleProvider::TitleColour, usernameText.c_str());

				ImGui::Spacing();
				ImGui::Spacing();

				auto comboLabelSize = ImGui::CalcTextSize(xor("Select cheat"));
				ImGui::SetCursorPosX(loaderAreaSize.x / 2 - comboLabelSize.x / 2);
				ImGui::Text(xor ("Select cheat"));

				const char* combo[] = { xor ("Maple Lite") };
				ImGui::PushItemWidth(loaderAreaSize.x);
				ImGui::PushStyleColor(ImGuiCol_Button, StyleProvider::DropDownArrowColour);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, StyleProvider::DropDownArrowHoveredColour);
				ImGui::Combo("", &currItem, combo, IM_ARRAYSIZE(combo));
				ImGui::PopStyleColor(2);

				auto expiresAtSize = ImGui::CalcTextSize(expiresAt.c_str());
				ImGui::SetCursorPosX(loaderAreaSize.x / 2 - expiresAtSize.x / 2);
				ImGui::TextColored(StyleProvider::ExpirationColour, xor (expiresAt.c_str()));

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::SetCursorPosX(loaderAreaSize.x / 2 - 50);
				if(ImGui::Button(xor ("Load"), ImVec2(100, ImGui::GetFrameHeight())))
				{
					// TODO: maple onii-chan, we need some visuals for the loading process
					// request maple dll
					DllStreamRequest dllStream = DllStreamRequest(mClient);

					pipe_ret_t sendRet = client.sendBytes(dllStream.Data);
					if (!sendRet.success)
					{
						MessageBoxA(UI::Window, xor ("Failed to communicate with the server!\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

						ShutdownAndExit();
					}

					State = State | DownloadingDLL;
				}

				ImGui::PopFont();
			}
			ImGui::EndChild();
		}
	}
	ImGui::End();

	ImGui::EndFrame();
	if (UI::D3DDevice->BeginScene() == D3D_OK)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		UI::D3DDevice->EndScene();
	}

	HRESULT result = UI::D3DDevice->Present(0, 0, 0, 0);
	if (result == D3DERR_DEVICELOST && UI::D3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		HRESULT hr = UI::D3DDevice->Reset(&UI::D3DPresentParams);
		if (hr == D3DERR_INVALIDCALL)
			return false;

		ImGui_ImplDX9_CreateDeviceObjects();
	}

	return true;
}

void OnIncomingMessage(const char* msg, size_t size)
{
	auto* const response = static_cast<Response*>(Response::ConstructResponse(msg, size, mClient));
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

			ShutdownAndExit();

			break;
		}
		case ResponseType::Handshake:
		{
			auto* const handshakeResponse = static_cast<HandshakeResponse*>(response);

			mClient = new MatchedClient(client);
			mClient->aes->SetIV(handshakeResponse->IV);
			mClient->aes->SetKey(handshakeResponse->Key);
			
			break;
		}
		case ResponseType::Login:
		{
			auto* const loginResponse = static_cast<LoginResponse*>(response);
				
			sessionToken = loginResponse->SessionToken;
			expiresAt = "Expires at: " + loginResponse->ExpiresAt;

			switch (loginResponse->Result)
			{
				case LoginResult::Success:
				{
					State = LoggedIn;

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
				case LoginResult::InvalidRequest:
				{
					MessageBoxA(UI::Window, xor ("Invalid request!"), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

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
			MessageBoxA(UI::Window, xor ("Received DLL Stream"), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

			State = State & ~DownloadingDLL; // remove the downloading flag
			State = State | Injecting; // start injecting and therefore add the injecting flag

			break;
		}
	}

	delete response;
}

void OnDisconnection(const pipe_ret_t& ret)
{
	MessageBoxA(UI::Window, xor ("You have been disconnected from the server!\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

	ShutdownAndExit();
}

bool ConnectToServer()
{
	client_observer_t observer;
	observer.wantedIp = "127.0.0.1";
	observer.incoming_packet_func = OnIncomingMessage;
	observer.disconnected_func = OnDisconnection;
	client.subscribe(observer);

	pipe_ret_t connectRet = client.connectTo("127.0.0.1", 9999);
	if (connectRet.success)
	{
		// Send initial Handshake, to get RSA Encrypted Client Key and IV
		HandshakeRequest handshakePacket = HandshakeRequest();

		if (const pipe_ret_t sendRet = client.sendBytes(handshakePacket.Data); !sendRet.success)
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
	
	MSG msg;
	memset(&msg, 0, sizeof(msg));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		if (!DrawMenu())
		{
			MessageBoxA(UI::Window, xor ("Internal graphics error occurred!\nPlease make sure your graphics card drivers are up to date."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
			
			break;
		}
	}

	ShutdownAndExit();
}
