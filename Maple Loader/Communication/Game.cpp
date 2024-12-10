#include "Game.h"

#include "entt.hpp"
#include "Fnv1a.h"

#include "../Utilities/Security/xorstr.hpp"
#include "../Utilities/Textures/TextureLoader.h"
#include "../Utilities/Reflection/TypeRegistrar.h"
#include "../UI/Assets/Textures.h"

static const TypeRegistrar<Game> registrar;

Game::~Game()
{
	if (m_IconTexture)
		TextureLoader::FreeTexture(m_IconTexture);

	if (m_BannerTexture)
		TextureLoader::FreeTexture(m_BannerTexture);
}

uint32_t Game::GetID()
{
	return m_ID;
}

const std::string& Game::GetName()
{
	return m_Name;
}

void* Game::GetIconTexture()
{
	if (!m_IconTexture)
	{
		m_IconTexture = TextureLoader::LoadTextureFromURL(xorstr_("https://maple.software/assets/games/icons/") + std::to_string(m_ID) + xorstr_(".png"));

		if (!m_IconTexture)
			m_IconTexture = TextureLoader::LoadTextureFromMemory(Textures::DefaultGameIcon, Textures::DefaultGameIconSize);
	}

	return m_IconTexture;
}

void* Game::GetBannerTexture()
{
	if (!m_BannerTexture)
	{
		m_BannerTexture = TextureLoader::LoadTextureFromURL(xorstr_("https://maple.software/assets/games/banners/") + std::to_string(m_ID) + xorstr_(".png"));

		if (!m_BannerTexture)
			m_BannerTexture = TextureLoader::LoadTextureFromMemory(Textures::DefaultBanner, Textures::DefaultBannerSize);
	}

	return m_BannerTexture;
}

void Game::Register()
{
	if (registrar.IsRegistered)
		return;

	entt::meta<Game>()
		.data<&Game::m_ID>(Hash32Fnv1aConst("ID"))
		.data<&Game::m_Name>(Hash32Fnv1aConst("Name"));
}
