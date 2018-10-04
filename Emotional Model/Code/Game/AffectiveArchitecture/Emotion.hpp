#pragma once
#include "Game/AffectiveArchitecture/AffectiveCommon.hpp"
#include "Engine/Core/CommonInclude.hpp"
#include "Engine/Math/Range.hpp"

#include <string>
#include <vector>

struct EmotionRule
{
	bool m_hasAgent;
	Trilean m_isPositiveEffect;
	Trilean m_isPositiveAttitude;
	Trilean m_isPositivePraise;
	Trilean m_isPraisedAgent;
	Range m_dcRange;
};

struct EmotionFactor
{
	BasicEmotion m_name;
	float m_value;
};

class Emotion
{
public:
	BasicEmotion m_name;
	std::string m_oppositeEmotionName;
	std::vector<EmotionRule> m_rules;

	Emotion();
	~Emotion();

	static bool IsPositiveEmotion(BasicEmotion emotionName);
	static bool IsSadEmotion(BasicEmotion emotionName);
	static bool IsOppositeEmotion(const BasicEmotion& firstEmotionName, const BasicEmotion& secondEmotionName);
	static std::string GetBasicEmotionAsString(BasicEmotion emotion);
};