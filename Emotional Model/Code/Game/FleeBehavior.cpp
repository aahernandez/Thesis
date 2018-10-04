#include "Game/FleeBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtilities.hpp"

FleeBehavior* Behavior::s_fleeBehavior = new FleeBehavior();

FleeBehavior::FleeBehavior()
	: Behavior()
{
	m_name = "Flee";
	m_timeRunning = 0.f;
}

FleeBehavior::FleeBehavior(FleeBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

FleeBehavior::~FleeBehavior()
{

}

void FleeBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void FleeBehavior::OnEnd(Character *character)
{
	character;
}

float FleeBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[FEAR]);
	}
	else
	{
		m_lastCalculatedUtility = character->m_emotionState[FEAR];
	}
	return m_lastCalculatedUtility;
}

void FleeBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void FleeBehavior::Act(float deltaSeconds, Character *character)
{
	if (character->GetMapCoords() == character->m_target)
	{
		character->m_owningMap->GenerateDijkstraMapForAllCharactersNotOfFaction(character->m_faction, character);
		character->m_target = character->m_owningMap->GetHighestScoreDijkstraMapTile(character->GetMapCoords())->m_mapPosition;
		character->GeneratePathToTarget();
	}

	character->FollowPath(deltaSeconds);

	GenerateEvents(character);
}

void FleeBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr)
		return;

	secondaryCharacter;

	Action* eventAction = Action::s_actionRegistry["IsFleeing"];
	Event* fleeEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, fleeEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, fleeEvent, certaintyDegree);
		}
	}
}

FleeBehavior* FleeBehavior::Clone()
{
	return new FleeBehavior(*this);
}

