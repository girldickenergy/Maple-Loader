#pragma once

#include "AnimatedTexture.h"
#include "ImGui/imgui.h"

class AnimationHandler
{
private:
	static void StartAnimation(AnimatedTexture aniTex);
public:
	static void DoAnimation(AnimatedTexture aniTex, const ImVec2& loc, const ImVec2& max);
};
