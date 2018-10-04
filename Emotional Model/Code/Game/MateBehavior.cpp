#include "Game/MateBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

MateBehavior* Behavior::s_mateBehavior = new MateBehavior();

MateBehavior::MateBehavior()
	: Behavior()
	, m_mateTimer(0.f)
{
	m_name = "Mate";
	m_timeRunning = 0.f;
}

MateBehavior::MateBehavior(MateBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

MateBehavior::~MateBehavior()
{

}

void MateBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void MateBehavior::OnEnd(Character* character)
{
	m_patientCharacter = nullptr;
	character->m_isReactingExtremely[JOY] = false;
}

float MateBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	float utility = character->m_emotionState[JOY] - m_mateTimer;
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(utility);
	}
	else if (character->m_isReactingExtremely[JOY])
	{
		m_lastCalculatedUtility = utility + SATURATION_UTILITY;
	}
	else
	{
		m_lastCalculatedUtility = 0.f;
	}
	return m_lastCalculatedUtility;
}

void MateBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	m_mateTimer -= deltaSeconds;
	m_mateTimer = Clamp(m_mateTimer, 0.f, TIME_BETWEEN_MATING);
}

void MateBehavior::Act(float deltaSeconds, Character *character)
{
	if (m_patientCharacter == nullptr)
	{
		m_patientCharacter = character->GetMostLikedCharacterOfSameFaction();
		if (m_patientCharacter == nullptr)
			return;
	}
	if (character->m_path.empty())
	{
		character->m_target = m_patientCharacter->GetMapCoords();
		if (character->GetMapCoords() == character->m_target)
		{
			character->m_owningMap->CreateCharacterOfType(character->m_faction, character->GetMapCoords());
			m_mateTimer = TIME_BETWEEN_MATING;
			 ((MateBehavior*)m_patientCharacter->GetBehaviorWithName("Mate"))->m_mateTimer = TIME_BETWEEN_MATING;
			return;
		}
		else
		{
			character->GeneratePathToTarget();
		}
	}

	character->FollowPath(deltaSeconds);
	character->ClearPath();
	GenerateEvents(character, m_patientCharacter);
}

void MateBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr || secondaryCharacter == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["Mate"];
	Event* mateEvent = new Event(character, eventAction, secondaryCharacter);
	Event::AddEvent(character, mateEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = 0.f;
			if (currentCharacter == mateEvent->m_patient)
			{
				//if the character thinks the other character likes them enough to mate with them
				certaintyDegree = currentCharacter->m_socialRelations[character->m_name]->m_liking / 0.5f;
			}
			else if (currentCharacter->HasLineOfSight(secondaryCharacter))
			{
				float followerLiking = currentCharacter->m_socialRelations[character->m_name]->m_liking;
				float followeeLiking = currentCharacter->m_socialRelations[mateEvent->m_patient->m_name]->m_liking;
				certaintyDegree = followeeLiking / followerLiking;
			}
			ClampZeroToOne(certaintyDegree);
			Event::AddEvent(currentCharacter, mateEvent, certaintyDegree);
		}
	}
}

void MateBehavior::GenerateCreateChildEvent(Character* character, Character* mateCharacter /*= nullptr*/)
{
	Action* eventAction = Action::s_actionRegistry["Mate"];
	Event* newEvent = new Event(character, eventAction, mateCharacter);
	Event::AddEvent(character, newEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character) && currentCharacter->HasLineOfSight(mateCharacter))
		{
			Event::AddEvent(currentCharacter, newEvent, 1.f);
		}
	}
}

MateBehavior* MateBehavior::Clone()
{
	return new MateBehavior(*this);
}

