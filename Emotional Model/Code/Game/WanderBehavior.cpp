#include "Game/WanderBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtilities.hpp"

WanderBehavior* Behavior::s_wanderBehavior = new WanderBehavior();

WanderBehavior::WanderBehavior()
	: Behavior()
{
	m_name = "Wander";
	m_timeRunning = 0.f;
}

WanderBehavior::WanderBehavior(WanderBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

WanderBehavior::~WanderBehavior()
{

}

void WanderBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void WanderBehavior::OnEnd(Character* character)
{
	character->ClearPath();
}

float WanderBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	character;
	isCurrentBehavior;
	m_lastCalculatedUtility = 0.05f;
	return m_lastCalculatedUtility;
}

void WanderBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void WanderBehavior::Act(float deltaSeconds, Character *character)
{
	if (character->m_path.empty())
	{
		character->GenerateAndSetRandomPath();
	}

	character->FollowPath(deltaSeconds);

	GenerateEvents(character);
}

void WanderBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	secondaryCharacter;
	if (character == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["IsWandering"];
	Event* wanderEvent = new Event(character, eventAction, character);
	Event::AddEvent(character, wanderEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, wanderEvent, certaintyDegree);
		}
	}
}

WanderBehavior* WanderBehavior::Clone()
{
	return new WanderBehavior(*this);
}

