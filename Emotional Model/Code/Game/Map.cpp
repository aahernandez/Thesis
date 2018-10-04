#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/CharacterBuilder.hpp"
#include "Game/Character.hpp"
#include "Game/Behavior.hpp"
#include "Game/FeatureBuilder.hpp"
#include "Game/PathGenerator.hpp"
#include "Game/DijkstraMapGenerator.hpp"
#include "Game/EmotionStatsScreen.hpp"
#include "Engine/RHI/SimpleRenderer.hpp"
#include "Engine/RHI/Font.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/IndexBuffer.hpp"
#include "Engine/Profiling/Profiler.hpp"
#include "Engine/RHI/MeshBuilder.hpp"
#include "Engine/Math/Math2D.hpp"

std::vector<VertexBuffer*> Map::m_fillVBA;
std::vector<IndexBuffer*> Map::m_fillIBA;

Map::Map()
	: m_currentPath(nullptr)
	, m_currentFloodFillIteration(0)
	, m_maxNumCarrots(10)
	, m_carrotCount(0)
	, m_wasFeatureJustAdded(false)
{
}

Map::Map(IntVector2 dimensions)
	: m_currentPath(nullptr)
	, m_dimensions(dimensions)
	, m_currentFloodFillIteration(0)
	, m_maxNumCarrots(10)
	, m_carrotCount(0)
	, m_wasFeatureJustAdded(false)
{
	m_tiles.resize(m_dimensions.x * m_dimensions.y);
	for (int tileCount = 0; tileCount < (int)m_tiles.size(); tileCount++)
	{
		IntVector2 tileCoords = GetTileCoordsFromIndex(tileCount);
		m_tiles[tileCount].m_mapPosition = tileCoords;
		m_tiles[tileCount].SetTileDefinition(TileDefinition::s_registry["grass"]);
	}
}

Map::~Map()
{
	ClearVBAsandIBAs();
}

void Map::Initialize()
{
	InitFeatures();
}

void Map::InitFeatures()
{
	while (m_carrotCount != m_maxNumCarrots)
	{
		UpdateCarrots(0.f);
	}
}

void Map::Update(float deltaSeconds)
{
	std::vector<Character*>::iterator characterIter = m_characters.begin();
	int characterSize = (int)m_characters.size();
	for (int characterIndex = 0; characterIndex < characterSize; characterIndex++)
	{
		Character* currentCharacter = *characterIter;
		if (currentCharacter->m_currentHealth <= 0)
		{
			std::vector<Character*>::iterator secondCharacterIter;
			for (secondCharacterIter = m_characters.begin(); secondCharacterIter != m_characters.end(); ++secondCharacterIter)
			{
				Character* secondCurrentCharacter = *secondCharacterIter;
				if (secondCurrentCharacter->m_currentBehavior->m_patientCharacter == currentCharacter)
				{
					secondCurrentCharacter->m_currentBehavior->m_patientCharacter = nullptr;
					secondCurrentCharacter->m_target = FindRandomTile()->m_mapPosition;
				}
			}

			Entity::RemoveEntityFromRegistry(currentCharacter->m_name);

			delete currentCharacter;
			currentCharacter = nullptr;
			m_wasFeatureJustAdded = true;
			characterIter = m_characters.erase(characterIter);
			if (characterIter == m_characters.end())
			{
				break;
			}
		}
		++characterIter;
	}

	characterIter = m_characters.begin();
	for (int characterIndex = 0; characterIndex < characterSize; characterIndex++)
	{
		Character* currentCharacter = *characterIter;
		if (currentCharacter->m_currentHealth > 0)
		{
			currentCharacter->Update(deltaSeconds);
		}
		++characterIter;
	}
	m_wasFeatureJustAdded = false;

	characterIter = m_characters.begin();
	for (int characterIndex = 0; characterIndex < characterSize; characterIndex++)
	{
		Character* currentCharacter = *characterIter;
		currentCharacter->UpdateEntity(deltaSeconds);
		++characterIter;
	}

	UpdateCarrots(deltaSeconds);

	std::vector<Feature*>::iterator featureIter = m_features.begin();
	while (featureIter != m_features.end())
	{
		Feature* currentFeature = *featureIter;
		currentFeature->Update(deltaSeconds);
		if (currentFeature->m_health <= 0.f)
		{
			delete currentFeature;
			featureIter = m_features.erase(featureIter);
			m_carrotCount--;
		}
		else
		{
			++featureIter;
		}
	}

}

void Map::UpdateCarrots(float deltaSeconds)
{
	m_wasFeatureJustAdded = false;
	deltaSeconds;
	if (m_carrotCount < m_maxNumCarrots)
	{
		Feature* newFeature = FeatureBuilder::BuildFeature("Carrot", this);
		m_features.push_back(newFeature);
		m_carrotCount++;
		m_wasFeatureJustAdded = true;
	}
	else if (m_carrotCount > m_maxNumCarrots)
	{
		std::vector<Feature*>::iterator featureIter = m_features.begin();
		while (featureIter != m_features.end() && m_carrotCount > m_maxNumCarrots)
		{
			Feature* currentFeature = *featureIter;
			if (currentFeature->m_name == "Carrot")
			{
				delete currentFeature;
				featureIter = m_features.erase(featureIter);
				m_carrotCount--;
			}
			else
			{
				++featureIter;
			}
		}
	}
}

void Map::Render()
{
	AABB2D newOtherSize = Game::s_gameInstance->m_emotionStats->GetAlteredMapOrtho();
	m_tileColors.clear();
	bool isOnLastColor = true;
	int numTilesThisLoop = 0;

	for (int tileIndex = 0; tileIndex < (int)m_tiles.size(); tileIndex++)
	{
		Tile& currentTile = m_tiles[tileIndex];
		RGBA currentColor = RGBA::WHITE;
		int colorIndex = GetColorIndexFromColorMap(currentTile.m_fillColor);

		if (isOnLastColor && colorIndex == -1)
		{
			if (currentTile.m_mapPosition.x > newOtherSize.mins.x - 1 && currentTile.m_mapPosition.x < newOtherSize.maxs.x)
			{
				if (currentTile.m_mapPosition.y > newOtherSize.mins.y - 1 && currentTile.m_mapPosition.y < newOtherSize.maxs.y)
				{
					isOnLastColor = false;
					Vector2 bottomLeft = currentTile.m_mapPosition;
					Vector2 topRight = currentTile.m_mapPosition + Vector2(1.f, 1.f);
					m_tileColors.push_back(RGBA(currentTile.m_fillColor));
					currentColor = currentTile.m_fillColor;
					numTilesThisLoop++;

					g_theSimpleRenderer->m_meshBuilder->Begin(true, g_theSimpleRenderer->m_whiteTexture);

					g_theSimpleRenderer->m_meshBuilder->AddVertex(Vertex(Vector3(bottomLeft.x, bottomLeft.y, 1.f), Vector2(0.f, 0.f), currentTile.m_fillColor));
					g_theSimpleRenderer->m_meshBuilder->AddVertex(Vertex(Vector3(topRight.x, topRight.y, 1.f), Vector2(0.f, 0.f), currentTile.m_fillColor));
					g_theSimpleRenderer->m_meshBuilder->AddVertex(Vertex(Vector3(bottomLeft.x, topRight.y, 1.f), Vector2(0.f, 0.f), currentTile.m_fillColor));
					g_theSimpleRenderer->m_meshBuilder->AddVertex(Vertex(Vector3(topRight.x, bottomLeft.y, 1.f), Vector2(0.f, 0.f), currentTile.m_fillColor));

					g_theSimpleRenderer->m_meshBuilder->AddIndex(0);
					g_theSimpleRenderer->m_meshBuilder->AddIndex(1);
					g_theSimpleRenderer->m_meshBuilder->AddIndex(2);
					g_theSimpleRenderer->m_meshBuilder->AddIndex(0);
					g_theSimpleRenderer->m_meshBuilder->AddIndex(3);
					g_theSimpleRenderer->m_meshBuilder->AddIndex(1);
				}
			}
		}
		else if (!isOnLastColor && colorIndex == (int)m_tileColors.size() - 1)
		{
			if (currentTile.m_mapPosition.x > newOtherSize.mins.x - 1 && currentTile.m_mapPosition.x < newOtherSize.maxs.x)
			{
				if (currentTile.m_mapPosition.y > newOtherSize.mins.y - 1 && currentTile.m_mapPosition.y < newOtherSize.maxs.y)
				{
					Vector2 bottomLeft = currentTile.m_mapPosition;
					Vector2 topRight = currentTile.m_mapPosition + Vector2(1.f, 1.f);
					g_theSimpleRenderer->m_meshBuilder->AddVertex(Vertex(Vector3(bottomLeft.x, bottomLeft.y, 1.f), Vector2(0.f, 0.f), currentTile.m_fillColor));
					g_theSimpleRenderer->m_meshBuilder->AddVertex(Vertex(Vector3(topRight.x, topRight.y, 1.f), Vector2(0.f, 0.f), currentTile.m_fillColor));
					g_theSimpleRenderer->m_meshBuilder->AddVertex(Vertex(Vector3(bottomLeft.x, topRight.y, 1.f), Vector2(0.f, 0.f), currentTile.m_fillColor));
					g_theSimpleRenderer->m_meshBuilder->AddVertex(Vertex(Vector3(topRight.x, bottomLeft.y, 1.f), Vector2(0.f, 0.f), currentTile.m_fillColor));

					g_theSimpleRenderer->m_meshBuilder->AddIndex((numTilesThisLoop * 4) + 0);
					g_theSimpleRenderer->m_meshBuilder->AddIndex((numTilesThisLoop * 4) + 1);
					g_theSimpleRenderer->m_meshBuilder->AddIndex((numTilesThisLoop * 4) + 2);
					g_theSimpleRenderer->m_meshBuilder->AddIndex((numTilesThisLoop * 4) + 0);
					g_theSimpleRenderer->m_meshBuilder->AddIndex((numTilesThisLoop * 4) + 3);
					g_theSimpleRenderer->m_meshBuilder->AddIndex((numTilesThisLoop * 4) + 1);

					numTilesThisLoop++;
				}
			}
		}

		if (tileIndex == (int)m_tiles.size() - 1 && !isOnLastColor)
		{
			g_theSimpleRenderer->m_meshBuilder->End();
			tileIndex = 0;
			currentColor = RGBA::WHITE;
			isOnLastColor = true;
			numTilesThisLoop = 0;
		}
	}

	std::vector<Feature*>::iterator featureIter;
	for (featureIter = m_features.begin(); featureIter != m_features.end(); ++featureIter)
	{
		Feature* currentFeature = *featureIter;
		currentFeature->Render();
	}

	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_characters.begin(); characterIter != m_characters.end(); ++characterIter)
	{
		Character *currentCharacter = *characterIter;
		currentCharacter->Render();
	}
}

void Map::FillLogWithTileInfo(Log *log, const Vector2 &mousePos)
{
	int tileIndex = GetTileIndexFromCoords(mousePos);
	m_tiles[tileIndex].FillLogWithDetails(log);
}

void Map::SetFillVBAandIBA()
{
	ClearVBAsandIBAs();
	m_fillVBA.reserve((m_dimensions.x * m_dimensions.y) * 4);
	m_fillIBA.reserve((m_dimensions.x * m_dimensions.y) * 6);

	std::vector<Tile>::iterator tileIter;
	for (tileIter = m_tiles.begin(); tileIter != m_tiles.end(); ++tileIter)
	{
		VertexBuffer *newVertexBuffer;
		IndexBuffer *newIndexBuffer;
		newVertexBuffer = nullptr;
		newIndexBuffer = nullptr;
		g_theSimpleRenderer->CreateQuad2DTextured(AABB2D(Vector2((float)tileIter->m_mapPosition.x, (float)tileIter->m_mapPosition.y), Vector2((float)tileIter->m_mapPosition.x + 1.f, (float)tileIter->m_mapPosition.y + 1.f)), AABB2D(Vector2(0.f, 0.f), Vector2(1.f, 1.f)), g_theSimpleRenderer->m_whiteTexture, tileIter->m_fillColor, &newVertexBuffer, &newIndexBuffer);
		m_fillVBA.push_back(newVertexBuffer);
		m_fillIBA.push_back(newIndexBuffer);
	}
}

void Map::ProcessTurn(float deltaSeconds)
{
	ClearLineOfSights();
	DetermineLineOfSights();

	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_characters.begin(); characterIter != m_characters.end(); ++characterIter)
	{
		(*characterIter)->Act(deltaSeconds);
	}
	SetFillVBAandIBA();
}

void Map::ClearVBAsandIBAs()
{
	std::vector<VertexBuffer*>::iterator fillVBAIter;
	for (fillVBAIter = m_fillVBA.begin(); fillVBAIter != m_fillVBA.end(); ++fillVBAIter)
	{
		delete *fillVBAIter;
		*fillVBAIter = nullptr;
	}
	m_fillVBA.clear();

	std::vector<IndexBuffer*>::iterator fillIBAIter;
	for (fillIBAIter = m_fillIBA.begin(); fillIBAIter != m_fillIBA.end(); ++fillIBAIter)
	{
		delete *fillIBAIter;
		*fillIBAIter = nullptr;
	}
	m_fillIBA.clear();
}

void Map::ResolveAttack(Character* attacker, Tile *defendingTile)
{
	if (defendingTile->HasCharacter())
	{
		if (!defendingTile->m_character->AreFactionsSame(attacker->m_faction))
		{
			defendingTile->m_character->TakeDamage(attacker->CalcDamage());
		}
	}
	else if (defendingTile->HasFeature())
	{
		if (defendingTile->m_feature->m_isOpenable)
		{
			for (std::vector<Feature*>::iterator featIter = m_features.begin(); featIter != m_features.end(); ++featIter)
			{
				if (*featIter == defendingTile->m_feature)
				{
					m_features.erase(featIter);
					break;
				}
			}
			defendingTile->DestroyFeature();
		}
	}
}

void Map::CreateCharacterOfType(std::string characterType, const IntVector2& position)
{
	Character* newCharacter = CharacterBuilder::BuildCharacter(characterType, this);
	newCharacter->m_owningTile = GetTileAtPosition(position);
	Vector2 randomPosInTile(GetRandomFloatZeroToOne(), GetRandomFloatZeroToOne());
	newCharacter->m_bounds = Disc2D(newCharacter->m_owningTile->GetTileCenter() + randomPosInTile, 0.5f);
	m_characters.push_back(newCharacter);
	if (Game::s_gameInstance->m_isCollectingAnalytics)
	{
		Game::s_gameInstance->CreateAnalyticsEventsForCharacter(newCharacter);
		Game::s_gameInstance->CreateAnalyticsSocialRelationsForCharacter(newCharacter);
		Game::s_gameInstance->CreateAnalyticsStartingAttributesForCharacter(newCharacter);
	}
}

Tile* Map::FindRandomTileInCardinalDirection(const IntVector2& position)
{
	int randomDirInt = GetRandomIntInRange(IntVector2(0, 3));
	IntVector2 targetTileCoords = position;
	switch (randomDirInt)
	{
	case 0: //north
		targetTileCoords.y = GetRandomIntInRange(position.y + 1, m_dimensions.y - 1);
		break;
	case 1: //east
		targetTileCoords.x = GetRandomIntInRange(position.x + 1, m_dimensions.x - 1);
		break;
	case 2: //south
		targetTileCoords.y = GetRandomIntInRange(0, position.y - 1);
		break;
	case 3: //west
		targetTileCoords.x = GetRandomIntInRange(0, position.x - 1);
		break;
	}

	return GetTileAtPosition(targetTileCoords);
}

Tile* Map::FindRandomTileAdjacent(const IntVector2& position)
{
	Tile* tileAtPosition = GetTileAtPosition(position);
	int randomDirInt = GetRandomIntInRange(IntVector2(0, 3));
	switch (randomDirInt)
	{
	case 0: //north
		return GetTileToNorth(tileAtPosition);
	case 1: //east
		return GetTileToEast(tileAtPosition);
	case 2: //south
		return GetTileToSouth(tileAtPosition);
	default: //west
		return GetTileToWest(tileAtPosition);
	}
}

Tile* Map::FindFirstNonSolidTile()
{
	std::vector<Tile>::iterator tileIter;
	for (tileIter = m_tiles.begin(); tileIter != m_tiles.end(); ++tileIter)
	{
		if (tileIter->m_tileDefition->m_isSolid == false)
		{
			return &(*tileIter);
		}
	}
	return nullptr;
}

Tile* Map::FindRandomPassableTileOfType(std::string tileType)
{
	TileDefinition *tileDefToFind = nullptr;
	if (!tileType.empty())
	{
		tileDefToFind = TileDefinition::s_registry[tileType];
	}

	while (true)
	{
		IntVector2 position;
		position.x = GetRandomIntInRange(1, m_dimensions.x - 1);
		position.y = GetRandomIntInRange(1, m_dimensions.y - 1);
		Tile *tile = GetTileAtPosition(position);
		if (tile->IsPassable() && (tileDefToFind == nullptr || tile->m_tileDefition == tileDefToFind))
		{
			return tile;
		}
	}
}

Tile* Map::FindRandomTile()
{
	while (true)
	{
		IntVector2 position;
		position.x = GetRandomIntInRange(1, m_dimensions.x - 1);
		position.y = GetRandomIntInRange(1, m_dimensions.y - 1);
		return GetTileAtPosition(position);
	}
}

Tile* Map::GetTileAtPosition(const IntVector2& position)
{
	if (position.x >= m_dimensions.x || position.y >= m_dimensions.y || position.x < 0 || position.y < 0)
		return nullptr;
	int indexPos = (position.y * m_dimensions.x) + position.x;
	if (indexPos > -1 && indexPos < (int)m_tiles.size())
		return &m_tiles[indexPos];
	return nullptr;
}

Tile* Map::FindClosestTileWithoutFeature(const Vector2& position)
{
	Tile *closestTile = nullptr;
	float closestTileDistance = FLT_MAX;
	
	IntVector2 flooredPosition = IntVector2(floor(position.x), floor(position.y));
	Tile* tileAtPosition = GetTileAtPosition(flooredPosition);
	for (int xCoord = flooredPosition.x - 1; xCoord <= flooredPosition.x + 1; xCoord++)
	{
		for (int yCoord = flooredPosition.y - 1; yCoord <= flooredPosition.y + 1; yCoord++)
		{
			IntVector2 tileCoords = IntVector2(xCoord, yCoord);
			Tile* currentTile = GetTileAtPosition(tileCoords);
			if (currentTile->m_feature == nullptr)
			{
				Vector2 tilePos = Vector2(tileCoords) + Vector2(0.5f, 0.5f);
				float distanceToTile = tilePos.CalcDistanceToVector(position);
				if (distanceToTile < closestTileDistance)
				{
					closestTileDistance = distanceToTile;
					closestTile = tileAtPosition;
				}
			}
		}
	}

	return closestTile;
}

int Map::GetTileIndexFromCoords(const IntVector2& position)
{
	return (position.y * m_dimensions.x) + position.x;
}

IntVector2 Map::GetTileCoordsFromIndex(int index)
{
	IntVector2 tileCoords;
	tileCoords.x = index % m_dimensions.x;
	tileCoords.y = (index / m_dimensions.x);
	return tileCoords;
}

Tile* Map::GetTileToNorth(Tile* currentTile)
{
	return GetTileAtPosition(IntVector2(currentTile->m_mapPosition.x, currentTile->m_mapPosition.y + 1));
}

Tile* Map::GetTileToSouth(Tile* currentTile)
{
	return GetTileAtPosition(IntVector2(currentTile->m_mapPosition.x, currentTile->m_mapPosition.y - 1));
}

Tile* Map::GetTileToWest(Tile* currentTile)
{
	return GetTileAtPosition(IntVector2(currentTile->m_mapPosition.x - 1, currentTile->m_mapPosition.y));
}

Tile* Map::GetTileToEast(Tile* currentTile)
{
	return GetTileAtPosition(IntVector2(currentTile->m_mapPosition.x + 1, currentTile->m_mapPosition.y));
}

Tile* Map::GetTileToNorthEast(Tile *currentTile)
{
	return GetTileAtPosition(IntVector2(currentTile->m_mapPosition.x + 1, currentTile->m_mapPosition.y + 1));
}

Tile* Map::GetTileToNorthWest(Tile *currentTile)
{
	return GetTileAtPosition(IntVector2(currentTile->m_mapPosition.x - 1, currentTile->m_mapPosition.y + 1));
}

Tile* Map::GetTileToSouthEast(Tile *currentTile)
{
	return GetTileAtPosition(IntVector2(currentTile->m_mapPosition.x + 1, currentTile->m_mapPosition.y - 1));
}

Tile* Map::GetTileToSouthWest(Tile *currentTile)
{
	return GetTileAtPosition(IntVector2(currentTile->m_mapPosition.x - 1, currentTile->m_mapPosition.y - 1));
}

int Map::CalcNumNeighborsOfType(Tile *currentTile, TileDefinition *tileDef)
{
	int numNeighborsOfType = 0;
	Tile *neighborTile = GetTileToNorth(currentTile);
	if (neighborTile != nullptr && neighborTile->m_tileDefition == tileDef)	numNeighborsOfType++;
	neighborTile = GetTileToSouth(currentTile);
	if (neighborTile != nullptr && neighborTile->m_tileDefition == tileDef)	numNeighborsOfType++;
	neighborTile = GetTileToEast(currentTile);
	if (neighborTile != nullptr && neighborTile->m_tileDefition == tileDef)	numNeighborsOfType++;
	neighborTile = GetTileToWest(currentTile);
	if (neighborTile != nullptr && neighborTile->m_tileDefition == tileDef)	numNeighborsOfType++;
	neighborTile = GetTileToNorthEast(currentTile);
	if (neighborTile != nullptr && neighborTile->m_tileDefition == tileDef)	numNeighborsOfType++;
	neighborTile = GetTileToNorthWest(currentTile);
	if (neighborTile != nullptr && neighborTile->m_tileDefition == tileDef)	numNeighborsOfType++;
	neighborTile = GetTileToSouthEast(currentTile);
	if (neighborTile != nullptr && neighborTile->m_tileDefition == tileDef)	numNeighborsOfType++;
	neighborTile = GetTileToSouthWest(currentTile);
	if (neighborTile != nullptr && neighborTile->m_tileDefition == tileDef)	numNeighborsOfType++;

	return numNeighborsOfType;
}

bool Map::IsOutOfBounds(const IntVector2& position)
{
	if (position.x < 0 || position.x > m_dimensions.x || position.y < 0 || position.y > m_dimensions.y)
		return true;
	return false;
}

bool Map::IsTilePositionSolid(const IntVector2& position)
{
	Tile* tile = GetTileAtPosition(position);
	return tile->m_tileDefition->m_isSolid;
}

bool Map::IsTilePositionPassable(const IntVector2& position)
{
	Tile* tile = GetTileAtPosition(position);
	if (tile != nullptr)
		return tile->m_tileDefition->m_isPassable;
	else 
		return false;
}

Vector2 Map::Raycast(const Vector2& start, const Vector2& end, bool canHitOpaque, bool canHitSolid)
{
	Vector2 displacement = end - start;
	Vector2 displacementFraction = displacement * 0.01f;
	Vector2 currentPos = start;
	for (int lineOfSightStep = 0; lineOfSightStep < 100; ++lineOfSightStep)
	{
		currentPos += displacementFraction;
		Tile *tile = GetTileAtPosition(currentPos);
		if (canHitOpaque)
		{
			if (tile->m_tileDefition->m_isOpaque)
				return currentPos;
		}
		if (canHitSolid)
		{
			if (tile->m_tileDefition->m_isSolid)
				return currentPos;
		}
	}
	return currentPos;
}

RaycastResults2D Map::RaycastAW(const Vector2& start, const Vector2& end, bool canHitOpaque, bool canHitSolid)
{
	RaycastResults2D results;
	IntVector2 startTilePos = IntVector2(start);
	Tile *startTile = GetTileAtPosition(startTilePos);
	IntVector2 tilePos = startTilePos;

	if (startTile == nullptr)
	{
		results.didImpact = false;
		return results;
	}

	if ((canHitSolid && startTile->m_tileDefition->m_isSolid) || (canHitOpaque && startTile->m_tileDefition->m_isOpaque))
	{
		if (startTile->m_tileDefition->m_isSolid)
		{
			results.didImpact = true;
			results.impactPosition = start;
			results.impactFraction = 0;
			results.impactTileCoords = tilePos;
			results.impactSurfaceNormal = Vector2(0, 0);
			return results;
		}
	}

	Vector2 rayDisplacement = end - start;
	float tDeltaX = 1 / abs(rayDisplacement.x);
	int tileStepX = 1;
	if (rayDisplacement.x < 0)
		tileStepX = -1;
	int offsetToLeadingEdgeX = (tileStepX + 1) / 2;
	float firstVerticalIntersetionX = (float)(start.x + offsetToLeadingEdgeX);
	float tOfNextXCrossing = abs(firstVerticalIntersetionX - start.x) * tDeltaX;

	float tDeltaY = 1 / abs(rayDisplacement.y);
	int tileStepY = 1;
	if (rayDisplacement.y < 0)
		tileStepY = -1;
	int offsetToLeadingEdgeY = (tileStepY + 1) / 2;
	float firstVerticalIntersetionY = (float)(start.y + offsetToLeadingEdgeY);
	float tOfNextYCrossing = abs(firstVerticalIntersetionY - start.y) * tDeltaY;

	while (true)
	{
		if (tOfNextXCrossing <= tOfNextYCrossing)
		{
			if (tOfNextXCrossing >= 1)
			{
				results.didImpact = false;
				return results;
			}
			tilePos.x += tileStepX;
			Tile *newTile = GetTileAtPosition(tilePos);
			if (newTile == nullptr)
			{
				results.didImpact = false;
				return results;
			}

			if ((canHitSolid && newTile->m_tileDefition->m_isSolid) || (canHitOpaque && newTile->m_tileDefition->m_isOpaque))
			{
				results.didImpact = true;
				results.impactPosition = start + (rayDisplacement * tOfNextXCrossing);
				results.impactFraction = tOfNextXCrossing;
				results.impactTileCoords = tilePos;

				if (tileStepX > 0)
				{
					results.impactSurfaceNormal = Vector2(-1, 0);
				}
				else
				{
					results.impactSurfaceNormal = Vector2(1, 0);
				}
				return results;
			}

			tOfNextXCrossing += tDeltaX;
		}
		else if (tOfNextYCrossing < tOfNextXCrossing)
		{
			if (tOfNextYCrossing >= 1)
			{
				results.didImpact = false;
				return results;
			}
			tilePos.y += tileStepY;
			Tile *newTile = GetTileAtPosition(tilePos);
			if (newTile == nullptr)
			{
				results.didImpact = false;
				return results;
			}

			if ((canHitSolid && newTile->m_tileDefition->m_isSolid) || (canHitOpaque && newTile->m_tileDefition->m_isOpaque))
			{
				results.didImpact = true;
				results.impactPosition = start + (rayDisplacement * tOfNextYCrossing);
				results.impactFraction = tOfNextYCrossing;
				results.impactTileCoords = tilePos;

				if (tileStepY > 0)
				{
					results.impactSurfaceNormal = Vector2(0, -1);
				}
				else
				{
					results.impactSurfaceNormal = Vector2(0, 1);
				}
				return results;
			}

			tOfNextYCrossing += tDeltaY;
		}
	}
}

float Map::GetLongestCharacterNameSize(float scale)
{
	float maxNameLength = FLT_MIN;
	for (int characterIndex = 0; characterIndex < (int)m_characters.size(); characterIndex++)
	{
		Character* currentCharacter = m_characters[characterIndex];
		float nameLength = g_theSimpleRenderer->m_font->CalculateTextWidth(currentCharacter->m_name.c_str(), scale);
		if (nameLength > maxNameLength)
		{
			maxNameLength = nameLength;
		}
	}
	return maxNameLength;
}

Character* Map::FindNearestActorOfType(const IntVector2& position, std::string typeName)
{
	Character *closestCharacter = nullptr;
	int closestCharacterDistance = INT_MAX;
	for (int characterIndex = 0; characterIndex < (int)m_characters.size(); characterIndex++)
	{
		if (m_characters[characterIndex]->m_name == typeName)
		{
			int manhattanDistance = CalculateManhattanDistance(position, m_characters[characterIndex]->m_owningTile->m_mapPosition);
			if (manhattanDistance < closestCharacterDistance && manhattanDistance != 0)
			{
				closestCharacterDistance = manhattanDistance;
				closestCharacter = m_characters[characterIndex];
			}
		}
	}
	return closestCharacter;
}

Character* Map::FindNearestActorNotOfType(const IntVector2& position, std::string typeName)
{
	Character *closestCharacter = nullptr;
	int closestCharacterDistance = INT_MAX;
	for (int characterIndex = 0; characterIndex < (int)m_characters.size(); characterIndex++)
	{
		if (m_characters[characterIndex]->m_name != typeName)
		{
			int manhattanDistance = CalculateManhattanDistance(position, m_characters[characterIndex]->m_owningTile->m_mapPosition);
			if (manhattanDistance < closestCharacterDistance && manhattanDistance != 0)
			{
				closestCharacterDistance = manhattanDistance;
				closestCharacter = m_characters[characterIndex];
			}
		}
	}
	return closestCharacter;
}

Character* Map::FindNearestActorOfFaction(const IntVector2& position, std::string factionName)
{
	Character *closestCharacter = nullptr;
	int closestCharacterDistance = INT_MAX;
	for (int characterIndex = 0; characterIndex < (int)m_characters.size(); characterIndex++)
	{
		if (m_characters[characterIndex]->m_faction == factionName)
		{
			int manhattanDistance = CalculateManhattanDistance(position, m_characters[characterIndex]->m_owningTile->m_mapPosition);
			if (manhattanDistance < closestCharacterDistance && manhattanDistance != 0)
			{
				closestCharacterDistance = manhattanDistance;
				closestCharacter = m_characters[characterIndex];
			}
		}
	}
	return closestCharacter;
}

Character* Map::FindNearestActorOfFactionNotSelf(const IntVector2& position, std::string factionName, Character* self)
{
	Character *closestCharacter = nullptr;
	int closestCharacterDistance = INT_MAX;
	for (int characterIndex = 0; characterIndex < (int)m_characters.size(); characterIndex++)
	{
		if (m_characters[characterIndex] == self || m_characters[characterIndex] == nullptr)
			continue;

		if (m_characters[characterIndex]->m_faction == factionName)
		{
			int manhattanDistance = CalculateManhattanDistance(position, m_characters[characterIndex]->m_owningTile->m_mapPosition);
			if (manhattanDistance < closestCharacterDistance && manhattanDistance != 0)
			{
				closestCharacterDistance = manhattanDistance;
				closestCharacter = m_characters[characterIndex];
			}
		}
	}
	return closestCharacter;
}

Character* Map::FindNearestActorNotOfFaction(const IntVector2& position, std::string factionName)
{
	Character *closestCharacter = nullptr;
	int closestCharacterDistance = INT_MAX;
	for (int characterIndex = 0; characterIndex < (int)m_characters.size(); characterIndex++)
	{
		if (m_characters[characterIndex]->m_faction != factionName)
		{
			int manhattanDistance = CalculateManhattanDistance(position, m_characters[characterIndex]->m_bounds.m_center);
			if (manhattanDistance < closestCharacterDistance && manhattanDistance != 0)
			{
				closestCharacterDistance = manhattanDistance;
				closestCharacter = m_characters[characterIndex];
			}
		}
	}
	return closestCharacter;
}

Character* Map::FindNearestActorNotSelf(const IntVector2& position, Character* self)
{
	Character *closestCharacter = nullptr;
	int closestCharacterDistance = INT_MAX;
	for (int characterIndex = 0; characterIndex < (int)m_characters.size(); characterIndex++)
	{
		Character* currentCharacter = m_characters[characterIndex];
		if (currentCharacter != self)
		{
			int manhattanDistance = CalculateManhattanDistance(position, currentCharacter->m_bounds.m_center);
			if (manhattanDistance < closestCharacterDistance && manhattanDistance != 0)
			{
				closestCharacterDistance = manhattanDistance;
				closestCharacter = currentCharacter;
			}
		}
	}
	return closestCharacter;
}

bool Map::DoesActorNotOfFactionExistNotSelf(const std::string& factionName, Character* self)
{
	for (int characterIndex = 0; characterIndex < (int)m_characters.size(); characterIndex++)
	{
		Character* currentCharacter = m_characters[characterIndex];
		if (currentCharacter != nullptr && currentCharacter != self && currentCharacter->m_faction != factionName)
		{
			return true;
		}
	}
	return false;
}

bool Map::DoesOtherActorExist(Character* self)
{
	for (int characterIndex = 0; characterIndex < (int)m_characters.size(); characterIndex++)
	{
		Character* currentCharacter = m_characters[characterIndex];
		if (currentCharacter != nullptr && currentCharacter != self)
		{
			return true;
		}
	}
	return false;
}

Feature* Map::FindNearestFeatureOfType(const IntVector2& position, const std::string& featureType)
{
	Feature* nearestFeature = nullptr;
	int distanceToNearestFeature = INT_MAX;

	std::vector<Feature*>::iterator featureIter;
	for (featureIter = m_features.begin(); featureIter != m_features.end(); ++featureIter)
	{
		Feature* currentFeature = *featureIter;
		if (currentFeature->m_name == featureType)
		{
			int manhattanDistance = CalculateManhattanDistance(position, currentFeature->m_owningTile->m_mapPosition);
			if (manhattanDistance < distanceToNearestFeature)
			{
				distanceToNearestFeature = manhattanDistance;
				nearestFeature = currentFeature;
			}
		}
	}

	return nearestFeature;
}

Feature* Map::FindNearestDestroyableFeature(const IntVector2& position)
{
	Feature* nearestFeature = nullptr;
	int distanceToNearestFeature = INT_MAX;

	std::vector<Feature*>::iterator featureIter;
	for (featureIter = m_features.begin(); featureIter != m_features.end(); ++featureIter)
	{
		Feature* currentFeature = *featureIter;
		if (currentFeature->m_isDestoyable)
		{
			int manhattanDistance = CalculateManhattanDistance(position, currentFeature->m_owningTile->m_mapPosition);
			if (manhattanDistance < distanceToNearestFeature)
			{
				distanceToNearestFeature = manhattanDistance;
				nearestFeature = currentFeature;
			}
		}
	}

	return nearestFeature;
}

Tile * Map::FindNearestTileOfType(const IntVector2& position, std::string tileType)
{
	Tile *closestTile = nullptr;
	int closestTileDistance = INT_MAX;
	for (int tileIndex = 0; tileIndex < (int)m_characters.size(); tileIndex++)
	{
		if (m_tiles[tileIndex].m_tileDefition->m_name == tileType)
		{
			int manhattanDistance = CalculateManhattanDistance(position, m_tiles[tileIndex].m_mapPosition);
			if (manhattanDistance < closestTileDistance && manhattanDistance != 0)
			{
				closestTileDistance = manhattanDistance;
				closestTile = &m_tiles[tileIndex];
			}
		}
	}
	return closestTile;
}

Tile * Map::FindNearestTileNotOfType(const IntVector2& position, std::string tileType)
{
	Tile *closestTile = nullptr;
	int closestTileDistance = INT_MAX;
	for (int tileIndex = 0; tileIndex < (int)m_characters.size(); tileIndex++)
	{
		if (m_tiles[tileIndex].m_tileDefition->m_name != tileType)
		{
			int manhattanDistance = CalculateManhattanDistance(position, m_tiles[tileIndex].m_mapPosition);
			if (manhattanDistance < closestTileDistance && manhattanDistance != 0)
			{
				closestTileDistance = manhattanDistance;
				closestTile = &m_tiles[tileIndex];
			}
		}
	}
	return closestTile;
}

bool Map::AreCharactersAdjacent(Character* firstCharacter, Character* secondCharacter)
{
	if (secondCharacter == nullptr)
	{
		return false;
	}
	int manhattanDistance = CalculateManhattanDistance(firstCharacter->GetMapCoords(), secondCharacter->GetMapCoords());
	return manhattanDistance <= 1;
}

Path Map::GeneratePath(IntVector2 start, IntVector2 end, Character *character /*= nullptr*/)
{
	Path path;
	StartSteppedPath(start, end, character);

	while (!ContinueSteppedPath(path)) {}
	return path;
}

void Map::StartSteppedPath(IntVector2 start, IntVector2 end, Character *character /*= nullptr*/)
{
	if (m_currentPath != nullptr)
		delete m_currentPath;
	m_currentPath = new PathGenerator(start, end, *this, character);
}

bool Map::ContinueSteppedPath(Path& outPath)
{
	m_currentPath->SelectAndCloseBestOpenTile();
	if (m_currentPath->IsCurrentTileGoal())
	{
		m_currentPath->CreatePathToGoal(outPath);
		return true;
	}

	m_currentPath->AddNeighborsToOpenList();
	return false;
}

void Map::GenerateDijkstraMapForAllCharacters(Character* exceptionCharacter)
{
	DijkstraMapGenerator* mapGenerator = new DijkstraMapGenerator(*this);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_characters.begin(); characterIter != m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;
		if (currentCharacter != exceptionCharacter)
		{
			mapGenerator->AddTileToOpenList(currentCharacter->GetMapCoords(), 0.f);
		}
	}

	while (!mapGenerator->m_openList.empty())
	{
		mapGenerator->SelectAndCloseBestOpenTileForMap();
		mapGenerator->AddNeighborsToOpenList();
	}
}

void Map::GenerateDijkstraMapForAllCharactersOfFaction(const std::string& factionName, Character* exceptionCharacter /*= nullptr*/)
{
	DijkstraMapGenerator* mapGenerator = new DijkstraMapGenerator(*this);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_characters.begin(); characterIter != m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;
		if (currentCharacter != exceptionCharacter && currentCharacter->m_faction == factionName)
		{
			mapGenerator->AddTileToOpenList(currentCharacter->GetMapCoords(), 0.f);
		}
	}

	while (!mapGenerator->m_openList.empty())
	{
		mapGenerator->SelectAndCloseBestOpenTileForMap();
		mapGenerator->AddNeighborsToOpenList();
	}
}

void Map::GenerateDijkstraMapForAllCharactersNotOfFaction(const std::string& factionName, Character* exceptionCharacter /*= nullptr*/)
{
	DijkstraMapGenerator* mapGenerator = new DijkstraMapGenerator(*this);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_characters.begin(); characterIter != m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;
		if (currentCharacter != exceptionCharacter && currentCharacter->m_faction != factionName)
		{
			mapGenerator->AddTileToOpenList(currentCharacter->GetMapCoords(), 0.f);
		}
	}

	while (!mapGenerator->m_openList.empty())
	{
		mapGenerator->SelectAndCloseBestOpenTileForMap();
		mapGenerator->AddNeighborsToOpenList();
	}
}

void Map::GenerateDijkstraMapForCharacter(Character* character)
{
	DijkstraMapGenerator* mapGenerator = new DijkstraMapGenerator(*this);
	mapGenerator->AddTileToOpenList(character->GetMapCoords(), 0.f);

	while (!mapGenerator->m_openList.empty())
	{
		mapGenerator->SelectAndCloseBestOpenTileForMap();
		mapGenerator->AddNeighborsToOpenList();
	}
}

Tile* Map::GetLowestScoreDijkstraMapTile(const IntVector2& startPos)
{
	Tile* tile = GetTileAtPosition(startPos);
	Tile* eastTile = GetTileToEast(tile);
	Tile* westTile = GetTileToWest(tile);
	Tile* southTile = GetTileToSouth(tile);
	Tile* northTile = GetTileToNorth(tile);
	Tile* lowestTile = tile;

	if (eastTile != nullptr && eastTile->m_dijkstraMapFScore <= lowestTile->m_dijkstraMapFScore && eastTile->IsPassable())
	{
		lowestTile = westTile;
	}
	if (westTile != nullptr && westTile->m_dijkstraMapFScore <= lowestTile->m_dijkstraMapFScore && westTile->IsPassable())
	{
		lowestTile = westTile;
	}
	if (southTile != nullptr && southTile->m_dijkstraMapFScore <= lowestTile->m_dijkstraMapFScore && southTile->IsPassable())
	{
		lowestTile = southTile;
	}
	if (northTile != nullptr && northTile->m_dijkstraMapFScore <= lowestTile->m_dijkstraMapFScore && northTile->IsPassable())
	{
		lowestTile = northTile;
	}

	return lowestTile;
}

Tile* Map::GetHighestScoreDijkstraMapTile(const IntVector2& startPos)
{
	Tile* tile = GetTileAtPosition(startPos);
	Tile* eastTile = GetTileToEast(tile);
	Tile* westTile = GetTileToWest(tile);
	Tile* southTile = GetTileToSouth(tile);
	Tile* northTile = GetTileToNorth(tile);
	Tile* highestTile = tile;

	if (eastTile != nullptr && eastTile->m_dijkstraMapFScore >= highestTile->m_dijkstraMapFScore && eastTile->IsPassable())
	{
		highestTile = eastTile;
	}
	if (westTile != nullptr && westTile->m_dijkstraMapFScore >= highestTile->m_dijkstraMapFScore && westTile->IsPassable())
	{
		highestTile = westTile;
	}
	if (southTile != nullptr && southTile->m_dijkstraMapFScore >= highestTile->m_dijkstraMapFScore && southTile->IsPassable())
	{
		highestTile = southTile;
	}
	if (northTile != nullptr && northTile->m_dijkstraMapFScore >= highestTile->m_dijkstraMapFScore && northTile->IsPassable())
	{
		highestTile = northTile;
	}

	return highestTile;
}

void Map::ClearLineOfSights()
{
	for (std::vector<Character*>::iterator characterIter = m_characters.begin(); characterIter != m_characters.end(); ++characterIter)
	{
		(*characterIter)->m_charactersInSight.clear();
	}
}

void Map::DetermineLineOfSights()
{
	for (std::vector<Character*>::iterator characterIter = m_characters.begin(); characterIter != m_characters.end(); ++characterIter)
	{
		Character *currentCharacter = *characterIter;
		for (std::vector<Character*>::iterator nextCharacterIter = characterIter + 1; nextCharacterIter != m_characters.end(); ++nextCharacterIter)
		{
			Character *nextCharacter = *nextCharacterIter;
			if (currentCharacter->HasLineOfSight(nextCharacter))
			{
				currentCharacter->m_charactersInSight.push_back(nextCharacter);
				nextCharacter->m_charactersInSight.push_back(currentCharacter);

				currentCharacter->m_currentAimTarget = nextCharacter;
				nextCharacter->m_currentAimTarget = currentCharacter;
			}
		}
	}
}

int Map::GetColorIndexFromColorMap(const RGBA& color)
{
	for (int index = 0; index < (int)m_tileColors.size(); index++)
	{
		if (m_tileColors[index] == color)
			return index;
	}
	return -1;
}
