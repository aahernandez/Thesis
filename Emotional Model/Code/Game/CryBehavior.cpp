#include "Game/CryBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtilities.hpp"

CryBehavior* Behavior::s_cryBehavior = new CryBehavior();

CryBehavior::CryBehavior()
	: Behavior()
{
	m_name = "Cry";
	m_timeRunning = 0.f;
}

CryBehavior::CryBehavior(CryBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

CryBehavior::~CryBehavior()
{

}

void CryBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void CryBehavior::OnEnd(Character *character)
{
	character->ClearPath();
}

float CryBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[DISTRESS]);
	}
	else
	{
		m_lastCalculatedUtility = character->m_emotionState[DISTRESS];
	}
	return m_lastCalculatedUtility;
}

void CryBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void CryBehavior::Act(float deltaSeconds, Character *character)
{
	if (character->m_path.empty())
	{
		character->GenerateAndSetRandomPath();
	}

	character->FollowPath(deltaSeconds);

	GenerateEvents(character);
}

void CryBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr)
		return;

	secondaryCharacter;

	Action* eventAction = Action::s_actionRegistry["IsCrying"];
	Event* cryEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, cryEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, cryEvent, certaintyDegree);
		}
	}
}

CryBehavior* CryBehavior::Clone()
{
	return new CryBehavior(*this);
}

