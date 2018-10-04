#pragma once
#include "Game/AffectiveArchitecture/AffectiveCommon.hpp"

#include <string>
#include <map>

struct EmotionFactor;

class Personality
{
public:
	Personality();
	~Personality();

	float m_traits[NUM_PERSONALITY_TRAITS];
	void ApplyEmotionFactorsForEmotionStimulus(EmotionFactor* emotionFactor);

	static std::string GetPersonalityTraitAsString(const PersonalityTrait& trait);
};