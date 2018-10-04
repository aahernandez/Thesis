#include "Game/FollowBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtilities.hpp"

FollowBehavior* Behavior::s_followBehavior = new FollowBehavior();

FollowBehavior::FollowBehavior()
	: Behavior()
{
	m_name = "Follow";
	m_timeRunning = 0.f;
}

FollowBehavior::FollowBehavior(FollowBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
}

FollowBehavior::~FollowBehavior()
{

}

void FollowBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void FollowBehavior::OnEnd(Character* character)
{
	character->ClearPath();
	m_patientCharacter = nullptr;
}

float FollowBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	if (isCurrentBehavior)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[ADMIRATION]);
	}
	else
	{
		m_lastCalculatedUtility = character->m_emotionState[ADMIRATION];
	}
	return m_lastCalculatedUtility;
}

void FollowBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void FollowBehavior::Act(float deltaSeconds, Character *character)
{
	if (m_patientCharacter == nullptr)
	{
		m_patientCharacter = character->m_owningMap->FindNearestActorOfFactionNotSelf(character->GetMapCoords(), character->m_faction, character);
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

void FollowBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr || secondaryCharacter == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["Follow"];
	Event* followEvent = new Event(character, eventAction, secondaryCharacter);
	Event::AddEvent(character, followEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = 0.f;
			if (currentCharacter == followEvent->m_patient)
			{
				//if the character thinks the follower likes them enough to follow them
				certaintyDegree = (currentCharacter->m_socialRelations[character->m_name]->m_liking) / 0.5f;
			}
			else if (currentCharacter->HasLineOfSight(secondaryCharacter))
			{
				float followerLiking = currentCharacter->m_socialRelations[character->m_name]->m_liking;
				float followeeLiking = currentCharacter->m_socialRelations[followEvent->m_patient->m_name]->m_liking;
				certaintyDegree = followeeLiking / followerLiking;
			}
			ClampZeroToOne(certaintyDegree);
			Event::AddEvent(currentCharacter, followEvent, certaintyDegree);
		}
	}
}

FollowBehavior* FollowBehavior::Clone()
{
	return new FollowBehavior(*this);
}

