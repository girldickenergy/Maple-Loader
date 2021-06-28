#pragma once

#include <utility>
#include <windows.h>

#pragma comment(lib,"d3d9.lib")
#include <d3d9.h>

#include "ImGui/imgui.h"

class UI
{
	static inline IDirect3D9* d3d;

	static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	static HWND createWindow(HINSTANCE instance, std::pair<int, int> size);
	static bool createDevice(HWND hwnd);
public:
	static inline HWND Window;
	
	static inline IDirect3DDevice9* D3DDevice;
	static inline D3DPRESENT_PARAMETERS D3DPresentParams;
	
	static bool Initialize(HINSTANCE instance, int showCmd);
	static void Shutdown();
};
