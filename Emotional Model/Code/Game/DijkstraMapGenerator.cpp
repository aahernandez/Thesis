#include "Game/DijkstraMapGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/RHI/SimpleRenderer.hpp"

DijkstraMapGenerator::DijkstraMapGenerator(Map& map)
	: m_map(map)
	, m_isReturningPathToCurrentTile(false)
{

}

DijkstraMapGenerator::~DijkstraMapGenerator()
{
	Destroy();
}


void DijkstraMapGenerator::Destroy()
{
	std::vector<Tile*>::iterator closedListIter;
	for (closedListIter = m_closedList.begin(); closedListIter != m_closedList.end(); ++closedListIter)
	{
		delete *closedListIter;
		*closedListIter = nullptr;
	}

	std::vector<Tile*>::iterator openListIter;
	for (openListIter = m_openList.begin(); openListIter != m_openList.end(); ++openListIter)
	{
		delete *openListIter;
		*openListIter = nullptr;
	}
}

void DijkstraMapGenerator::SelectAndCloseBestOpenTileForMap()
{
	float lowestFScore = FLT_MAX;
	int lowestFScoreIndex = INT_MAX;
	for (int openListCount = 0; openListCount < (int)m_openList.size(); openListCount++)
	{
		if (m_openList[openListCount]->m_dijkstraMapFScore < lowestFScore)
		{
			lowestFScore = m_openList[openListCount]->m_dijkstraMapFScore;
			lowestFScoreIndex = openListCount;
		}
	}

	if (lowestFScoreIndex == INT_MAX)
	{
		m_isReturningPathToCurrentTile = true;
		return;
	}
	m_currentTile = m_openList[lowestFScoreIndex];
	m_openList.erase(m_openList.begin() + lowestFScoreIndex);
	m_closedList.push_back(m_currentTile);
}

void DijkstraMapGenerator::AddNeighborsToOpenList()
{
	IntVector2 currentPos = m_currentTile->m_mapPosition;
	AddNeighborToOpenList(IntVector2(currentPos.x + 1, currentPos.y));
	AddNeighborToOpenList(IntVector2(currentPos.x - 1, currentPos.y));
	AddNeighborToOpenList(IntVector2(currentPos.x, currentPos.y + 1));
	AddNeighborToOpenList(IntVector2(currentPos.x, currentPos.y - 1));
}

void DijkstraMapGenerator::AddNeighborToOpenList(const IntVector2& tilePosition)
{
	if (m_map.IsOutOfBounds(tilePosition))
		return;
	if (IsInClosedList(tilePosition) || IsInOpenList(tilePosition))
		return;
	if (!m_map.IsTilePositionPassable(tilePosition))
		return;
	AddTileToOpenList(tilePosition, m_currentTile->m_dijkstraMapFScore);
}

void DijkstraMapGenerator::AddTileToOpenList(const IntVector2& tilePosition, float originTileFScore)
{
	Tile *tileAtPosition = m_map.GetTileAtPosition(tilePosition);
	tileAtPosition->m_dijkstraMapFScore = originTileFScore + 1.f;
	m_openList.push_back(tileAtPosition);
}

bool DijkstraMapGenerator::IsInClosedList(const IntVector2& tilePosition)
{
	std::vector<Tile*>::iterator closedListIter;
	for (closedListIter = m_closedList.begin(); closedListIter != m_closedList.end(); ++closedListIter)
	{
		Tile* currentTile = *closedListIter;
		IntVector2 closedTilePos = currentTile->m_mapPosition;
		if (tilePosition == closedTilePos)
			return true;
	}
	return false;
}

bool DijkstraMapGenerator::IsInOpenList(const IntVector2& tilePosition)
{
	std::vector<Tile*>::iterator openListIter;
	for (openListIter = m_openList.begin(); openListIter != m_openList.end(); ++openListIter)
	{
		Tile* currentTile = *openListIter;
		IntVector2 openTilePos = currentTile->m_mapPosition;
		if (tilePosition == openTilePos)
			return true;
	}
	return false;
}
