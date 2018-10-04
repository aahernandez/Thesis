#include "Game/HideBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

HideBehavior* Behavior::s_hideBehavior = new HideBehavior();

HideBehavior::HideBehavior()
	: Behavior()
{
	m_name = "Hide";
	m_timeRunning = 0.f;
}

HideBehavior::HideBehavior(HideBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

HideBehavior::~HideBehavior()
{

}

void HideBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void HideBehavior::OnEnd(Character* character)
{
	character->m_isReactingExtremely[SHAME] = false;
}

float HideBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[SHAME]);
	}
	else if (character->m_isReactingExtremely[SHAME])
	{
		m_lastCalculatedUtility = character->m_emotionState[SHAME] + SATURATION_UTILITY;
	}
	else
	{
		m_lastCalculatedUtility = 0.f;
	}
	return m_lastCalculatedUtility;
}

void HideBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void HideBehavior::Act(float deltaSeconds, Character *character)
{
	if (character->GetMapCoords() == character->m_target)
	{
		character->m_owningMap->GenerateDijkstraMapForAllCharacters(character);
		character->m_target = character->m_owningMap->GetHighestScoreDijkstraMapTile(character->GetMapCoords())->m_mapPosition;
		character->GeneratePathToTarget();
	}

	character->FollowPath(deltaSeconds);

	GenerateEvents(character);
}

void HideBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr)
		return;

	secondaryCharacter;

	Action* eventAction = Action::s_actionRegistry["IsHiding"];
	Event* hideEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, hideEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, hideEvent, certaintyDegree);
		}
	}
}

HideBehavior* HideBehavior::Clone()
{
	return new HideBehavior(*this);
}

