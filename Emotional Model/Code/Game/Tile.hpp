#pragma once
#include "Engine/Core/RGBA.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Game/Character.hpp"
#include "Game/Feature.hpp"

class TileDefinition;
class Log;

class Tile
{
public:
	RGBA m_fillColor;
	char m_glyph;
	Feature *m_feature;
	Character *m_character;
	TileDefinition *m_tileDefition;
	IntVector2 m_mapPosition;
	float m_dijkstraMapFScore;

	Tile();
	Tile(const IntVector2 &mapPosition);
	~Tile();

	void Initialize();

	void FillLogWithDetails(Log *log);
	void SetTileDefinition(TileDefinition *tileDefinition);
	void InteractWithCharacter();
	Vector2 GetTileCenter();

	bool IsSolid();
	bool IsPassable();
	bool HasCharacter();
	bool HasFeature();
	void DestroyFeature();
	float GetLocalGCost();
	std::string GetName();
};