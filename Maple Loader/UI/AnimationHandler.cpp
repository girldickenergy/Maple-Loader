#include "AnimationHandler.h"

#include "../Utils/TextureUtils.h"

void AnimationHandler::SetFrames(std::vector<IDirect3DTexture9*> _frames)
{
	frames = _frames;
	Reset();
}

void AnimationHandler::Reset()
{
	firstFrame = true;
	currentFrame = 0;
}

void AnimationHandler::DoAnimation(const ImVec2& loc, const ImVec2& max)
{
	if (frames.empty())
		return;

	ImGui::GetWindowDrawList()->AddImage(frames[currentFrame], loc, max);

	if (currentFrame + 1 == frames.size())
	{
		Reset();
	}
	else if (!firstFrame) //limit to 30fps
	{
		currentFrame++;
		firstFrame = true;
	}
	else
	{
		firstFrame = false;
	}
}
