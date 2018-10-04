#pragma once
#include "Game/AffectiveArchitecture/AffectiveCommon.hpp"
#include "Game/AffectiveArchitecture/Event.hpp"

#include <string>
#include <map>

enum BasicEmotion;
class ITCXMLNode;
class Emotion;
class Entity;
struct EmotionRule;
struct EmotionFactor;

class EmotionModel
{
private:
	static BasicEmotion GetEmotionTypeFromString(std::string emotionName);
	static bool AreAgentAndPatientSame(const EventQuad& stimulusEvent);
	static bool AreActionEffectAndNPCLikingBothAboveOrBelowZero(const EventQuad& stimulusEvent, Entity *entity);
	static bool IsActionAttitudeGreaterZero(const EventQuad& stimulusEvent, Entity *entity);


	static EmotionFactor* CalculateIntensity(Emotion* currentEmotion, const EventQuad& stimulusEvent, Entity* entity);

	static void RegisterNewType(const ITCXMLNode& xmlNode);
	static void DestroyRegistry();

public:
	std::string m_name;
	static Emotion* s_emotionRegistry[NUM_BASIC_EMOTIONS];

	EmotionModel();
	~EmotionModel();

	static EmotionFactor* CalculateEmotionalStimulusForEntity(const EventQuad& stimulusEvent, Entity *entity);
	static void LoadEmotionsFromXML();
};
