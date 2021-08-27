#include "AnimationHandler.h"

#include "../Utils/TextureUtils.h"
#include "StyleProvider.h"

void AnimationHandler::StartAnimation(AnimatedTexture* aniTex)
{
	aniTex->startedAt = GetTickCount();
	aniTex->lastFrameUpdate = GetTickCount();
}

void AnimationHandler::DoAnimation(AnimatedTexture* aniTex, const ImVec2& loc, const ImVec2& max)
{
	if (!aniTex->IsStarted())
		StartAnimation(aniTex);

	if (aniTex->currFrame == aniTex->Textures.size() - 1) // loop
		aniTex->currFrame = 0;
	
	if (GetTickCount() - aniTex->lastFrameUpdate > 33) { // 30fps
		aniTex->currFrame++;
		aniTex->lastFrameUpdate = GetTickCount();
	}

	if (!aniTex->AreTexturesConstructed())
		aniTex->ConstructTextures();

	if (IDirect3DTexture9* tex = aniTex->GetCurrentFrameTexture(); tex!=nullptr)
		ImGui::GetWindowDrawList()->AddImage(tex, ImVec2(0, 0), StyleProvider::WindowSize);
}
