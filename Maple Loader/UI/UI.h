#pragma once

#pragma comment(lib,"d3d9.lib")
#include <d3d9.h>

#include "../Communication/States.h"

class UI
{
	static inline WNDCLASSEX windowClass = {};
	static inline bool canDrag = true;
	static inline int dragOffsetX = 0;
	static inline int dragOffsetY = 0;
	static inline bool shouldSelectFirstGame = true;
	static inline States previousState = States::Idle;

	static bool createD3DDevice(HWND hWnd);
	static void cleanupD3DDevice();
	static void resetDevice();
	static LRESULT WINAPI wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void handleWindowDrag();
	static void updateWindowSize();
public:
	static inline HWND Window = NULL;
	static inline LPDIRECT3D9 D3D = NULL;
	static inline LPDIRECT3DDEVICE9 D3DDevice = NULL;
	static inline D3DPRESENT_PARAMETERS D3DPresentParameters = {};

	static bool Initialize(HINSTANCE hInst);
	static void Render();
	static void Shutdown();
};