#pragma once
#include "Game/Behavior.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"

class BlameBehavior : public Behavior
{
public:
	BlameBehavior();
	BlameBehavior(BlameBehavior *behaviorCopy);
	virtual ~BlameBehavior() override;
	virtual void Init(Character *character) override;
	virtual void OnEnd(Character* character) override;
	virtual float CalcUtility(Character *character, bool isCurrentBehavior) override;
	virtual void Update(float deltaSeconds) override;
	virtual void Act(float deltaSeconds, Character *character) override;
	virtual void GenerateEvents(Character* character, Character* secondaryCharacter = nullptr) override;
	virtual BlameBehavior* Clone() override;
};