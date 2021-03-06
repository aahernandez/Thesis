#pragma once
#include "Game/Behavior.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"

class AttackBehavior : public Behavior
{
public:
	AttackBehavior();
	AttackBehavior(AttackBehavior *behaviorCopy);
	virtual ~AttackBehavior() override;
	virtual void Init(Character *character) override;
	virtual	void OnEnd(Character *character) override;
	virtual float CalcUtility(Character *character, bool isCurrentBehavior) override;
	virtual void Update(float deltaSeconds) override;
	virtual void Act(float deltaSeconds, Character *character) override;
	virtual void GenerateEvents(Character* character, Character* secondaryCharacter = nullptr) override;
	virtual AttackBehavior* Clone() override;

private:
	bool m_isAdjacentToEnemy;
	float m_timeTillNextAttack;
};