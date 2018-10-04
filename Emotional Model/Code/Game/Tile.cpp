#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Core/Log.hpp"

Tile::Tile()
	: m_mapPosition(IntVector2())
{
	Initialize();
}

Tile::Tile(const IntVector2 &mapPosition)
{
	m_mapPosition = mapPosition;
	Initialize();
}

Tile::~Tile()
{

}

void Tile::Initialize()
{
	m_fillColor = RGBA::WHITE;
	m_feature = nullptr;
	m_character = nullptr;
	m_tileDefition = nullptr;
	m_dijkstraMapFScore = FLT_MAX;
}

void Tile::FillLogWithDetails(Log *log)
{
	log->StoreTextInLog(m_tileDefition->m_name, RGBA::WHITE);
	if (m_tileDefition->m_isSolid)
		log->StoreTextInLog(" -Is Solid", RGBA::WHITE, SUBTITLE);
	if (m_tileDefition->m_isOpaque)
		log->StoreTextInLog(" -Is Opaque", RGBA::WHITE, SUBTITLE);

	if (m_character != nullptr)
	{
		m_character->FillLogWithDetails(log);
	}
	if (m_feature != nullptr)
	{
		m_feature->FillLogWithDetails(log);
	}
}

void Tile::SetTileDefinition(TileDefinition *tileDefinition)
{
	m_tileDefition = tileDefinition;

	if (m_tileDefition->m_glyphs.size() > 0)
	{
		int glyphIndex = GetRandomIntInRange(0, (int)m_tileDefition->m_glyphs.size() - 1);
		m_glyph = m_tileDefition->m_glyphs[glyphIndex];
	}

	int fillColorIndex = GetRandomIntInRange(0, (int)m_tileDefition->m_fillColors.size() - 1);
	m_fillColor = m_tileDefition->m_fillColors[fillColorIndex];
}

void Tile::InteractWithCharacter()
{
	if (m_feature != nullptr)
		m_feature->InteractWithCharacter(m_character);
}

Vector2 Tile::GetTileCenter()
{
	Vector2 newPosition = m_mapPosition;
	newPosition.x += 0.5f;
	newPosition.y += 0.5f;
	return newPosition;
}

bool Tile::IsSolid()
{
	if (m_tileDefition->m_isSolid)
	{
		return true;
	}
	if (m_character != nullptr)
	{
		return true;
	}
	if (m_feature != nullptr)
	{
		if (m_feature->m_isSolid)
		{
			return true;
		}
	}
	return false;
}

bool Tile::IsPassable()
{
	if (!m_tileDefition->m_isPassable)
	{
		return false;
	}
// 	if (m_character != nullptr)
// 	{
// 		return false;
// 	}
	if (m_feature != nullptr)
	{
		if (m_feature->m_isSolid)
		{
			return false;
		}
	}
	return true;
}

bool Tile::HasCharacter()
{
	return m_character != nullptr;
}

bool Tile::HasFeature()
{
	return m_feature != nullptr;
}

void Tile::DestroyFeature()
{
	delete m_feature;
	m_feature = nullptr;
}

float Tile::GetLocalGCost()
{
	return m_tileDefition->GetLocalGCost();
}

std::string Tile::GetName()
{
	return m_tileDefition->m_name;
}
