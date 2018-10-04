#include "Game/DaydreamBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

DaydreamBehavior* Behavior::s_daydreamBehavior = new DaydreamBehavior();

DaydreamBehavior::DaydreamBehavior()
	: Behavior()
{
	m_name = "Daydream";
	m_timeRunning = 0.f;
}

DaydreamBehavior::DaydreamBehavior(DaydreamBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

DaydreamBehavior::~DaydreamBehavior()
{

}

void DaydreamBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void DaydreamBehavior::OnEnd(Character* character)
{
	character->m_isReactingExtremely[HOPE] = false;
}

float DaydreamBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[HOPE]);
	}
	else if (character->m_isReactingExtremely[HOPE])
	{
		m_lastCalculatedUtility = character->m_emotionState[HOPE] + SATURATION_UTILITY;
	}
	else
	{
		m_lastCalculatedUtility = 0.f;
	}
	
	return m_lastCalculatedUtility;
}

void DaydreamBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void DaydreamBehavior::Act(float deltaSeconds, Character *character)
{
	deltaSeconds;
	character;
	GenerateEvents(character);
}

void DaydreamBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr)
		return;

	secondaryCharacter;

	Action* eventAction = Action::s_actionRegistry["IsDaydreaming"];
	Event* daydreamEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, daydreamEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, daydreamEvent, certaintyDegree);
		}
	}
}

DaydreamBehavior* DaydreamBehavior::Clone()
{
	return new DaydreamBehavior(*this);
}

