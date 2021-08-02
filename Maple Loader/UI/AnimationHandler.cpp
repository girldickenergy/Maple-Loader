#include "AnimationHandler.h"
#include "../Utils/TextureUtils.h"
#include "StyleProvider.h"

#include "DirectX/d3dx9tex.h"
#pragma comment(lib, "D3dx9")

void AnimationHandler::StartAnimation(AnimatedTexture aniTex)
{
	aniTex.startedAt = GetTickCount();
	aniTex.lastFrameUpdate = GetTickCount();
}

void AnimationHandler::DoAnimation(AnimatedTexture aniTex, const ImVec2& loc, const ImVec2& max)
{
	if (!aniTex.IsStarted())
		StartAnimation(aniTex);

	if (aniTex.currFrame == aniTex.Frames.size()) // loop
		aniTex.currFrame = 0;
	
	if (GetTickCount() - aniTex.lastFrameUpdate > 33) { // 30fps
		aniTex.currFrame++;
		aniTex.lastFrameUpdate = GetTickCount();
	}

	IDirect3DTexture9* tex = TextureUtils::CreateTexture(aniTex.Frames[aniTex.currFrame].data(), aniTex.Frames[aniTex.currFrame].size());
	ImGui::GetWindowDrawList()->AddImage(tex, ImVec2(0, 0), StyleProvider::WindowSize);
}
