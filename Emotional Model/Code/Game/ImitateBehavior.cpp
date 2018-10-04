#include "Game/ImitateBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

ImitateBehavior* Behavior::s_imitateBehavior = new ImitateBehavior();

ImitateBehavior::ImitateBehavior()
	: Behavior()
{
	m_name = "Imitate";
	m_imitationBehaviorName = "";
	m_imitationBehaviorCharacter = nullptr;
	m_timeRunning = 0.f;
}

ImitateBehavior::ImitateBehavior(ImitateBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
	m_imitationBehaviorName = behaviorCopy->m_imitationBehaviorName;
	m_imitationBehaviorCharacter = behaviorCopy->m_imitationBehaviorCharacter;
}

ImitateBehavior::~ImitateBehavior()
{

}

void ImitateBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
}

void ImitateBehavior::OnEnd(Character* character)
{
	m_imitationBehaviorName = "";
	m_imitationBehaviorCharacter = nullptr;
	m_patientCharacter = nullptr;
	character->m_isReactingExtremely[ADMIRATION] = false;
	character->ClearPath();
}

float ImitateBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	Character* imitationCharacter = GetCharacterToImitate(character);
	if (isCurrentBehavior && imitationCharacter != nullptr)
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[ADMIRATION]);
	}
	else if (character->m_isReactingExtremely[ADMIRATION] && imitationCharacter != nullptr)
	{
		m_lastCalculatedUtility = character->m_emotionState[ADMIRATION] + SATURATION_UTILITY;
	}
	else
	{
		m_lastCalculatedUtility = 0.f;
	}
	return m_lastCalculatedUtility;
}

void ImitateBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	deltaSeconds;
}

void ImitateBehavior::Act(float deltaSeconds, Character *character)
{
	if (m_patientCharacter == nullptr)
	{
		m_imitationBehaviorCharacter = GetCharacterToImitate(character);
		m_imitationBehaviorName = m_imitationBehaviorCharacter->m_currentBehavior->m_name;
	}

	if (m_imitationBehaviorName != "Imitate" && m_imitationBehaviorCharacter != nullptr)
	{
		m_patientCharacter = m_imitationBehaviorCharacter;
		Behavior* imitationBehavior = character->GetBehaviorWithName(m_imitationBehaviorName);
		imitationBehavior->Act(deltaSeconds, character);
		GenerateEvents(character, m_imitationBehaviorCharacter);
	}
}

void ImitateBehavior::GenerateEvents(Character* character, Character* secondaryCharacter /*= nullptr*/)
{
	if (character == nullptr || secondaryCharacter == nullptr)
		return;

	Action* eventAction = Action::s_actionRegistry["Imitate"];
	Event* imitateEvent = new Event(character, eventAction, secondaryCharacter);
	Event::AddEvent(character, imitateEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = character->m_owningMap->m_characters.begin(); characterIter != character->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != character && currentCharacter->HasLineOfSight(character))
		{
			float certaintyDegree = 0.f;
			if (currentCharacter == imitateEvent->m_patient)
			{
				//if the character thinks the follower likes them enough to follow them
				certaintyDegree = (currentCharacter->m_socialRelations[character->m_name]->m_liking) / 0.5f;
			}
			else if (currentCharacter->HasLineOfSight(secondaryCharacter))
			{
				float followerLiking = currentCharacter->m_socialRelations[character->m_name]->m_liking;
				float followeeLiking = currentCharacter->m_socialRelations[imitateEvent->m_patient->m_name]->m_liking;
				certaintyDegree = followeeLiking / followerLiking;
			}
			ClampZeroToOne(certaintyDegree);
			Event::AddEvent(currentCharacter, imitateEvent, certaintyDegree);
		}
	}
}

ImitateBehavior* ImitateBehavior::Clone()
{
	return new ImitateBehavior(*this);
}

Character* ImitateBehavior::GetCharacterToImitate(Character* character) const
{
	Character* imitationCharacter = character->m_owningMap->FindNearestActorOfFactionNotSelf(character->GetMapCoords(), character->m_faction, character);
	std::string imitationBehavior = "";
	if (imitationCharacter != nullptr && imitationCharacter->m_currentBehavior != nullptr)
	{
		imitationBehavior = imitationCharacter->m_currentBehavior->m_name;
	}

	while (imitationBehavior == "Imitate" && imitationCharacter != nullptr && imitationCharacter != character
		&& ((ImitateBehavior*)imitationCharacter->m_currentBehavior)->m_imitationBehaviorCharacter != imitationCharacter)
	{
		imitationBehavior = ((ImitateBehavior*)imitationCharacter->m_currentBehavior)->m_imitationBehaviorName;
		imitationCharacter = ((ImitateBehavior*)imitationCharacter->m_currentBehavior)->m_imitationBehaviorCharacter;
	}

	if (imitationBehavior == "Mate" || imitationBehavior == "Imitate")
	{
		return nullptr;
	}

	return imitationCharacter;
}

