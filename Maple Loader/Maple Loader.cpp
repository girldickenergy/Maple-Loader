#define _WINSOCKAPI_
#include <windows.h>

#include "AntiDebug/xorstr.hpp"
#include "TCP/TCP_Client.h"
#include "Crypto/Wrapper/RSADecrypt.h"
#include "TCP/Utils/SendByteVector.hpp"

#include "UI/UI.h"
#include "UI/ImGui/imgui_impl_dx9.h"
#include "UI/ImGui/imgui_impl_win32.h"
#include "UI/StyleProvider.h"
#include "Utils/HWID.h"
#include "Utils/StringUtilities.cpp"

TcpClient client;
static inline RSADecrypt* RSAA = new RSADecrypt();

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
bool loggedIn = false;
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
			exit(0);

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

		if (!loggedIn)
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
				if (ImGui::Button(xor ("Login"), ImVec2(100, ImGui::GetFrameHeight())))
				{
					//construct login packet here later =w=
					std::string hwid = HWID::GetHWID();
					if (hwid.empty())
					{
						MessageBoxA(UI::Window, xor ("Failed to gather hardware information!\nPlease report this.\n\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);

						return 0;
					}

					MessageBoxA(UI::Window, xor (hwid.c_str()), xor ("Your HWID"), MB_OK);
					
					loggedIn = true;
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

				auto expiresAt = ImGui::CalcTextSize(xor ("Expires at: never"));
				ImGui::SetCursorPosX(loaderAreaSize.x / 2 - expiresAt.x / 2);
				ImGui::TextColored(StyleProvider::ExpirationColour, xor ("Expires at: never"));

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::SetCursorPosX(loaderAreaSize.x / 2 - 50);
				ImGui::Button(xor ("Load"), ImVec2(100, ImGui::GetFrameHeight()));

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
	/*char text[256] = "A message from the server: ";
	strcat(text, msg);
	
	MessageBoxA(UI::Window, xor (text), xor ("Maple Loader"), MB_OK);*/

	//TODO: add filtering and handler
	// IF HEARTBEAT
	const std::regex re(R"(0xdeadbeef)");
	const std::vector<std::string> tokenized = Split(std::string(msg, size), re);

	std::vector<unsigned char> encrypted = std::vector<unsigned char>();
	for (const auto& byte : tokenized[1])
		encrypted.push_back(byte);

	encrypted.erase(encrypted.begin());

	std::vector<unsigned char> s = RSAA->Decode(encrypted, std::stoi(tokenized[0]));
	//std::cout << "Got msg from server: " << s << std::endl;
}

void OnDisconnection(const pipe_ret_t& ret)
{
	MessageBoxA(UI::Window, xor ("You have been disconnected from the server!\nThe application will now exit."), xor ("Maple Loader"), MB_ICONERROR | MB_OK);
	
	client.finish();
	exit(0);
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
		return true;

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

	UI::Shutdown();

	return 0;
}
