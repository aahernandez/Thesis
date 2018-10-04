#include "Game/TantrumBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

TantrumBehavior* Behavior::s_tantrumBehavior = new TantrumBehavior();

TantrumBehavior::TantrumBehavior()
	: Behavior()
	, m_currentDirection(-1)
	, m_timeTillDamageFeature(TIME_BETWEEN_ATTACKS)
{
	m_name = "Tantrum";
	m_timeRunning = 0.f;
}

TantrumBehavior::TantrumBehavior(TantrumBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
	m_currentDirection = behaviorCopy->m_currentDirection;
	m_speedModifier = behaviorCopy->m_speedModifier;
	m_timeTillDamageFeature = behaviorCopy->m_timeTillDamageFeature;
}

TantrumBehavior::~TantrumBehavior()
{

}

void TantrumBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
	m_timeTillDamageFeature = TIME_BETWEEN_ATTACKS;
}

void TantrumBehavior::OnEnd(Character* character)
{
	character->m_isReactingExtremely[DISAPPOINTMENT] = false;
}

float TantrumBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[DISAPPOINTMENT]);
	}
	else if (character->m_isReactingExtremely[DISAPPOINTMENT])
	{
		m_lastCalculatedUtility = character->m_emotionState[DISAPPOINTMENT] + SATURATION_UTILITY;
	}
	else
	{
		m_lastCalculatedUtility = 0.f;
	}
	return m_lastCalculatedUtility;
}

void TantrumBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void TantrumBehavior::Act(float deltaSeconds, Character *character)
{
	Tile* characterTargetTile = character->m_owningMap->GetTileAtPosition(character->m_target);
	Feature* closestDestroyableFeature = characterTargetTile->m_feature;
	
	if (characterTargetTile->m_feature == nullptr)
	{
		closestDestroyableFeature = character->m_owningMap->FindNearestDestroyableFeature(character->m_owningTile->m_mapPosition);
		if (closestDestroyableFeature != nullptr)
		{
			characterTargetTile = closestDestroyableFeature->m_owningTile;
			character->m_target = characterTargetTile->m_mapPosition;
		}
		character->ClearPath();
	}

	int distanceToTile = CalculateManhattanDistance(character->m_owningTile->m_mapPosition, characterTargetTile->m_mapPosition);
	if (distanceToTile <= 0)
	{
		if (m_timeTillDamageFeature <= 0.f && closestDestroyableFeature != nullptr)
		{
			closestDestroyableFeature->m_health--;
			m_timeTillDamageFeature = TIME_BETWEEN_ATTACKS;
		}
	}

	if (character->m_path.empty())
	{
		character->GeneratePathToTarget();
	}

	character->FollowPath(deltaSeconds);

	GenerateEvents(character);
}

void TantrumBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	secondaryCharacter;
	if (character == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["IsTantruming"];
	Event* tantrumEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, tantrumEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, tantrumEvent, certaintyDegree);
		}
	}
}

TantrumBehavior* TantrumBehavior::Clone()
{
	return new TantrumBehavior(*this);
}

