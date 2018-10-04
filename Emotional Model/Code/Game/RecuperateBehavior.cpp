#include "Game/RecuperateBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

RecuperateBehavior* Behavior::s_recuperateBehavior = new RecuperateBehavior();

RecuperateBehavior::RecuperateBehavior()
	: Behavior()
{
	m_name = "Recuperate";
	m_timeRunning = 0.f;
}

RecuperateBehavior::RecuperateBehavior(RecuperateBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

RecuperateBehavior::~RecuperateBehavior()
{

}

void RecuperateBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void RecuperateBehavior::OnEnd(Character* character)
{
	character->m_isReactingExtremely[RELIEF] = false;
}

float RecuperateBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[RELIEF]);
	}
	else if (character->m_isReactingExtremely[RELIEF])
	{
		m_lastCalculatedUtility = character->m_emotionState[RELIEF] + SATURATION_UTILITY;
	}
	else
	{
		m_lastCalculatedUtility = 0.f;
	}
	return m_lastCalculatedUtility;
}

void RecuperateBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void RecuperateBehavior::Act(float deltaSeconds, Character *character)
{
	character->DecayEmotion(deltaSeconds * RECUPERATE_SPEED, RELIEF);
	GenerateEvents(character);
}

void RecuperateBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	secondaryCharacter;
	if (character == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["IsRecuperating"];
	Event* recuperateEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, recuperateEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, recuperateEvent, certaintyDegree);
		}
	}
}

RecuperateBehavior* RecuperateBehavior::Clone()
{
	return new RecuperateBehavior(*this);
}

