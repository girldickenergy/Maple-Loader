#pragma once

#include <d3d9.h>
#include <vector>

#include "ImGui/imgui.h"

class AnimationHandler
{
	static inline std::vector<IDirect3DTexture9*> frames;
	static inline bool firstFrame = true;
	static inline int currentFrame = 0;
public:
	static void SetFrames(std::vector<IDirect3DTexture9*> _frames);
	static void Reset();
	static void DoAnimation(const ImVec2& loc, const ImVec2& max);
};
