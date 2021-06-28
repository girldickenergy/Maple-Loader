#include "UI.h"

#include "StyleProvider.h"
#include "../resource.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT UI::wndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
		return true;

	if (message == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

HWND UI::createWindow(HINSTANCE instance, const std::pair<int, int> size)
{
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));

	HICON hIcon = static_cast<HICON>(::LoadImage(instance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR));
	
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = wndProc;
	wc.hInstance = instance;
	wc.hIcon = hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = 0;
	wc.lpszClassName = L"LoaderClass";

	RECT pos;
	GetClientRect(GetDesktopWindow(), &pos);
	pos.left = (pos.right / 2) - (size.first / 2);
	pos.top = (pos.bottom / 2) - (size.second / 2);

	RegisterClassEx(&wc);

	return CreateWindowEx(0, wc.lpszClassName, L"Maple Loader", WS_POPUP, pos.left, pos.top, size.first, size.second, 0, 0, wc.hInstance, 0);
}

bool UI::createDevice(HWND hwnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d)
		return false;

	memset(&D3DPresentParams, 0, sizeof(D3DPresentParams));

	D3DPresentParams.Windowed = TRUE;
	D3DPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	D3DPresentParams.BackBufferFormat = D3DFMT_UNKNOWN;
	D3DPresentParams.EnableAutoDepthStencil = TRUE;
	D3DPresentParams.AutoDepthStencilFormat = D3DFMT_D16;
	D3DPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	auto res = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &D3DPresentParams, &D3DDevice);
	if (res != D3D_OK)
		return false;

	return true;
}

bool UI::Initialize(HINSTANCE instance, int showCmd)
{
	Window = createWindow(instance, { 400, 250 });
	if (!Window)
		return false;

	if (!createDevice(Window))
		return false;

	ShowWindow(Window, showCmd);

	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	StyleProvider::LoadFonts();
	StyleProvider::LoadColours();
	
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(0, 0);
	style.WindowBorderSize = 0;
	style.ItemSpacing = ImVec2(5, 10);
	style.FrameRounding = 5;

	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX9_Init(D3DDevice);

	return true;
}

void UI::Shutdown()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	D3DDevice->Release();
	d3d->Release();
	DestroyWindow(Window);
}
