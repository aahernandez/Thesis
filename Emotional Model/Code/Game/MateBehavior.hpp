#pragma once
#include "Game/Behavior.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"

class MateBehavior : public Behavior
{
public:
	MateBehavior();
	MateBehavior(MateBehavior *behaviorCopy);
	virtual ~MateBehavior() override;
	virtual void Init(Character *character) override;
	virtual void OnEnd(Character* character) override;
	virtual float CalcUtility(Character *character, bool isCurrentBehavior) override;
	virtual void Update(float deltaSeconds) override;
	virtual void Act(float deltaSeconds, Character *character) override;
	virtual void GenerateEvents(Character* character, Character* secondaryCharacter = nullptr) override;
	virtual void GenerateCreateChildEvent(Character* character, Character* mateCharacter = nullptr);
	virtual MateBehavior* Clone() override;

private:
	float m_mateTimer;
};