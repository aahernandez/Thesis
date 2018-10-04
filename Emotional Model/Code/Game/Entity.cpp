#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/AffectiveArchitecture/Action.hpp"
#include "Game/AffectiveArchitecture/Event.hpp"
#include "Game/AffectiveArchitecture/Personality.hpp"
#include "Game/AffectiveArchitecture/Emotion.hpp"
#include "Game/AffectiveArchitecture/EmotionModel.hpp"
#include "Game/AffectiveArchitecture/SocialRelation.hpp"
#include "Game/AffectiveArchitecture/SocialRole.hpp"
#include "Engine/Profiling/Profiler.hpp"
#include "Engine/Core/AnalyticsSystem.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtilities.hpp"
#include "Engine/Math/Vector2.hpp"

std::vector<Entity*> Entity::s_entityRegistry;
std::map<std::string, std::vector<SolidarityEvent*>> Entity::s_solidarityEvents;

Entity::Entity(std::string newName)
{
	m_name = newName;
	m_personality = new Personality();

	InitEmotionVectors();

	s_entityRegistry.push_back(this);

	InitActionPraise();
}

Entity::~Entity()
{

}

void Entity::InitEmotionVectors()
{
	CalculateEmotionAverages();

	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		m_emotionsTriggered[emotionCount] = 0.f;
		m_intensityThreshold[emotionCount] = CalulateIntensityThreshold((BasicEmotion)emotionCount);
		m_saturationThreshold[emotionCount] = CalulateSaturationThreshold((BasicEmotion)emotionCount);
		m_emotionState[emotionCount] = GetRandomFloatInRange(m_emotionsAverage[emotionCount], 1.f);
		m_isReactingExtremely[emotionCount] = false;
		m_hasEmotionStateGoneUp[emotionCount] = false;
	}

}

void Entity::InitActionPraise()
{
	ActionIterator actionIter;
	for (actionIter = Action::s_actionRegistry.begin(); actionIter != Action::s_actionRegistry.end(); ++actionIter)
	{
		Action* currentAction = (*actionIter).second;
		m_actionPraises[currentAction->m_name] = GetRandomFloatInRange(-1.f, 1.f);
	}
}

void Entity::Update(float deltaSeconds)
{
	UpdateTriggeredEmotions(deltaSeconds);
	UpdateEmotionalState(deltaSeconds);
}

void Entity::UpdateTriggeredEmotions(float deltaSeconds)
{
	std::string fileName = Stringf("%sEvents", m_name.c_str());
	std::string eventData = "";
	std::string eventFillData = "%f,%s,%s,%s,%f,%s,%f,%s,";

	EventIterator eventIter;
	for (eventIter = m_perceivedEvents.begin(); eventIter != m_perceivedEvents.end(); ++eventIter)
	{
		EventQuad& currentEvent = *eventIter;
		EmotionFactor* emotionFactor = EmotionModel::CalculateEmotionalStimulusForEntity((currentEvent), this);
		if (emotionFactor != nullptr)
		{
			ApplyPersonalityToEmotionFactorForEmotionStimulus(emotionFactor);
			emotionFactor->m_value -= m_intensityThreshold[emotionFactor->m_name];
			emotionFactor->m_value = Clamp(emotionFactor->m_value, 0.f, 1.f);

			if (emotionFactor->m_value > 0.f)
			{
				m_emotionsTriggered[emotionFactor->m_name] += emotionFactor->m_value * deltaSeconds * EMOTION_CHANGE_SPEED;

				if (emotionFactor->m_value >= m_saturationThreshold[emotionFactor->m_name])
				{
					m_isReactingExtremely[emotionFactor->m_name] = true;
					if (Game::s_gameInstance->m_isCollectingAnalytics)
					{
						eventData = Stringf(eventFillData.c_str(),
							(float)GetCurrentTimeSeconds(), currentEvent.first->m_agent->m_name.c_str(), currentEvent.first->m_action->m_name.c_str(),
							currentEvent.first->m_patient->m_name.c_str(), currentEvent.second, Emotion::GetBasicEmotionAsString(emotionFactor->m_name).c_str(), emotionFactor->m_value, "Above");
					}
				}
				else
				{
					if (Game::s_gameInstance->m_isCollectingAnalytics)
					{
						eventData = Stringf(eventFillData.c_str(), 
							(float)GetCurrentTimeSeconds(), currentEvent.first->m_agent->m_name.c_str(), currentEvent.first->m_action->m_name.c_str(),
							currentEvent.first->m_patient->m_name.c_str(), currentEvent.second, Emotion::GetBasicEmotionAsString(emotionFactor->m_name).c_str(), emotionFactor->m_value, "Below");
					}
				}

				UpdateSocialRelations(deltaSeconds, currentEvent, emotionFactor);
				ClampZeroToOne(m_emotionsTriggered[emotionFactor->m_name]);
			}
			else
			{
				emotionFactor->m_value = 0.f;
				if (Game::s_gameInstance->m_isCollectingAnalytics)
				{
					eventData = Stringf(eventFillData.c_str(),
						(float)GetCurrentTimeSeconds(), currentEvent.first->m_agent->m_name.c_str(), currentEvent.first->m_action->m_name.c_str(),
						currentEvent.first->m_patient->m_name.c_str(), currentEvent.second, Emotion::GetBasicEmotionAsString(emotionFactor->m_name).c_str(), emotionFactor->m_value, "Below");
				}
			}

		}

		if (!eventData.empty() && Game::s_gameInstance->m_isCollectingAnalytics)
		{
			eventData += GetEmotionDataString();
			g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), eventData.c_str());
			eventData.clear();
		}
	}
}

void Entity::UpdateEmotionalState(float deltaSeconds)
{
	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		if (m_emotionsTriggered[emotionCount] == 0.f)
		{
			DecayEmotion(deltaSeconds, (BasicEmotion)emotionCount);
		}
		float previousEmotionState = roundf(m_emotionState[emotionCount] * 100) / 100;
		m_emotionState[emotionCount] += m_emotionsTriggered[emotionCount];
		float newEmotionState = roundf(m_emotionState[emotionCount] * 100) / 100;
		if (newEmotionState > previousEmotionState)
		{
			m_hasEmotionStateGoneUp[emotionCount] = true;
		}
		else if (newEmotionState < previousEmotionState)
		{
			m_hasEmotionStateGoneUp[emotionCount] = false;
		}
		m_emotionState[emotionCount] = Clamp(m_emotionState[emotionCount], 0.f, 1.f);
		m_emotionsTriggered[emotionCount] = 0.f;
	}
}

void Entity::UpdateSocialRelations(float deltaSeconds, const EventQuad& currentEvent, EmotionFactor *emotionFactor)
{
	//Get character affecting
	Entity *socialEntity = currentEvent.first->m_agent;	
	SocialRelation *socialRelationForEntity = m_socialRelations[socialEntity->m_name];
	if (socialEntity == nullptr || socialRelationForEntity == nullptr)
	{
		return;
	}

 	//change liking value
	if (Emotion::IsPositiveEmotion(emotionFactor->m_name))
	{
		socialRelationForEntity->m_liking += emotionFactor->m_value * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
		ClampZeroToOne(socialRelationForEntity->m_liking);
	}
	else
	{
		socialRelationForEntity->m_liking -= emotionFactor->m_value * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
		ClampZeroToOne(socialRelationForEntity->m_liking);
	}

	//change dominance
	if (emotionFactor->m_name == PRIDE || emotionFactor->m_name == ANGER)
	{
		socialRelationForEntity->m_dominance += emotionFactor->m_value * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
		ClampZeroToOne(socialRelationForEntity->m_dominance);
	}
	else if (emotionFactor->m_name == FEAR || emotionFactor->m_name == DISTRESS 
		|| emotionFactor->m_name == ADMIRATION || emotionFactor->m_name == SHAME)
	{
		if (emotionFactor->m_name == FEAR || emotionFactor->m_name == DISTRESS)
		{
			socialEntity->m_socialRelations[m_name]->m_dominance += emotionFactor->m_value * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
			ClampZeroToOne(socialEntity->m_socialRelations[m_name]->m_dominance);
		}

		socialRelationForEntity->m_dominance -= emotionFactor->m_value * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
		ClampZeroToOne(socialRelationForEntity->m_dominance);
	}

	//update familiarity
	if (currentEvent.second > 0.5)
	{
		socialRelationForEntity->m_familiarity += emotionFactor->m_value * currentEvent.second * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
		ClampZeroToOne(socialRelationForEntity->m_familiarity);
	}
	else
	{
		socialRelationForEntity->m_familiarity -= (1 - emotionFactor->m_value) * currentEvent.second * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
		ClampZeroToOne(socialRelationForEntity->m_familiarity);
	}

	SolidarityEvent* newSolidarityEvent = new SolidarityEvent();
	newSolidarityEvent->emotionFeelingEntity = this;
	newSolidarityEvent->emotionFelt = new EmotionFactor();
	newSolidarityEvent->emotionFelt->m_name = emotionFactor->m_name;
	newSolidarityEvent->emotionFelt->m_value = emotionFactor->m_value;
	std::string key = Stringf("%s%s%s"
		, (*currentEvent.first).m_agent->m_name.c_str(), (*currentEvent.first).m_action->m_name.c_str(), (*currentEvent.first).m_patient->m_name.c_str());
	s_solidarityEvents[key].push_back(newSolidarityEvent);
}

void Entity::UpdateSolidarity(float deltaSeconds)
{
	std::map<std::string, std::vector<SolidarityEvent*>>::iterator solidarityEventVectorIter;
	for (solidarityEventVectorIter = s_solidarityEvents.begin(); solidarityEventVectorIter != s_solidarityEvents.end(); ++solidarityEventVectorIter)
	{
		std::vector<SolidarityEvent*>& currentSolidarityEventVector = solidarityEventVectorIter->second;

		std::vector<SolidarityEvent*>::iterator solidarityEventIter;
		for (solidarityEventIter = currentSolidarityEventVector.begin(); solidarityEventIter != currentSolidarityEventVector.end(); ++solidarityEventIter)
		{
			SolidarityEvent* currentSolidarityEvent = *solidarityEventIter;

			std::vector<SolidarityEvent*>::iterator solidarityEventCompareIter;
			for (solidarityEventCompareIter = currentSolidarityEventVector.begin(); solidarityEventCompareIter != currentSolidarityEventVector.end(); ++solidarityEventCompareIter)
			{
				SolidarityEvent* newSolidarityEvent = *solidarityEventCompareIter;

				if (currentSolidarityEvent == newSolidarityEvent)
				{
					continue;
				}


				if (currentSolidarityEvent->emotionFelt->m_name == newSolidarityEvent->emotionFelt->m_name)
				{
					SocialRelation* currentSocialRelation = currentSolidarityEvent->emotionFeelingEntity->m_socialRelations[newSolidarityEvent->emotionFeelingEntity->m_name];
					currentSocialRelation->m_solidarity += currentSolidarityEvent->emotionFelt->m_value * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
					ClampZeroToOne(currentSocialRelation->m_solidarity);

					SocialRelation* newSocialRelation = newSolidarityEvent->emotionFeelingEntity->m_socialRelations[currentSolidarityEvent->emotionFeelingEntity->m_name];
					newSocialRelation->m_solidarity += newSolidarityEvent->emotionFelt->m_value * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
					ClampZeroToOne(newSocialRelation->m_solidarity);
				}
				else if (Emotion::IsOppositeEmotion(currentSolidarityEvent->emotionFelt->m_name, newSolidarityEvent->emotionFelt->m_name))
				{
					SocialRelation* currentSocialRelation = currentSolidarityEvent->emotionFeelingEntity->m_socialRelations[newSolidarityEvent->emotionFeelingEntity->m_name];
					currentSocialRelation->m_solidarity -= currentSolidarityEvent->emotionFelt->m_value * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
					ClampZeroToOne(currentSocialRelation->m_solidarity);

					SocialRelation* newSocialRelation = newSolidarityEvent->emotionFeelingEntity->m_socialRelations[currentSolidarityEvent->emotionFeelingEntity->m_name];
					newSocialRelation->m_solidarity -= newSolidarityEvent->emotionFelt->m_value * deltaSeconds * SOCIAL_RELATION_CHANGE_SPEED;
					ClampZeroToOne(newSocialRelation->m_solidarity);
				}
			}
		}
	}
}

void Entity::RemoveSolidarityEvents()
{
	std::map<std::string, std::vector<SolidarityEvent*>>::iterator solidarityEventVectorIter = s_solidarityEvents.begin();
	while (solidarityEventVectorIter != s_solidarityEvents.end())
	{
		std::vector<SolidarityEvent*>& currentSolidarityEventVector = (solidarityEventVectorIter->second);
		std::vector<SolidarityEvent*>::iterator solidarityEventIter = currentSolidarityEventVector.begin();
		while (solidarityEventIter != currentSolidarityEventVector.end())
		{
			SolidarityEvent* currentSolidarityEvent = *solidarityEventIter;
			if (currentSolidarityEvent != nullptr)
			{
				delete currentSolidarityEvent;
				currentSolidarityEvent = nullptr;
			}
			++solidarityEventIter;
		}
		currentSolidarityEventVector.clear();
		++solidarityEventVectorIter;
	}
	s_solidarityEvents.clear();
}

void Entity::RemoveEntityFromRegistry(const std::string& entityName)
{
	EntityVector::iterator entityIter;
	for (entityIter = s_entityRegistry.begin(); entityIter != s_entityRegistry.end(); ++entityIter)
	{
		Entity* currentEntity = *entityIter;
		if (currentEntity->m_name == entityName)
		{
			s_entityRegistry.erase(entityIter);
			return;
		}
	}
}

void Entity::CalculateEmotionAverages()
{
	float higherPositiveAverage = (m_personality->m_traits[AGREEABLENESS] + m_personality->m_traits[EXTRAVERSION]);
	ClampZeroToOne(higherPositiveAverage);
	float lowerPositiveAverage = (1 - m_personality->m_traits[NEUROTOCISM]);
	ClampZeroToOne(lowerPositiveAverage);
	float higherNegativeAverage = (m_personality->m_traits[NEUROTOCISM]);
	ClampZeroToOne(higherNegativeAverage);
	float lowerNegativeAverage = ((1 - m_personality->m_traits[CONSCIENTIOUSNESS]) + (1 - m_personality->m_traits[AGREEABLENESS]));
	ClampZeroToOne(lowerNegativeAverage);

	float positiveAverage = (higherPositiveAverage + lowerPositiveAverage) / 3;
	float negativeAverage = (higherNegativeAverage + lowerNegativeAverage) / 3;

	m_emotionsAverage[JOY] =			GetRandomFloatInRange(0.f, positiveAverage);
	m_emotionsAverage[HOPE] =			GetRandomFloatInRange(0.f, positiveAverage);
	m_emotionsAverage[DISAPPOINTMENT] =	GetRandomFloatInRange(0.f, negativeAverage);
	m_emotionsAverage[DISTRESS] =		GetRandomFloatInRange(0.f, negativeAverage);
	m_emotionsAverage[FEAR] =			GetRandomFloatInRange(0.f, negativeAverage);
	m_emotionsAverage[RELIEF] =			GetRandomFloatInRange(0.f, positiveAverage);
	m_emotionsAverage[PRIDE] =			GetRandomFloatInRange(0.f, positiveAverage);
	m_emotionsAverage[ADMIRATION] =		GetRandomFloatInRange(0.f, positiveAverage);
	m_emotionsAverage[SHAME] =			GetRandomFloatInRange(0.f, negativeAverage);
	m_emotionsAverage[ANGER] =			GetRandomFloatInRange(0.f, negativeAverage);

	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		m_emotionsAverage[emotionCount] = Clamp(m_emotionsAverage[emotionCount], 0.f, 1.f);
	}
}

float Entity::GetActionAttitude(Action *action)
{
	return m_actionPraises[action->m_name];
}

float Entity::CalulateIntensityThreshold(BasicEmotion emotionName)
{
	float intensityThreshold = (1 - m_personality->m_traits[NEUROTOCISM]) + (m_personality->m_traits[CONSCIENTIOUSNESS]);

	if (Emotion::IsSadEmotion(emotionName))
	{
		intensityThreshold += m_personality->m_traits[AGREEABLENESS];
		intensityThreshold /= 3.f;
	}
	else
	{
		intensityThreshold *= 0.5f;
	}

	intensityThreshold *= INTENSITY_THRESHOLD_DIMINISHER;

	intensityThreshold = GetRandomFloatInRange(0.f, intensityThreshold);

	ClampZeroToOne(intensityThreshold);
	return intensityThreshold;
}

float Entity::CalulateSaturationThreshold(BasicEmotion emotionName)
{
	float saturationPersonalityAverage =  (1 - m_personality->m_traits[NEUROTOCISM]) + (1 - m_personality->m_traits[OPENNESS]);

	if (!Emotion::IsPositiveEmotion(emotionName))
	{
		saturationPersonalityAverage += (1 + m_personality->m_traits[CONSCIENTIOUSNESS]);
		saturationPersonalityAverage /= 3;
	}
	else
	{
		saturationPersonalityAverage *= 0.5f;
	}

	float saturationThreshold = GetRandomFloatInRange(m_intensityThreshold[emotionName] + saturationPersonalityAverage, 1.f);

	ClampZeroToOne(saturationThreshold);
	return saturationThreshold;
}

void Entity::SetAllEmotionStates(float newEmotionState)
{
	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		m_emotionState[emotionCount] = newEmotionState;
	}
}

void Entity::RemovePerceivedEvents()
{
	EventIterator eventIter = m_perceivedEvents.begin();
	while (eventIter != m_perceivedEvents.end())
	{
		EventQuad& currentEvent = *eventIter;
		delete currentEvent.first;
		currentEvent.first = nullptr;
		++eventIter;
	}
	m_perceivedEvents.clear();
}

void Entity::SetRandomSocialRelation(Entity* character)
{
	SocialRelation *newRelation = new SocialRelation(SocialRole::GetRandomSocialRole());
	m_socialRelations[character->m_name] = newRelation;
}

void Entity::ApplyPersonalityToEmotionFactorForEmotionStimulus(EmotionFactor* emotionFactor)
{
	m_personality->ApplyEmotionFactorsForEmotionStimulus(emotionFactor);	
}

void Entity::DecayEmotion(float deltaSeconds, BasicEmotion emotionName)
{
	float personalityAverages = (m_personality->m_traits[NEUROTOCISM]) + (1 - m_personality->m_traits[CONSCIENTIOUSNESS]);
	if (Emotion::IsSadEmotion(emotionName))
	{
		personalityAverages += (1 - m_personality->m_traits[AGREEABLENESS]);
		personalityAverages /= 3;
	}
	else
	{
		personalityAverages *= 0.5f;
	}

	float emotionDecay = personalityAverages * deltaSeconds * DECAY_SPEED;

	if (m_emotionState[emotionName] > m_emotionsAverage[emotionName])
	{
		m_emotionsTriggered[emotionName] -= emotionDecay;
	}
	else if (m_emotionState[emotionName] < m_emotionsAverage[emotionName])
	{
		m_emotionsTriggered[emotionName] += emotionDecay;
	}
}

std::string Entity::GetExtremeBehaviorFromEmotion(BasicEmotion emotionName)
{
	if (emotionName == JOY)
	{
		return "Profess Love";
	}
	else if (emotionName == HOPE)
	{
		return "Praise";
	}
	else if (emotionName == DISAPPOINTMENT)
	{
		return "Put Head in Hand";
	}
	else if (emotionName == DISTRESS)
	{
		return "Cry";
	}
	else if (emotionName == FEAR)
	{
		return "Flee";
	}
	else if (emotionName == RELIEF)
	{
		return "Cry";
	}
	else if (emotionName == PRIDE)
	{
		return "Show Off";
	}
	else if (emotionName == ADMIRATION)
	{
		return "Awe";
	}
	else if (emotionName == SHAME)
	{
		return "Fall to Knees";
	}
	else if (emotionName == ANGER)
	{
		return "Lash Out";
	}
	return "None";
}

std::string Entity::GetBehaviorFromEmotion(BasicEmotion emotionName)
{
	if (emotionName == JOY)
	{
		return "Smile";
	}
	else if (emotionName == HOPE)
	{
		return "Cross Fingers";
	}
	else if (emotionName == DISAPPOINTMENT)
	{
		return "Sigh";
	}
	else if (emotionName == DISTRESS)
	{
		return "Yelp";
	}
	else if (emotionName == FEAR)
	{
		return "Sweat";
	}
	else if (emotionName == RELIEF)
	{
		return "Smile";
	}
	else if (emotionName == PRIDE)
	{
		return "Smirk";
	}
	else if (emotionName == ADMIRATION)
	{
		return "Congradulate";
	}
	else if (emotionName == SHAME)
	{
		return "Hang Head";
	}
	else if (emotionName == ANGER)
	{
		return "Argue";
	}
	return "None";
}

float Entity::GetCertaintyDegreeBasedOnSolidarityAndFamiliarity(Entity* solidarityEntity)
{
	float averageSolidarityAndFamiliarity = (m_socialRelations[solidarityEntity->m_name]->m_solidarity + m_socialRelations[solidarityEntity->m_name]->m_familiarity) * 0.5f;
	return GetSemiRandomCertaintyDegreeBasedOnValue(averageSolidarityAndFamiliarity);
}

float Entity::GetSemiRandomCertaintyDegreeBasedOnValue(float value)
{
	float certaintyDegree = 0.f;

	if (value == 1.f)
	{
		certaintyDegree = 1.f;
	}
	else if (value >= 0.5f)
	{
		if (GetRandomFloatZeroToOne() <= value)
		{
			certaintyDegree = 1.f;
		}
		else
		{
			certaintyDegree = value;
		}
	}
	else if (value < 0.5f && value != 0.f)
	{
		if (GetRandomFloatZeroToOne() <= value)
		{
			certaintyDegree = value;
		}
		else
		{
			certaintyDegree = 0.f;
		}
	}

	return certaintyDegree;
}

BasicEmotion Entity::GetHighestEmotion() const
{
	BasicEmotion highestEmotionName = NO_BASIC_EMOTION;
	float highestEmotionValue = FLT_MIN;
	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		if (m_emotionState[emotionCount] > highestEmotionValue)
		{
			highestEmotionName = (BasicEmotion)emotionCount;
			highestEmotionValue = m_emotionState[emotionCount];
		}
	}

	return highestEmotionName;
}

std::string Entity::GetEmotionDataString()
{
	std::string emotionData = "";

	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		emotionData += Stringf("%f,", m_emotionState[emotionCount]);
	}

	return emotionData;
}

bool Entity::AreAnySaturationThresholdsPassed() const
{
	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		if (m_isReactingExtremely[emotionCount])
			return true;
	}

	return false;
}

Entity* Entity::GetRandomEntity(bool canBeNullptr, Entity* notEntity /*= nullptr*/)
{
	int randomCharacterIndex = GetRandomIntInRange(0, (int)s_entityRegistry.size() - 1);
	if (canBeNullptr)
	{
		randomCharacterIndex = GetRandomIntInRange(0, (int)s_entityRegistry.size());
		if (randomCharacterIndex == (int)s_entityRegistry.size())
		{
			return nullptr;
		}
	}
	else
	{
		while (s_entityRegistry[randomCharacterIndex] == notEntity)
		{
			randomCharacterIndex = GetRandomIntInRange(0, (int)s_entityRegistry.size() - 1);
		}
	}

	return s_entityRegistry[randomCharacterIndex];
}

SolidarityEvent::~SolidarityEvent()
{
	emotionFeelingEntity = nullptr;
	delete emotionFelt;
	emotionFelt = nullptr;
}
