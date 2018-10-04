#include "Game/GloatBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

GloatBehavior* Behavior::s_gloatBehavior = new GloatBehavior();

GloatBehavior::GloatBehavior()
	: Behavior()
{
	m_name = "Show Off";
	m_timeRunning = 0.f;
}

GloatBehavior::GloatBehavior(GloatBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
	m_speedModifier = behaviorCopy->m_speedModifier;
}

GloatBehavior::~GloatBehavior()
{

}

void GloatBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void GloatBehavior::OnEnd(Character* character)
{
	character->m_isReactingExtremely[PRIDE] = false;
}

float GloatBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[PRIDE]);
	}
	else if (character->m_isReactingExtremely[PRIDE])
	{
		m_lastCalculatedUtility = character->m_emotionState[PRIDE] + SATURATION_UTILITY;
	}
	else
	{
		m_lastCalculatedUtility = 0.f;
	}
	return m_lastCalculatedUtility;
}

void GloatBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void GloatBehavior::Act(float deltaSeconds, Character *character)
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

void GloatBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr)
		return;

	secondaryCharacter;

	Action* eventAction = Action::s_actionRegistry["IsGloating"];
	Event* gloatEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, gloatEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, gloatEvent, certaintyDegree);
		}
	}
}

GloatBehavior* GloatBehavior::Clone()
{
	return new GloatBehavior(*this);
}

