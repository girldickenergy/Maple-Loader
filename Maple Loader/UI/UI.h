#pragma once

#pragma comment(lib,"d3d9.lib")
#include <d3d9.h>

#include "ImGui/imgui.h"

class UI
{
	static LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	static HWND createWindow(HINSTANCE instance);
	static bool createDevice(HWND hwnd);

	static inline int dragOffsetX = 0;
	static inline int dragOffsetY = 0;
	static void handleWindowDrag();

	static inline bool shouldSelectFirstGame = true;
public:
	static inline HWND Window;

	static inline IDirect3D9* D3D;
	static inline IDirect3DDevice9* D3DDevice;
	static inline D3DPRESENT_PARAMETERS D3DPresentParams;
	
	static bool Initialize(HINSTANCE instance, int showCmd);
	static bool Render();
	static void Shutdown();
};
