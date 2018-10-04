#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/RGBA.hpp"

class Log;
class Map;
class Character;
class Tile;
class Texture2D;

class Feature
{
public:
	std::string m_name;
	bool m_isSolid;
	bool m_isDestoyable;
	bool m_isOpenable;
	byte m_damageDealt;
	byte m_health;
	IntVector2 m_destinationTilePos;
	Tile *m_owningTile;
	Map *m_owningMap;
	Map *m_destinationLevel;
	char m_glyph;
	RGBA m_glyphColor;
	Vector2 m_logPosition;
	Texture2D* m_imageTexture;

	Feature(Map *owningMap, std::string tileType = "");
	~Feature();

	void Initialize();
	void InitTexture(std::string filePath);
	void FillLogWithDetails(Log *log);

	void InteractWithCharacter(Character *character);
	void Update(float deltaSeconds);
	void Render() const;
};