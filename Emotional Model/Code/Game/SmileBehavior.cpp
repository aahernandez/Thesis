#include "Game/SmileBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtilities.hpp"

SmileBehavior* Behavior::s_smileBehavior = new SmileBehavior();

SmileBehavior::SmileBehavior()
	: Behavior()
{
	m_name = "Smile";
	m_timeRunning = 0.f;
}

SmileBehavior::SmileBehavior(SmileBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

SmileBehavior::~SmileBehavior()
{

}

void SmileBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void SmileBehavior::OnEnd(Character *character)
{
	character->ClearPath();
}

float SmileBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[JOY]);
	}
	else
	{
		m_lastCalculatedUtility = character->m_emotionState[JOY];
	}
	return m_lastCalculatedUtility;
}

void SmileBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void SmileBehavior::Act(float deltaSeconds, Character *character)
{
	if (character->m_path.empty())
	{
		character->GenerateAndSetRandomPath();
	}

	character->FollowPath(deltaSeconds);

	GenerateEvents(character);
}

void SmileBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	secondaryCharacter;
	if (character == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["IsSmiling"];
	Event* smileEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, smileEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, smileEvent, certaintyDegree);
		}
	}
}

SmileBehavior* SmileBehavior::Clone()
{
	return new SmileBehavior(*this);
}

