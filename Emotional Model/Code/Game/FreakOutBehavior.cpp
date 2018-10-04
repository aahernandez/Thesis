#include "Game/FreakOutBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

FreakOutBehavior* Behavior::s_recklessBehavior = new FreakOutBehavior();

FreakOutBehavior::FreakOutBehavior()
	: Behavior()
{
	m_name = "Freak Out";
	m_timeRunning = 0.f;
}

FreakOutBehavior::FreakOutBehavior(FreakOutBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

FreakOutBehavior::~FreakOutBehavior()
{

}

void FreakOutBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void FreakOutBehavior::OnEnd(Character* character)
{
	character->m_isReactingExtremely[DISTRESS];
}

float FreakOutBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[DISTRESS]);
	}
	else if (character->m_isReactingExtremely[DISTRESS])
	{
		m_lastCalculatedUtility = character->m_emotionState[DISTRESS] + SATURATION_UTILITY;
	}
	else
	{
		m_lastCalculatedUtility = 0.f;
	}
	return m_lastCalculatedUtility;
}

void FreakOutBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void FreakOutBehavior::Act(float deltaSeconds, Character *character)
{
	if (character->m_path.empty())
	{
		character->GenerateAndSetRandomPathWithinRadius(m_behaviorRange);
	}

	character->FollowPath(deltaSeconds * m_speedModifier);

	GenerateEvents(character);
}

void FreakOutBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr)
		return;

	secondaryCharacter;

	Action* eventAction = Action::s_actionRegistry["IsFreakingOut"];
	Event* freakOutBehavior = new Event(character, eventAction, character);
	Event::AddEvent(character, freakOutBehavior, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, freakOutBehavior, certaintyDegree);
		}
	}
}

FreakOutBehavior* FreakOutBehavior::Clone()
{
	return new FreakOutBehavior(*this);
}

