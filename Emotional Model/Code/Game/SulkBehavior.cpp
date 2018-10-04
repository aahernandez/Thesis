#include "Game/SulkBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtilities.hpp"

SulkBehavior* Behavior::s_sulkBehavior = new SulkBehavior();

SulkBehavior::SulkBehavior()
	: Behavior()
	, m_currentDirection(-1)
{
	m_name = "Sulk";
	m_timeRunning = 0.f;
}

SulkBehavior::SulkBehavior(SulkBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
	m_currentDirection = behaviorCopy->m_currentDirection;
	m_speedModifier = behaviorCopy->m_speedModifier;
}

SulkBehavior::~SulkBehavior()
{

}

void SulkBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void SulkBehavior::OnEnd(Character* character)
{
	character;
}

float SulkBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[DISAPPOINTMENT]);
	}
	else
	{
		m_lastCalculatedUtility = character->m_emotionState[DISAPPOINTMENT];
	}
	return m_lastCalculatedUtility;
}

void SulkBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void SulkBehavior::Act(float deltaSeconds, Character *character)
{
	if (m_currentDirection == -1)
	{
		m_currentDirection = (char)GetRandomIntInRange(0, 3);
	}
	
	Tile* tileToMoveTo = nullptr;

	switch (m_currentDirection)
	{
	case 0: //north
		tileToMoveTo = character->m_owningMap->GetTileToNorth(character->m_owningTile);
		break;
	case 1: //south
		tileToMoveTo = character->m_owningMap->GetTileToSouth(character->m_owningTile);
		break;
	case 2: //east
		tileToMoveTo = character->m_owningMap->GetTileToEast(character->m_owningTile);
		break;
	case 3: //west
		tileToMoveTo = character->m_owningMap->GetTileToWest(character->m_owningTile);
		break;
	}

	if (tileToMoveTo != nullptr && tileToMoveTo->IsPassable())
	{
		character->MoveTowardsNewTile(deltaSeconds * m_speedModifier, tileToMoveTo);
		if (character->IsOnTile(tileToMoveTo))
		{
			character->PutSelfOnNewTile(tileToMoveTo);
		}
	}
	else
	{
		m_currentDirection = -1;
		Act(deltaSeconds, character);
	}

	GenerateEvents(character);
}

void SulkBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	secondaryCharacter;
	if (character == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["IsSulking"];
	Event* sulkEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, sulkEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, sulkEvent, certaintyDegree);
		}
	}
}

SulkBehavior* SulkBehavior::Clone()
{
	return new SulkBehavior(*this);
}

