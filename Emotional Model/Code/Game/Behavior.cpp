#include "Game/Behavior.hpp"
#include "Game/BehaviorCommon.hpp"
#include "Game/PathGenerator.hpp"

Behavior::Behavior()
	: m_patientCharacter(nullptr)
	, m_lastCalculatedUtility(-1.f)
{

}

Behavior::~Behavior()
{

}

float Behavior::CalcUtility(Character *character, bool isCurrentUtility)
{
	character;
	isCurrentUtility;
	return 0.f;
}

void Behavior::Update(float deltaSeconds)
{
	m_timeRunning += deltaSeconds * UTILITY_DECAY_SPEED;
}

float Behavior::GetIsCurrentlyRunningUtility(float utility) const
{
	return utility + 1 - m_timeRunning;
}
