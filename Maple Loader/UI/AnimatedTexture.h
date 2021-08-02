#pragma once

#include <vector>
#include <Windows.h>

struct AnimatedTexture
{
	int currFrame = 0;
	DWORD startedAt = 0;
	DWORD lastFrameUpdate = 0;
	std::vector<std::vector<unsigned char>> Frames = std::vector<std::vector<unsigned char>>();
	AnimatedTexture(){}
	AnimatedTexture(std::vector<std::vector<unsigned char>> frames)
	{
		Frames = frames;
	}

	bool IsStarted()
	{
		return startedAt > 0;
	}
};
