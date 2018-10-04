#include "Game/SighBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

SighBehavior* Behavior::s_sighBehavior = new SighBehavior();

SighBehavior::SighBehavior()
	: Behavior()
{
	m_name = "Sigh";
	m_timeRunning = 0.f;
}

SighBehavior::SighBehavior(SighBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

SighBehavior::~SighBehavior()
{

}

void SighBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void SighBehavior::OnEnd(Character* character)
{
	character;
}

float SighBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[RELIEF]);
	}
	else
	{
		m_lastCalculatedUtility = character->m_emotionState[RELIEF];
	}
	return m_lastCalculatedUtility;
}

void SighBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void SighBehavior::Act(float deltaSeconds, Character *character)
{
	character->DecayEmotion(deltaSeconds * SIGH_SPEED, RELIEF);
	GenerateEvents(character);
}

void SighBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	secondaryCharacter;
	if (character == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["IsSighing"];
	Event* sighEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, sighEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, sighEvent, certaintyDegree);
		}
	}
}

SighBehavior* SighBehavior::Clone()
{
	return new SighBehavior(*this);
}

