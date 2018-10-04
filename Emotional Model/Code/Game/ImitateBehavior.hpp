#pragma once
#include "Game/Behavior.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"

class ImitateBehavior : public Behavior
{
public:

	std::string m_imitationBehaviorName;
	Character* m_imitationBehaviorCharacter;

	ImitateBehavior();
	ImitateBehavior(ImitateBehavior *behaviorCopy);
	virtual ~ImitateBehavior() override;
	virtual void Init(Character *character) override;
	virtual void OnEnd(Character* character) override;
	virtual float CalcUtility(Character *character, bool isCurrentBehavior) override;
	virtual void Update(float deltaSeconds) override;
	virtual void Act(float deltaSeconds, Character *character) override;
	virtual void GenerateEvents(Character* character, Character* secondaryCharacter = nullptr) override;
	virtual ImitateBehavior* Clone() override;
	Character* GetCharacterToImitate(Character* character) const;
};