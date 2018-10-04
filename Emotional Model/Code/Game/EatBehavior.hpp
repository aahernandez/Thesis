#pragma once
#include "Game/Behavior.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"

class EatBehavior : public Behavior
{
public:
	EatBehavior();
	EatBehavior(EatBehavior *behaviorCopy);
	virtual ~EatBehavior() override;
	virtual void Init(Character *character) override;
	virtual void OnEnd(Character* character) override;
	virtual float CalcUtility(Character *character, bool isCurrentBehavior) override;
	virtual void Update(float deltaSeconds) override;
	virtual void Act(float deltaSeconds, Character *character) override;
	virtual void GenerateEvents(Character* character, Character* secondaryCharacter = nullptr) override;
	virtual EatBehavior* Clone() override;

	Behavior* m_attackBehaviorReference;
	bool m_isCurrentlyConsumingFood;
	float m_timeTillNextBite;
};