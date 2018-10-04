#pragma once
#include "Game/AffectiveArchitecture/Action.hpp"
#include "Game/AffectiveArchitecture/Event.hpp"
#include "Game/AffectiveArchitecture/SocialRelation.hpp"
#include "Engine/Math/IntRange.hpp"
#include <vector>

class Character;
class Tile;
class WanderBehavior;
class HideBehavior;
class AttackBehavior;
class FollowBehavior;
class ShowOffBehavior;
class FleeBehavior;
class SulkBehavior;
class FreakOutBehavior;
class MateBehavior;
class RecuperateBehavior;
class CryBehavior;
class SmileBehavior;
class TantrumBehavior;
class SighBehavior;
class GloatBehavior;
class ImitateBehavior;
class BlameBehavior;
class DaydreamBehavior;
class EatBehavior;

class Behavior
{
public:
	int m_sightRange;
	IntRange m_mapRange;
	float m_timeRunning;
	float m_behaviorRange;
	float m_skittishness;
	float m_speedModifier;
	float m_lastCalculatedUtility;
	std::string m_name;
	Character* m_patientCharacter;

	static WanderBehavior* s_wanderBehavior;
	static HideBehavior* s_hideBehavior;
	static AttackBehavior* s_attackBehavior;
	static FollowBehavior* s_followBehavior;
	static ShowOffBehavior* s_showOffBehavior;
	static FleeBehavior* s_fleeBehavior;
	static SulkBehavior* s_sulkBehavior;
	static FreakOutBehavior* s_recklessBehavior;
	static MateBehavior* s_mateBehavior;
	static RecuperateBehavior* s_recuperateBehavior;
	static CryBehavior* s_cryBehavior;
	static SmileBehavior* s_smileBehavior;
	static TantrumBehavior* s_tantrumBehavior;
	static SighBehavior* s_sighBehavior;
	static GloatBehavior* s_gloatBehavior;
	static ImitateBehavior* s_imitateBehavior;
	static BlameBehavior* s_blameBehavior;
	static DaydreamBehavior* s_daydreamBehavior;
	static EatBehavior* s_eatBehavior;


	Behavior();
	virtual ~Behavior();
	virtual void Init(Character* character) = 0;
	virtual void OnEnd(Character* character) = 0;
	virtual float CalcUtility(Character* character, bool isCurrentUtility);
	virtual void Update(float deltaSeconds);
	virtual void Act(float deltaSeconds, Character* character) = 0;
	virtual void GenerateEvents(Character* character, Character* secondaryCharacter = nullptr) = 0;
	virtual Behavior* Clone() = 0;
	virtual float GetIsCurrentlyRunningUtility(float utility) const;
};