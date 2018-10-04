#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/RaycastResults2D.hpp"
#include <vector>

class VertexBuffer;
class IndexBuffer;
class Log;
class PathGenerator;

typedef std::vector<Tile*> Path;

class Map
{
public:
	std::string m_name;
	std::vector<Tile> m_tiles;
	std::vector<Feature*> m_features;
	std::vector<Character*> m_characters;
	std::vector<RGBA> m_tileColors;
	PathGenerator *m_currentPath;
	static std::vector<VertexBuffer*> m_fillVBA;
	static std::vector<IndexBuffer*> m_fillIBA;
	IntVector2 m_dimensions;
	unsigned char m_currentFloodFillIteration;
	byte m_maxNumCarrots;
	byte m_carrotCount;
	bool m_wasFeatureJustAdded;

	Map();
	Map(IntVector2 dimensions);
	~Map();

	void Initialize();
	void InitFeatures();

	void Update(float deltaSeconds);
	void UpdateCarrots(float deltaSeconds);
	void Render();

	void FillLogWithTileInfo(Log *log, const Vector2 &mousePos);
	void SetFillVBAandIBA();
	void ProcessTurn(float deltaSeconds);
	void ClearVBAsandIBAs();
	void ResolveAttack(Character* attacker, Tile *defendingTile);
	void CreateCharacterOfType(std::string characterType, const IntVector2& position);

	Tile* FindRandomTileInCardinalDirection(const IntVector2& position);
	Tile* FindRandomTileAdjacent(const IntVector2& position);
	Tile* FindFirstNonSolidTile();
	Tile* FindRandomPassableTileOfType(std::string tileType = "");
	Tile* FindRandomTile();
	Tile* GetTileAtPosition(const IntVector2& position);
	Tile* FindClosestTileWithoutFeature(const Vector2& position);
	int GetTileIndexFromCoords(const IntVector2& position);
	IntVector2 GetTileCoordsFromIndex(int index);
	Tile* GetTileToNorth(Tile* currentTile);
	Tile* GetTileToSouth(Tile* currentTile);
	Tile* GetTileToWest(Tile* currentTile);
	Tile* GetTileToEast(Tile* currentTile);
	Tile* GetTileToNorthEast(Tile *currentTile);
	Tile* GetTileToNorthWest(Tile *currentTile);
	Tile* GetTileToSouthEast(Tile *currentTile);
	Tile* GetTileToSouthWest(Tile *currentTile);
	int CalcNumNeighborsOfType(Tile *currentTile, TileDefinition *tileDef);
	bool IsOutOfBounds(const IntVector2& position);
	bool IsTilePositionSolid(const IntVector2& position);
	bool IsTilePositionPassable(const IntVector2& position);
	Vector2 Raycast(const Vector2& start, const Vector2& end, bool canHitOpaque, bool canHitSolid);
	RaycastResults2D RaycastAW(const Vector2& start, const Vector2& end, bool canHitOpaque, bool canHitSolid);
	float GetLongestCharacterNameSize(float scale);

	Character* FindNearestActorOfType(const IntVector2& position, std::string typeName);
	Character* FindNearestActorNotOfType(const IntVector2& position, std::string typeName);
	Character* FindNearestActorOfFaction(const IntVector2& position, std::string factionName);
	Character* FindNearestActorOfFactionNotSelf(const IntVector2& position, std::string factionName, Character* self);
	Character* FindNearestActorNotOfFaction(const IntVector2& position, std::string factionName);
	Character* FindNearestActorNotSelf(const IntVector2& position, Character* self);
	bool DoesActorNotOfFactionExistNotSelf(const std::string& factionName, Character* self);
	bool DoesOtherActorExist(Character* self);
	Feature* FindNearestFeatureOfType(const IntVector2& position, const std::string& featureType);
	Feature* FindNearestDestroyableFeature(const IntVector2& position);
	Tile *FindNearestTileOfType(const IntVector2& position, std::string tileType);
	Tile *FindNearestTileNotOfType(const IntVector2& position, std::string tileType);
	bool AreCharactersAdjacent(Character* firstCharacter, Character* secondCharacter);

	Path GeneratePath(IntVector2 start, IntVector2 end, Character *character = nullptr);
	void StartSteppedPath(IntVector2 start, IntVector2 end, Character *character = nullptr);
	bool ContinueSteppedPath(Path& outPath);

	void GenerateDijkstraMapForAllCharacters(Character* exceptionCharacter = nullptr);
	void GenerateDijkstraMapForAllCharactersOfFaction(const std::string& factionName, Character* exceptionCharacter = nullptr);
	void GenerateDijkstraMapForAllCharactersNotOfFaction(const std::string& factionName, Character* exceptionCharacter = nullptr);
	void GenerateDijkstraMapForCharacter(Character* character);
	Tile* GetLowestScoreDijkstraMapTile(const IntVector2& startPos);
	Tile* GetHighestScoreDijkstraMapTile(const IntVector2& startPos);

	void ClearLineOfSights();
	void DetermineLineOfSights();
	int GetColorIndexFromColorMap(const RGBA& color);
};