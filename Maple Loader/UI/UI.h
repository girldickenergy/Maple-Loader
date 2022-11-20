#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <glfw3.h>

class UI
{
	static inline bool inLoginState = true;
	static inline bool shouldSelectFirstGame = true;
	static inline bool canDrag = true;
	static inline int dragOffsetX = 0;
	static inline int dragOffsetY = 0;

	static void setWindowSize(int width, int height);
	static void handleWindowDrag();
public:
	static inline GLFWwindow* GLFWWindow;
	static inline HWND NativeWindow;

	static bool Initialize();
	static void Render();
	static void Shutdown();
};