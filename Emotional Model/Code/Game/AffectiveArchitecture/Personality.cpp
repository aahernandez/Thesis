#include "Game/AffectiveArchitecture/Personality.hpp"
#include "Game/AffectiveArchitecture/Emotion.hpp"
#include "Engine/Math/MathUtilities.hpp"

Personality::Personality()
{
	m_traits[OPENNESS] = GetRandomFloatZeroToOne();
	m_traits[CONSCIENTIOUSNESS] = GetRandomFloatZeroToOne();
	m_traits[EXTRAVERSION] = GetRandomFloatZeroToOne();
	m_traits[AGREEABLENESS] = GetRandomFloatZeroToOne();
	m_traits[NEUROTOCISM] = GetRandomFloatZeroToOne();
}

Personality::~Personality()
{
}

void Personality::ApplyEmotionFactorsForEmotionStimulus(EmotionFactor* emotionFactor)
{
	float averageTraitInfluence = 1.f;
	if (emotionFactor->m_name == JOY || emotionFactor->m_name == HOPE 
		|| emotionFactor->m_name == ADMIRATION || emotionFactor->m_name == RELIEF
		|| emotionFactor->m_name == PRIDE)
	{
		averageTraitInfluence = ((1 + m_traits[AGREEABLENESS]) + (1 + m_traits[EXTRAVERSION]) + (1 - m_traits[NEUROTOCISM])) / 3;
	}
	else if (emotionFactor->m_name == DISAPPOINTMENT || emotionFactor->m_name == DISTRESS
		|| emotionFactor->m_name == FEAR || emotionFactor->m_name == ANGER
		|| emotionFactor->m_name == SHAME)
	{
		averageTraitInfluence = ((1 + m_traits[NEUROTOCISM]) + (1 - m_traits[AGREEABLENESS]) ) * 0.5f;
	}

	emotionFactor->m_value *= averageTraitInfluence;
	emotionFactor->m_value = Clamp(emotionFactor->m_value, 0.f, 1.f);
}

std::string Personality::GetPersonalityTraitAsString(const PersonalityTrait& trait)
{
	if (trait == OPENNESS)
	{
		return "Openness";
	}
	else if (trait == CONSCIENTIOUSNESS)
	{
		return "Conscientiousness";
	}
	else if (trait == EXTRAVERSION)
	{
		return "Extraversion";
	}
	else if (trait == AGREEABLENESS)
	{
		return "Agreeableness";
	}
	else if (trait == NEUROTOCISM)
	{
		return "Neuroticism";
	}

	return "NO_PERSONALITY_TRAIT";
}
