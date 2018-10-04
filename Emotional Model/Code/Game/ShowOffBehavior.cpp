#include "Game/ShowOffBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtilities.hpp"

ShowOffBehavior* Behavior::s_showOffBehavior = new ShowOffBehavior();

ShowOffBehavior::ShowOffBehavior()
	: Behavior()
{
	m_name = "Show Off";
	m_timeRunning = 0.f;
}

ShowOffBehavior::ShowOffBehavior(ShowOffBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
	m_speedModifier = behaviorCopy->m_speedModifier;
}

ShowOffBehavior::~ShowOffBehavior()
{

}

void ShowOffBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void ShowOffBehavior::OnEnd(Character* character)
{
	character;
}

float ShowOffBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[PRIDE]);
	}
	else
	{
		m_lastCalculatedUtility = character->m_emotionState[PRIDE];
	}
	return m_lastCalculatedUtility;
}

void ShowOffBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void ShowOffBehavior::Act(float deltaSeconds, Character *character)
{
	Tile* tileToMoveTo = nullptr;

	if (character->m_path.empty())
	{
		character->m_owningMap->GenerateDijkstraMapForAllCharacters(character);
		tileToMoveTo = character->m_owningMap->FindRandomTile();

		while (!tileToMoveTo->IsPassable() && m_mapRange.IsIntInRange((int)tileToMoveTo->m_dijkstraMapFScore))
		{
			tileToMoveTo = character->m_owningMap->FindRandomTile();
		}

		character->m_target = tileToMoveTo->m_mapPosition;

		character->GeneratePathToTarget();
	}

	character->FollowPath(deltaSeconds);

	GenerateEvents(character);
}

void ShowOffBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	secondaryCharacter;
	if (character == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["IsShowingOff"];
	Event* showOffEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, showOffEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, showOffEvent, certaintyDegree);
		}
	}
}

ShowOffBehavior* ShowOffBehavior::Clone()
{
	return new ShowOffBehavior(*this);
}

