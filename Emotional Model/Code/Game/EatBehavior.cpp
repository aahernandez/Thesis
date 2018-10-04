#include "Game/EatBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

EatBehavior* Behavior::s_eatBehavior = new EatBehavior();

EatBehavior::EatBehavior()
	: Behavior()
{
	m_name = "Eat";
	m_timeRunning = 0.f;
	m_attackBehaviorReference = nullptr;
	m_isCurrentlyConsumingFood = false;
	m_timeTillNextBite = TIME_BETWEEN_EATING_BITES;
}

EatBehavior::EatBehavior(EatBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
	m_attackBehaviorReference = behaviorCopy->m_attackBehaviorReference;
	m_isCurrentlyConsumingFood = behaviorCopy->m_isCurrentlyConsumingFood;
	m_timeTillNextBite = behaviorCopy->m_timeTillNextBite;
}

EatBehavior::~EatBehavior()
{

}

void EatBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;

	std::vector<Behavior*>::iterator behaviorIter;
	for (behaviorIter = character->m_behaviors.begin(); behaviorIter != character->m_behaviors.end(); ++behaviorIter)
	{
		Behavior* currentBehavior = *behaviorIter;
		if (currentBehavior->m_name == "Attack")
		{
			m_attackBehaviorReference = currentBehavior;
			break;
		}
	}

	character->ClearPath();
}

void EatBehavior::OnEnd(Character* character)
{
	character;
	m_patientCharacter = nullptr;
	character->ClearPath();
}

float EatBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = (1.f - character->m_hungerFulfillment) * 2.f;
	}
	else
	{
		m_lastCalculatedUtility = (1.f - character->m_hungerFulfillment) * 2.f;
	}
	return m_lastCalculatedUtility;
}

void EatBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	m_timeTillNextBite -= deltaSeconds;
}

void EatBehavior::Act(float deltaSeconds, Character *character)
{
	m_isCurrentlyConsumingFood = false;

	if (character->m_faction == "prey")
	{ 
		Tile* characterTargetTile = character->m_owningMap->GetTileAtPosition(character->m_target);
		Feature* closestCarrot = characterTargetTile->m_feature;
		if (characterTargetTile->m_feature == nullptr || characterTargetTile->m_feature->m_name != "Carrot" || character->m_owningMap->m_wasFeatureJustAdded)
		{
			closestCarrot = character->m_owningMap->FindNearestFeatureOfType(character->m_owningTile->m_mapPosition, "Carrot");
			if (closestCarrot != nullptr)
			{
				characterTargetTile = closestCarrot->m_owningTile;
				character->m_target = characterTargetTile->m_mapPosition;
			}
			character->ClearPath();
		}

		int distanceToTile = CalculateManhattanDistance(character->m_owningTile->m_mapPosition, characterTargetTile->m_mapPosition);
		if (distanceToTile <= 0)
		{
			m_isCurrentlyConsumingFood = true;
			if (m_timeTillNextBite <= 0.f && closestCarrot != nullptr)
			{
				closestCarrot->m_health--;
				character->m_currentHealth = Clamp<int>(character->m_currentHealth + 1, 0, character->m_maxHealth);
				m_timeTillNextBite = TIME_BETWEEN_EATING_BITES;
				character->m_hungerFulfillment += HUNGER_FULFILLMENT_FROM_BITE;
			}
		}
	}
	else if (character->m_faction == "predator")
	{
		Tile* characterTargetTile = character->m_owningMap->GetTileAtPosition(character->m_target);
		Feature* closestCorpse = characterTargetTile->m_feature;
		if (characterTargetTile->m_feature == nullptr || characterTargetTile->m_feature->m_name != "Corpse" || character->m_owningMap->m_wasFeatureJustAdded)
		{
			closestCorpse = character->m_owningMap->FindNearestFeatureOfType(character->m_owningTile->m_mapPosition, "Corpse");
			if (closestCorpse != nullptr)
			{
				characterTargetTile = closestCorpse->m_owningTile;
				character->m_target = characterTargetTile->m_mapPosition;
				character->ClearPath();
			}
			else
			{
				m_attackBehaviorReference->Init(character);
				m_attackBehaviorReference->Act(deltaSeconds, character);
				GenerateEvents(character, m_patientCharacter);
				return;
			}
		}

		int distanceToTile = CalculateManhattanDistance(character->m_owningTile->m_mapPosition, characterTargetTile->m_mapPosition);
		if (distanceToTile <= 0)
		{
			m_isCurrentlyConsumingFood = true;
			if (m_timeTillNextBite <= 0.f && closestCorpse != nullptr)
			{
				closestCorpse->m_health--;
				character->m_currentHealth = Clamp<int>(character->m_currentHealth + 3, 0, character->m_maxHealth);
				m_timeTillNextBite = TIME_BETWEEN_EATING_BITES;
				character->m_hungerFulfillment += HUNGER_FULFILLMENT_FROM_BITE;
			}
		}
	}

	if (character->m_path.empty())
	{
		character->GeneratePathToTarget();
	}

	character->FollowPath(deltaSeconds);

	GenerateEvents(character, m_patientCharacter);
}

void EatBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr)
		return;

	secondaryCharacter;

	Action* eventAction = Action::s_actionRegistry["IsEating"];
	Event* eatEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, eatEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, eatEvent, certaintyDegree);
		}
	}
}

EatBehavior* EatBehavior::Clone()
{
	return new EatBehavior(*this);
}

