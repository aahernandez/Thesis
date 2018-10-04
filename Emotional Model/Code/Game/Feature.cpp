#include "Game/Feature.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/RHI/SimpleRenderer.hpp"
#include "Engine/RHI/Texture2D.hpp"

Feature::Feature(Map *owningMap, std::string tileType /*= ""*/)
{
	m_owningMap = owningMap;
	m_owningTile = owningMap->FindRandomPassableTileOfType(tileType);
	Initialize();
	m_owningTile->m_feature = this;
	m_destinationTilePos = m_owningTile->m_mapPosition;
	m_destinationLevel = owningMap;
}

Feature::~Feature()
{
	m_owningTile->m_feature = nullptr;
	m_owningTile = nullptr;
}

void Feature::Initialize()
{
	m_glyph = 'f';
	m_glyphColor = RGBA::PURPLE;
	m_isSolid = true;
	m_destinationTilePos = IntVector2(-1, -1);
	m_destinationLevel = nullptr;
}

void Feature::InitTexture(std::string filePath)
{
	if (filePath.empty())
	{
		m_imageTexture = nullptr;
	}
	else
	{
		m_imageTexture = new Texture2D(g_theSimpleRenderer->m_rhiDevice, filePath.c_str());
	}
}

void Feature::FillLogWithDetails(Log *log)
{
	log->StoreTextInLog(Stringf("%s (%s%c[-])", m_name.c_str(), m_glyphColor.GetAsString().c_str(), m_glyph), RGBA::WHITE);
	if (m_isSolid)
		log->StoreTextInLog(" -Is Solid", RGBA::WHITE, SUBTITLE);
	if (m_isDestoyable)
		log->StoreTextInLog(" -Is Destroyable", RGBA::WHITE, SUBTITLE);
	if (m_isOpenable)
		log->StoreTextInLog(" -Is Openable", RGBA::WHITE, SUBTITLE);
	if (m_damageDealt > 0)
		log->StoreTextInLog(Stringf(" -Deals [1.0,0.0,0.0]%i[-] damage", m_damageDealt), RGBA::WHITE, SUBTITLE);
}

void Feature::InteractWithCharacter(Character *character)
{
	character;
}

void Feature::Update(float deltaSeconds)
{
	deltaSeconds;
}

void Feature::Render() const
{
	if (m_imageTexture == nullptr)
	{
		std::string glyphString = "";
		glyphString += m_glyph;
		g_theSimpleRenderer->DrawText2DCenteredOnPosition(glyphString.c_str(), Vector2(m_owningTile->m_mapPosition.x + 0.5f, m_owningTile->m_mapPosition.y + 0.5f), m_glyphColor);
	}
	else
	{
		g_theSimpleRenderer->DrawQuad2DTextured( Vector2(m_owningTile->m_mapPosition) + Vector2(0.2f, 0.2f), Vector2(m_owningTile->m_mapPosition) + Vector2(0.8f, 0.8f), m_imageTexture, m_glyphColor);
	}
}