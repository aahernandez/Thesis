#include "Game/AttackBehavior.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Engine/Math/MathUtilities.hpp"

AttackBehavior* Behavior::s_attackBehavior = new AttackBehavior();

AttackBehavior::AttackBehavior()
	: Behavior()
{
	m_name = "Attack";
	m_timeRunning = 0.f;
	m_isAdjacentToEnemy = false;
	m_timeTillNextAttack = TIME_BETWEEN_ATTACKS;
}

AttackBehavior::AttackBehavior(AttackBehavior *behaviorCopy)
{
	m_behaviorRange = behaviorCopy->m_behaviorRange;
	m_name = behaviorCopy->m_name;
	m_isAdjacentToEnemy = behaviorCopy->m_isAdjacentToEnemy;
	m_patientCharacter = behaviorCopy->m_patientCharacter;
	m_timeTillNextAttack = behaviorCopy->m_timeTillNextAttack;
}

AttackBehavior::~AttackBehavior()
{

}

void AttackBehavior::Init(Character *character)
{
	character;
	m_timeRunning = 0.f;
	m_patientCharacter = nullptr;
}

void AttackBehavior::OnEnd(Character *character)
{
	character->m_isReactingExtremely[ANGER] = false;
	character->m_isReactingExtremely[FEAR] = false;
	m_patientCharacter = nullptr;
}

float AttackBehavior::CalcUtility(Character *character, bool isCurrentBehavior)
{
	m_lastCalculatedUtility = 0.f;
	if (isCurrentBehavior && character->m_owningMap->DoesActorNotOfFactionExistNotSelf(character->m_faction, character))
	{
		m_lastCalculatedUtility = GetIsCurrentlyRunningUtility(character->m_emotionState[ANGER]);
	}
	else if (character->m_isReactingExtremely[ANGER] && character->m_owningMap->DoesOtherActorExist(character))
	{
		m_lastCalculatedUtility = character->m_emotionState[ANGER] + SATURATION_UTILITY;
	}
	else if (character->m_isReactingExtremely[FEAR] && character->m_owningMap->DoesActorNotOfFactionExistNotSelf(character->m_faction, character))
	{
		m_lastCalculatedUtility = character->m_emotionState[FEAR] + SATURATION_UTILITY;
	}
	else if (character->m_owningMap->DoesActorNotOfFactionExistNotSelf(character->m_faction, character))
	{
		m_lastCalculatedUtility = character->m_emotionState[ANGER];
	}
	return m_lastCalculatedUtility;
}

void AttackBehavior::Update(float deltaSeconds)
{
	Behavior::Update(deltaSeconds);
	m_timeTillNextAttack -= deltaSeconds;
}

void AttackBehavior::Act(float deltaSeconds, Character *character)
{
	Map* gameMap = character->m_owningMap;
	m_isAdjacentToEnemy = false;

	if (m_patientCharacter == nullptr)
	{
		if (character->m_isReactingExtremely[ANGER])
		{
			m_patientCharacter = gameMap->FindNearestActorNotSelf(character->GetMapCoords(), character);
		}
		else
		{
			m_patientCharacter = gameMap->FindNearestActorNotOfFaction(character->GetMapCoords(), character->m_faction);
		}
	}

	if (m_patientCharacter == nullptr)
		return;

	if (gameMap->GetTileAtPosition(character->m_target)->m_character != m_patientCharacter)
	{
		character->m_target = m_patientCharacter->GetMapCoords();
		character->ClearPath();
	}

	if (character->m_path.empty())
	{
		character->GeneratePathToTarget();
	}
	if (gameMap->AreCharactersAdjacent(character, m_patientCharacter))
	{
		m_isAdjacentToEnemy = true;
		if (m_timeTillNextAttack <= 0.f)
		{
			m_timeTillNextAttack = TIME_BETWEEN_ATTACKS;
			m_patientCharacter->m_currentHealth--;
		}
	}
	else
	{
		character->FollowPath(deltaSeconds);
	}
	GenerateEvents(character, m_patientCharacter);
}

void AttackBehavior::GenerateEvents(Character* behaviorCharacter, Character* secondaryCharacter /*= nullptr*/)
{
	if (behaviorCharacter == nullptr || secondaryCharacter == nullptr)
		return;


	Action* eventAction = Action::s_actionRegistry["Attack"];
	Event* attackEvent = new Event(behaviorCharacter, eventAction, secondaryCharacter);
	Event::AddEvent(behaviorCharacter, attackEvent, 1.f);
	Event* hurtEvent = new Event(secondaryCharacter, Action::s_actionRegistry["IsHurting"], secondaryCharacter);
	Event::AddEvent(behaviorCharacter, hurtEvent, 1.f);

	std::vector<Character*>::iterator characterIter;
	for (characterIter = behaviorCharacter->m_owningMap->m_characters.begin(); characterIter != behaviorCharacter->m_owningMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;

		if (currentCharacter != behaviorCharacter)// && currentCharacter->HasLineOfSight(behaviorCharacter))
		{
			float certaintyDegree = 1.f;
			if (!m_isAdjacentToEnemy)
			{
				if (currentCharacter == attackEvent->m_patient)
				{
					//if the character thinks the enemy has dominance enough to warrant an attack
					float oppositeDominance = 1.f - currentCharacter->m_socialRelations[behaviorCharacter->m_name]->m_dominance;
					float averageSocialRelations = oppositeDominance + currentCharacter->m_socialRelations[behaviorCharacter->m_name]->m_familiarity;
					averageSocialRelations /= 2.f;
					certaintyDegree = currentCharacter->GetSemiRandomCertaintyDegreeBasedOnValue(averageSocialRelations);
				}
				else
				{
					float dominanceOverAttacker = 1.f - currentCharacter->m_socialRelations[behaviorCharacter->m_name]->m_dominance;
					float dominanceOverAttackee = currentCharacter->m_socialRelations[attackEvent->m_patient->m_name]->m_dominance;
					float dominanceDifference = dominanceOverAttacker - dominanceOverAttackee;
					ClampZeroToOne(dominanceDifference);
					float averageSocialRelations = dominanceDifference + currentCharacter->m_socialRelations[behaviorCharacter->m_name]->m_familiarity;
					averageSocialRelations /= 2.f;
					certaintyDegree = currentCharacter->GetSemiRandomCertaintyDegreeBasedOnValue(averageSocialRelations);
				}
				ClampZeroToOne(certaintyDegree);
			}
			Event::AddEvent(currentCharacter, attackEvent, certaintyDegree);
			Event::AddEvent(currentCharacter, hurtEvent, certaintyDegree);
		}
	}
}

AttackBehavior* AttackBehavior::Clone()
{
	return new AttackBehavior(*this);
}

