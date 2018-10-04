#include "Game/AffectiveArchitecture/Emotion.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Emotion::Emotion()
	: m_name(NO_BASIC_EMOTION)
	, m_oppositeEmotionName("NO_NAME")
{
	
}

Emotion::~Emotion()
{

}

bool Emotion::IsPositiveEmotion(BasicEmotion emotionName)
{
	if (emotionName == JOY || emotionName == HOPE
		|| emotionName == ADMIRATION || emotionName == RELIEF
		|| emotionName == PRIDE)
	{
		return true;
	}
	return false;
}

bool Emotion::IsSadEmotion(BasicEmotion emotionName)
{
	if (emotionName == DISAPPOINTMENT || emotionName == SHAME || emotionName == DISTRESS)
	{
		return true;
	}
	return false;
}

bool Emotion::IsOppositeEmotion(const BasicEmotion& firstEmotionName, const BasicEmotion& secondEmotionName)
{
	return firstEmotionName == secondEmotionName;
}

std::string Emotion::GetBasicEmotionAsString(BasicEmotion emotion)
{
	if (emotion == JOY)
		return "Joy";
	if (emotion == HOPE)
		return "Hope";
	if (emotion == DISAPPOINTMENT)
		return "Disappointment";
	if (emotion == DISTRESS)
		return "Distress";
	if (emotion == FEAR)
		return "Fear";
	if (emotion == RELIEF)
		return "Relief";
	if (emotion == PRIDE)
		return "Pride";
	if (emotion == ADMIRATION)
		return "Admiration";
	if (emotion == SHAME)
		return "Shame";
	if (emotion == ANGER)
		return "Anger";

	ASSERT_OR_DIE(false, "Emotion is not a basic emotion.");
	return "NO_EMOTION";
}
