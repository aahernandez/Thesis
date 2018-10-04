#include "Game/Character.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/Behavior.hpp"
#include "Game/PathGenerator.hpp"
#include "Game/FeatureBuilder.hpp"
#include "Game/AffectiveArchitecture/SocialRelation.hpp"
#include "Engine/RHI/Font.hpp"
#include "Engine/Profiling/Profiler.hpp"
#include "Engine/Core/Log.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/RHI/SimpleRenderer.hpp"
#include "Engine/Math/Math2D.hpp"

const float HUNGER_DECAY_RATE = 0.01f;
const float HUNGRY_THRESHOLD = 0.5f;

Character::Character(std::string name)
	: Entity(name)
	, m_currentBehavior(nullptr)
	, m_currentAimTarget(nullptr)
	, m_speed(1.f)
	, m_hungerFulfillment(GetRandomFloatZeroToOne())
{
	m_owningTile = nullptr;
	m_owningMap = nullptr;
	m_name = name;
	Initialize();
}

Character::~Character()
{
	SpawnCorpse();
	m_owningTile->m_character = nullptr;
	m_owningTile = nullptr;
}

void Character::Initialize()
{
}

void Character::Update(float deltaSeconds)
{
	UpdateNeeds(deltaSeconds);
	UpdateBehavior(deltaSeconds);
}

void Character::UpdateNeeds(float deltaSeconds)
{
	m_hungerFulfillment -= deltaSeconds * HUNGER_DECAY_RATE;

	if (m_hungerFulfillment < HUNGRY_THRESHOLD)
	{
		GenerateHungerEvent();
	}
}

void Character::Act(float deltaSeconds)
{
	CalcBehaviorUtility();
	m_currentBehavior->Update(deltaSeconds);
	m_currentBehavior->Act(deltaSeconds, this);
}

void Character::GenerateHungerEvent()
{
	Action* eventAction = Action::s_actionRegistry["IsHungry"];
	Event* hungerEvent = new Event(this, eventAction, this);
	Event::AddEvent(this, hungerEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_owningMap->m_characters.begin(); characterIter != m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != this && currentCharacter->HasLineOfSight(this))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(this);
			Event::AddEvent(currentCharacter, hungerEvent, certaintyDegree);
		}
	}
}

void Character::PutSelfOnNewTile(Tile *newTile)
{
	m_owningTile->m_character = nullptr;
	m_owningTile = newTile;
	m_owningTile->m_character = this;
	m_owningTile->InteractWithCharacter();
}

void Character::MoveTowardsNewTile(float deltaSeconds, Tile *newTile)
{
	if (newTile == nullptr)
		return;

	if (newTile->IsPassable())
	{
		Vector2 directionToMove = newTile->GetTileCenter() - m_bounds.m_center;
		directionToMove.Normalize();
		m_bounds.m_center += directionToMove * deltaSeconds * m_speed;
	}
	else
	{
		AttackTargetTile(newTile);
	}
}

void Character::AttackTargetTile(Tile *newTile)
{
	m_owningMap->ResolveAttack(this, newTile);
}

bool Character::IsNorthTileSolid()
{
	Tile *northTile = m_owningMap->GetTileToNorth(m_owningTile);
	return northTile->IsSolid();
}

bool Character::IsSouthTileSolid()
{
	Tile *southTile = m_owningMap->GetTileToSouth(m_owningTile);
	return southTile->IsSolid();
}

bool Character::IsWestTileSolid()
{
	Tile *westTile = m_owningMap->GetTileToWest(m_owningTile);
	return westTile->IsSolid();
}

bool Character::IsEastTileSolid()
{
	Tile *eastTile = m_owningMap->GetTileToEast(m_owningTile);
	return eastTile->IsSolid();
}

bool Character::IsOnTile(Tile* tileToCheck)
{
	return m_bounds.m_center.NearlyEquals(tileToCheck->GetTileCenter(), 0.1f);
}

void Character::FillLogWithDetails(Log *log)
{
	log->StoreTextInLog(Stringf("%s (%s%c[-])", m_name.c_str(), m_glyphColor.GetAsString().c_str(), m_glyph), RGBA::WHITE);
	RGBA healthHighlightColor = RGBA::GREEN_LIGHT;
	std::string healthHighlightString = healthHighlightColor.GetAsString();
	log->StoreTextInLog(Stringf("  -Health: %s%i[-]/%s%i", healthHighlightString.c_str(), m_currentHealth, healthHighlightString.c_str(), m_maxHealth), RGBA::WHITE, SUBTITLE);
	log->StoreTextInLog(Stringf("  -Behavior: %s", m_currentBehavior->m_name.c_str()), RGBA::WHITE, SUBTITLE);
	log->StoreTextInLog(Stringf("  -Faction: %s", m_faction.c_str()), RGBA::WHITE, SUBTITLE);
}

void Character::CalcBehaviorUtility()
{
	float currentUtility = FLT_MIN;
	if (m_currentBehavior != nullptr)
		currentUtility = m_currentBehavior->CalcUtility(this, true);

	bool wasBehaviorChanged = false;
	Behavior *previousBehavior = m_currentBehavior;
	std::vector<Behavior*>::iterator behaviorIter;
	for (behaviorIter = m_behaviors.begin(); behaviorIter != m_behaviors.end(); ++behaviorIter)
	{
		float behaviorUtility = (*behaviorIter)->CalcUtility(this, false);
		if (behaviorUtility > currentUtility)
		{
			m_currentBehavior = *behaviorIter;
			currentUtility = behaviorUtility;
			wasBehaviorChanged = true;
		}
	}

	if (m_currentBehavior != previousBehavior)
	{
		if (previousBehavior != nullptr)
		{
			previousBehavior->OnEnd(this);
		}
		m_currentBehavior->Init(this);
	}
}

void Character::TakeDirectDamage(int damage)
{
	if (damage > 0)
		m_currentHealth -= damage;

	m_damageLog->EraseLogAtIndex(0);
	m_damageLog->StoreTextInLog(Stringf("%i", damage), RGBA::RED, PARAGRAPH);
	m_logPosition = GetMapCoords();
}

void Character::TakeDamage(int damage)
{
	int defense = m_stats.CalcDefense(m_stats);
	int newDamage = damage - defense;
	if (newDamage > 0)
		m_currentHealth -= newDamage;

	m_logPosition = GetMapCoords();
}

bool Character::AreFactionsSame(std::string factionName)
{
	return m_faction.compare(factionName) == 0;
}

float Character::GetTileGCost(std::string tileName)
{
	return 0.f;
}

void Character::GenerateAndSetRandomPath()
{
	m_target = m_owningMap->FindRandomPassableTileOfType()->GetTileCenter();
	GeneratePathToTarget();
}

void Character::GenerateAndSetRandomCardinalPath()
{
	m_target = m_owningMap->FindRandomTileInCardinalDirection(GetMapCoords())->m_mapPosition;
	GeneratePathToTarget();
}

void Character::GeneratePathToTarget()
{
	ClearPath();
	m_path = m_owningMap->GeneratePath(GetMapCoords(), m_target, this);
	m_path.erase(m_path.end() - 1);
}

void Character::GenerateAndSetRandomPathWithinRadius(float radius)
{
	m_target = GetRandomPointOnDiscEdge(Disc2D(m_bounds.m_center, radius));	
	GeneratePathToTarget();
}

bool Character::IsOtherCharacterInPath()
{
	for (Path::iterator pathIter = m_path.begin(); pathIter != m_path.end(); ++pathIter)
	{
		Tile* currentPathTile = *pathIter;
		if (currentPathTile->HasCharacter())
			return true;
	}
	return false;
}

void Character::FollowPath(float deltaSeconds)
{
	if (!m_path.empty())
	{
		Tile *nextTileInPath = *(m_path.end() - 1);
		MoveTowardsNewTile(deltaSeconds, nextTileInPath);
		if (IsOnTile(nextTileInPath))
		{
			PutSelfOnNewTile(nextTileInPath);
			m_path.erase(m_path.end() - 1);
		}
	}
}

void Character::ClearPath()
{
	m_path.clear();
}

bool Character::HasPath()
{
	return !m_path.empty();
}

Character* Character::GetMostLikedCharacter()
{
	Character* mostLikedCharacter = nullptr;
	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_owningMap->m_characters.begin(); characterIter != m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;
		if (currentCharacter != this && (mostLikedCharacter == nullptr || m_socialRelations[currentCharacter->m_name]->m_liking > m_socialRelations[mostLikedCharacter->m_name]->m_liking))
		{
			mostLikedCharacter = currentCharacter;
		}
	}

	return mostLikedCharacter;
}

Character* Character::GetMostLikedCharacterOfSameFaction()
{
	Character* mostLikedCharacter = nullptr;
	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_owningMap->m_characters.begin(); characterIter != m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;
		if (currentCharacter != this && currentCharacter->m_faction == m_faction
			&& (mostLikedCharacter == nullptr || m_socialRelations[currentCharacter->m_name]->m_liking > m_socialRelations[mostLikedCharacter->m_name]->m_liking))
		{
			mostLikedCharacter = currentCharacter;
		}
	}

	return mostLikedCharacter;
}

Character* Character::GetLeastLikedCharacterOfSameFaction()
{
	Character* mostLikedCharacter = nullptr;
	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_owningMap->m_characters.begin(); characterIter != m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;
		if (currentCharacter != this && currentCharacter->m_faction == m_faction
			&& (mostLikedCharacter == nullptr || m_socialRelations[currentCharacter->m_name]->m_liking < m_socialRelations[mostLikedCharacter->m_name]->m_liking))
		{
			mostLikedCharacter = currentCharacter;
		}
	}

	return mostLikedCharacter;
}

Behavior* Character::GetBehaviorWithName(std::string behaviorName)
{
	std::vector<Behavior*>::iterator behaviorIter;
	for (behaviorIter = m_behaviors.begin(); behaviorIter != m_behaviors.end(); ++behaviorIter)
	{
		Behavior* currentBehavior = *behaviorIter;
		if (currentBehavior->m_name == behaviorName)
			return currentBehavior;
	}
	return nullptr;
}

void Character::DebugRender() const
{
	if (m_currentBehavior != nullptr)
		PathGenerator::RenderPath(m_path);

 	for (int characterCount = 0; characterCount < (int)m_charactersInSight.size(); characterCount++)
	{
		IntVector2 position = m_charactersInSight[characterCount]->m_owningTile->m_mapPosition;
		g_theSimpleRenderer->DrawDebugQuad2D(AABB2D(Vector2(position.x + 0.2f, position.y + 0.2f), Vector2(position.x + 0.8f, position.y + 0.8f)), 0.1f, RGBA::RED, RGBA::RED);
	}
}

void Character::RemoveSelfFromOwningTile()
{
	m_owningTile->m_character = nullptr;
}

IntVector2 Character::GetMapCoords()
{
	return m_owningTile->m_mapPosition;
}

void Character::UpdateBehavior(float deltaSeconds)
{
	Act(deltaSeconds);
}

void Character::UpdateEntity(float deltaSeconds)
{
	Entity::Update(deltaSeconds);
}

void Character::Render() const
{
	float characterQuadSubtractor = 0.3f;
	float saturationQuadSubtractor = 0.35f;

	if (AreAnySaturationThresholdsPassed())
	{
		g_theSimpleRenderer->DrawQuad2D(m_bounds.m_center - Vector2(saturationQuadSubtractor, saturationQuadSubtractor), m_bounds.m_center + Vector2(saturationQuadSubtractor, saturationQuadSubtractor), RGBA::YELLOW);
	}
	g_theSimpleRenderer->DrawQuad2D(m_bounds.m_center - Vector2(characterQuadSubtractor, characterQuadSubtractor), m_bounds.m_center + Vector2(characterQuadSubtractor, characterQuadSubtractor), m_glyphColor);
	BasicEmotion highestEmotion = GetHighestEmotion();
	g_theSimpleRenderer->DrawQuad2DTextured(m_bounds.m_center - Vector2(0.3f, 0.3f), m_bounds.m_center + Vector2(0.3f, 0.3f), Game::s_gameInstance->m_faceTextures[highestEmotion]);
	
	if (Game::s_gameInstance->m_selectedCharacter == this)
	{
		g_theSimpleRenderer->DrawCircleHollow2D(m_bounds.m_center, m_bounds.m_radius, 0.03f, 75, RGBA::PURPLE);
	}
	else if (Game::s_gameInstance->m_hoveredCharacter == this)
	{
		g_theSimpleRenderer->DrawCircleHollow2D(m_bounds.m_center, m_bounds.m_radius, 0.03f, 75, m_glyphColor);
	}

	std::string behaviorString = m_currentBehavior->m_name;
	if (m_currentBehavior->m_patientCharacter != nullptr)
	{
		behaviorString += Stringf(" %s", m_currentBehavior->m_patientCharacter->m_name.c_str());
	}

	AABB2D textBounds = AABB2D(m_bounds.m_center, m_bounds.m_radius * 2.f, m_bounds.m_radius);
	textBounds.Scale(Vector2(1.f, 0.5f));
	textBounds.Translate(Vector2(0.f, -textBounds.CalcSize().y * 1.5f));
	g_theSimpleRenderer->DrawText2DCenteredAndScaledToFitWithinBounds(behaviorString.c_str(), textBounds);
	textBounds.Translate(Vector2(0.f, textBounds.CalcSize().y * 3.f));
	g_theSimpleRenderer->DrawText2DCenteredAndScaledToFitWithinBounds(m_name.c_str(), textBounds);
}

int Character::CalcDamage()
{
	return m_stats.CalcStrengthAttack();
}

bool Character::HasLineOfSight(Character *character)
{
	if (character == nullptr)
	{
		return false;
	}
	Vector2 entityMapCoords = GetMapCoords();
	Vector2 characterMapCoords = character->GetMapCoords();

	for (float changeOfEntityX = 0.f; changeOfEntityX <= 1.f; changeOfEntityX += 0.5f)
	{
		for (float changeOfEntityY = 0.f; changeOfEntityY <= 1.f; changeOfEntityY += 0.5f)
		{
			for (float changeOfCharX = 0.f; changeOfCharX <= 1.f; changeOfCharX += 0.5f)
			{
				for (float changeOfCharY = 0.f; changeOfCharY <= 1.f; changeOfCharY += 0.5f)
				{
					Vector2 newEntityPos = Vector2(entityMapCoords.x + changeOfEntityX, entityMapCoords.y + changeOfEntityY);
					Vector2 newCharacterPos = Vector2(characterMapCoords.x + changeOfCharX, characterMapCoords.y + changeOfCharY);
					RaycastResults2D raycastResults = m_owningMap->RaycastAW(newEntityPos, newCharacterPos, false, true);

					if (!raycastResults.didImpact)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

void Character::SpawnCorpse()
{
	Feature* corpseFeature = FeatureBuilder::BuildFeature("Corpse", m_owningMap);
	m_owningMap->m_features.push_back(corpseFeature);
	Tile* tileToPlaceCorpse = m_owningMap->FindClosestTileWithoutFeature(m_bounds.m_center);
	corpseFeature->m_owningTile->m_feature = nullptr;
	corpseFeature->m_owningTile = tileToPlaceCorpse;
	tileToPlaceCorpse->m_feature = corpseFeature;
	if (m_faction == "predator")
	{
		corpseFeature->m_glyphColor = RGBA(200, 0, 0);
	}
	else if (m_faction == "prey")
	{
		corpseFeature->m_glyphColor = RGBA(0, 153, 204);
	}
}
