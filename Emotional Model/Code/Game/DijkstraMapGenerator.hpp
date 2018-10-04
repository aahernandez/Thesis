#pragma once
#include "Engine/Math/IntVector2.hpp"
#include "Game/Map.hpp"

class Tile;

class DijkstraMapGenerator
{
public:
	Tile *m_currentTile;
	Map& m_map;
	std::vector<Tile*> m_openList;
	std::vector<Tile*> m_closedList;
	bool m_isReturningPathToCurrentTile;

	DijkstraMapGenerator(Map& map);
	~DijkstraMapGenerator();

	void Destroy();

	void SelectAndCloseBestOpenTileForMap();
	void AddNeighborsToOpenList();
	void AddNeighborToOpenList(const IntVector2& tilePosition);
	void CreatePathToGoal(Path& outPath);
	void AddTileToOpenList(const IntVector2& tilePosition, float originTileFScore);
	bool IsInClosedList(const IntVector2& tilePosition);
	bool IsInOpenList(const IntVector2& tilePosition);
};