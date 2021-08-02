#pragma once

#include <vector>
#include <Windows.h>
#include "../Utils/TextureUtils.h"

struct AnimatedTexture
{
	int currFrame = 0;
	DWORD startedAt = 0;
	DWORD lastFrameUpdate = 0;
	std::vector<std::vector<unsigned char>> Frames = std::vector<std::vector<unsigned char>>();
	std::vector<IDirect3DTexture9*> Textures = std::vector<IDirect3DTexture9*>();
	AnimatedTexture(){}
	AnimatedTexture(std::vector<std::vector<unsigned char>> frames)
	{
		Frames = frames;
	}

	bool AreTexturesConstructed()
	{
		return Frames.empty() && !Textures.empty();
	}

	void ConstructTextures()
	{
		// Construct textures
		if (!AreTexturesConstructed())
		{
			for (const auto& vec : Frames)
			{
				IDirect3DTexture9* tex = TextureUtils::CreateTexture(vec.data(), vec.size());
				Textures.push_back(tex);
			}
			Frames.clear(); // We constructed all textures -> clear frames from memory
		}
	}
	
	bool IsStarted()
	{
		return startedAt > 0;
	}

	IDirect3DTexture9* GetCurrentFrameTexture()
	{
		if (AreTexturesConstructed())
			return Textures[currFrame];
		return nullptr;
	}
};
