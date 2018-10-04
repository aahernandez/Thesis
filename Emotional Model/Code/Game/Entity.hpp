#pragma once
#include "Game/AffectiveArchitecture/Event.hpp"
#include "Game/AffectiveArchitecture/Emotion.hpp"

#include <string>
#include <map>
#include <vector>

class Action;
class Personality;
class Entity;
class SocialRelation;
class BetaDistribution;
struct EmotionFactor;

typedef float EmotionMeasures[NUM_BASIC_EMOTIONS];
typedef bool EmotionToggles[NUM_BASIC_EMOTIONS];
typedef std::vector<Entity*> EntityVector;
typedef std::map<std::string, float> ActionAttitudesMap;

struct SolidarityEvent
{
	Entity *emotionFeelingEntity;
	EmotionFactor *emotionFelt;

	~SolidarityEvent();
};

class Entity
{
public:
	std::string m_name;
	ActionAttitudesMap m_actionPraises;
	EventVector m_perceivedEvents;
	EmotionMeasures m_emotionsTriggered;
	EmotionMeasures m_emotionState;
	EmotionMeasures m_emotionsAverage;
	EmotionMeasures m_intensityThreshold;
	EmotionMeasures m_saturationThreshold;
	EmotionToggles m_isReactingExtremely;
	EmotionToggles m_hasEmotionStateGoneUp;
	Personality *m_personality;
	std::map<std::string, SocialRelation*> m_socialRelations;
	float m_emotionDecayRate;

	static EntityVector s_entityRegistry;
	static std::map<std::string, std::vector<SolidarityEvent*>> s_solidarityEvents;

	Entity(std::string newName);
	~Entity();

	void InitEmotionVectors();
	void InitActionPraise();
	void Update(float deltaSeconds);
	void UpdateTriggeredEmotions(float deltaSeconds);
	void UpdateEmotionalState(float deltaSeconds);
	void UpdateSocialRelations(float deltaSeconds, const EventQuad& currentEvent, EmotionFactor *emotionFactor);
	static void UpdateSolidarity(float deltaSeconds);
	static void RemoveSolidarityEvents();
	static void RemoveEntityFromRegistry(const std::string& entityName);

	void CalculateEmotionAverages();
	float GetActionAttitude(Action *action);
	float CalulateIntensityThreshold(BasicEmotion emotionName);
	float CalulateSaturationThreshold(BasicEmotion emotionName);
	void SetAllEmotionStates(float value);
	void RemovePerceivedEvents();
	void SetRandomSocialRelation(Entity* entity);
	void ApplyPersonalityToEmotionFactorForEmotionStimulus(EmotionFactor* emotionFactor);
	void DecayEmotion(float deltaSeconds, BasicEmotion emotionName);
	std::string GetExtremeBehaviorFromEmotion(BasicEmotion emotionName);
	std::string GetBehaviorFromEmotion(BasicEmotion emotionName);
	float GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(Entity* solidarityEntity);
	float GetSemiRandomCertaintyDegreeBasedOnValue(float value);
	BasicEmotion GetHighestEmotion() const;
	std::string GetEmotionDataString();
	bool AreAnySaturationThresholdsPassed() const;

	static Entity* GetRandomEntity(bool canBeNullptr, Entity* notEntity = nullptr);
};