#include "Game/BlameBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtilities.hpp"

BlameBehavior* Behavior::s_blameBehavior = new BlameBehavior();

BlameBehavior::BlameBehavior()
	: Behavior()
{
	m_name = "Blame";
	m_timeRunning = 0.f;
}

BlameBehavior::BlameBehavior(BlameBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

BlameBehavior::~BlameBehavior()
{

}

void BlameBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void BlameBehavior::OnEnd(Character* character)
{
	character;
	m_patientCharacter = nullptr;
}

float BlameBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[SHAME]);
	}
	else
	{
		m_lastCalculatedUtility = character->m_emotionState[SHAME];
	}
	return m_lastCalculatedUtility;
}

void BlameBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void BlameBehavior::Act(float deltaSeconds, Character *character)
{
	if (m_patientCharacter == nullptr)
	{
		m_patientCharacter = character->GetLeastLikedCharacterOfSameFaction();
	}
	if (character->m_owningMap->GetTileAtPosition(character->m_target)->m_character == nullptr)
	{
		if (m_patientCharacter != nullptr)
		{
			character->m_target = m_patientCharacter->GetMapCoords();
			character->ClearPath();
		}
	}

	if (character->m_path.empty())
	{
		character->GeneratePathToTarget();
	}

	character->FollowPath(deltaSeconds);

	GenerateEvents(character, m_patientCharacter);
}

void BlameBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr)
		return;

	secondaryCharacter;
	Action* eventAction = Action::s_actionRegistry["Blame"];
	Event* blameEvent = new Event(character, eventAction, secondaryCharacter);
	Event::AddEvent(character, blameEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = currentCharacter->GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(character);
			Event::AddEvent(currentCharacter, blameEvent, certaintyDegree);
		}
	}
}

BlameBehavior* BlameBehavior::Clone()
{
	return new BlameBehavior(*this);
}

