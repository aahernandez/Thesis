#include "Game/AffectiveArchitecture/EmotionModel.hpp"
#include "Game/AffectiveArchitecture/Emotion.hpp"
#include "Game/AffectiveArchitecture/Event.hpp"
#include "Game/AffectiveArchitecture/Action.hpp"
#include "Game/AffectiveArchitecture/SocialRelation.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Profiling/Profiler.hpp"
#include "ThirdParty/IXMLParser/IXMLParser.h"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DeveloperConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"

Emotion* EmotionModel::s_emotionRegistry[NUM_BASIC_EMOTIONS];

EmotionModel::EmotionModel()
{
}

EmotionModel::~EmotionModel()
{

}

BasicEmotion EmotionModel::GetEmotionTypeFromString(std::string emotionName)
{
	if (emotionName == "Joy")
		return JOY;
	if (emotionName == "Hope")
		return HOPE;
	if (emotionName == "Disappointment")
		return DISAPPOINTMENT;
	if (emotionName == "Distress")
		return DISTRESS;
	if (emotionName == "Fear")
		return FEAR;
	if (emotionName == "Relief")
		return RELIEF;
	if (emotionName == "Pride")
		return PRIDE;
	if (emotionName == "Admiration")
		return ADMIRATION;
	if (emotionName == "Shame")
		return SHAME;
	if (emotionName == "Anger")
		return ANGER;

	ASSERT_OR_DIE(false, "Emotion is not a basic emotion.");
	return NO_BASIC_EMOTION;
}

bool EmotionModel::AreAgentAndPatientSame(const EventQuad& stimulusEvent)
{
	return stimulusEvent.first->m_agent == stimulusEvent.first->m_patient;
}

bool EmotionModel::AreActionEffectAndNPCLikingBothAboveOrBelowZero(const EventQuad& stimulusEvent, Entity *entity)
{
	float attitude = entity->m_socialRelations[stimulusEvent.first->m_patient->m_name]->m_liking;
	attitude = RangeMap1D(attitude, Vector2(0.f, 1.f), Vector2(-1.f, 1.f));

	return (attitude < 0.f && stimulusEvent.first->m_action->m_effect < 0.f) || (attitude > 0.f && stimulusEvent.first->m_action->m_effect > 0.f);
}

bool EmotionModel::IsActionAttitudeGreaterZero(const EventQuad& stimulusEvent, Entity *entity)
{
	return entity->GetActionAttitude(stimulusEvent.first->m_action) > 0.f;
}

EmotionFactor* EmotionModel::CalculateEmotionalStimulusForEntity(const EventQuad& stimulusEvent, Entity *entity)
{
	if (AreAgentAndPatientSame(stimulusEvent))
	{
		if (AreActionEffectAndNPCLikingBothAboveOrBelowZero(stimulusEvent, entity))
		{
			if (stimulusEvent.second == 1.f)
			{
				return CalculateIntensity(s_emotionRegistry[JOY], stimulusEvent, entity);
			}
			else if (stimulusEvent.second == 0.f)
			{
				return CalculateIntensity(s_emotionRegistry[DISAPPOINTMENT], stimulusEvent, entity);
			}
			else
			{
				return CalculateIntensity(s_emotionRegistry[HOPE], stimulusEvent, entity);
			}
		}
		else
		{
			if (stimulusEvent.second == 1.f)
			{
				return CalculateIntensity(s_emotionRegistry[DISTRESS], stimulusEvent, entity);
			}
			else if (stimulusEvent.second == 0.f)
			{
				return CalculateIntensity(s_emotionRegistry[RELIEF], stimulusEvent, entity);
			}
			else
			{
				return CalculateIntensity(s_emotionRegistry[FEAR], stimulusEvent, entity);
			}
		}	
	}
	else
	{
		if (IsActionAttitudeGreaterZero(stimulusEvent, entity))
		{
			if (entity == stimulusEvent.first->m_agent && stimulusEvent.second == 1.f)
			{
				return CalculateIntensity(s_emotionRegistry[PRIDE], stimulusEvent, entity);
			}
			else if (stimulusEvent.second > 0.f)
			{
				return CalculateIntensity(s_emotionRegistry[ADMIRATION], stimulusEvent, entity);
			}
		}
		else
		{
			if (entity == stimulusEvent.first->m_agent && stimulusEvent.second == 1.f)
			{
				return CalculateIntensity(s_emotionRegistry[SHAME], stimulusEvent, entity);
			}
			else if (stimulusEvent.second > 0.f)
			{
				return CalculateIntensity(s_emotionRegistry[ANGER], stimulusEvent, entity);
			}
		}
	}
	return nullptr;
}

EmotionFactor* EmotionModel::CalculateIntensity(Emotion* currentEmotion, const EventQuad& stimulusEvent, Entity* entity)
{
	EmotionFactor* newEmotionFactor = new EmotionFactor();
	newEmotionFactor->m_name = currentEmotion->m_name;
	newEmotionFactor->m_value = 0.f;

	if (currentEmotion->m_name == JOY || currentEmotion->m_name == DISTRESS
		|| currentEmotion->m_name == RELIEF || currentEmotion->m_name == DISAPPOINTMENT)
	{
		float liking = entity->m_socialRelations[stimulusEvent.first->m_patient->m_name]->m_liking;
		float actionEffect = RangeMap1D(stimulusEvent.first->m_action->m_effect, Vector2(-1.f, 1.f), Vector2(0.f, 1.f));

		if (liking < 0.5f)
		{
			liking = 1 - liking; //disliking
		}
		if (actionEffect < 0.5f)
		{
			actionEffect = 1 - actionEffect; //bad effect
		}
		
		newEmotionFactor->m_value = (entity->m_socialRelations[stimulusEvent.first->m_patient->m_name]->m_liking + stimulusEvent.first->m_action->m_effect) * 0.5f;
		
		if (actionEffect == 0.5f)
		{
			newEmotionFactor->m_value = 0.f;
		}
	}
	else if (currentEmotion->m_name == HOPE || currentEmotion->m_name == FEAR)
	{
		float liking = entity->m_socialRelations[stimulusEvent.first->m_patient->m_name]->m_liking;
		float actionEffect = RangeMap1D(stimulusEvent.first->m_action->m_effect, Vector2(-1.f,1.f), Vector2(0.f,1.f));

		if (liking < 0.5f)
		{
			liking = 1 - liking; //disliking
		}
		if (actionEffect < 0.5f)
		{
			actionEffect = 1 - actionEffect; //bad effect
		}
		newEmotionFactor->m_value = (liking + actionEffect) * stimulusEvent.second;
	}
	else if (currentEmotion->m_name == PRIDE || currentEmotion->m_name == ADMIRATION
		|| currentEmotion->m_name == SHAME || currentEmotion->m_name == ANGER)
	{
		float actionAttitude = RangeMap1D(entity->GetActionAttitude(stimulusEvent.first->m_action), Vector2(-1.f, 1.f), Vector2(0.f, 1.f));
		if (actionAttitude < 0.5f)
		{
			actionAttitude = 1 - actionAttitude; //negative attitude
		}
		newEmotionFactor->m_value = actionAttitude * stimulusEvent.second;
	}

	newEmotionFactor->m_value = Clamp(newEmotionFactor->m_value, 0.f, 1.f);
	return newEmotionFactor;
}

void EmotionModel::LoadEmotionsFromXML()
{
	IXMLDomParser iDom;
	iDom.setRemoveClears(false);
	ITCXMLNode xMainNode = iDom.openFileHelper("Data/XML/Emotions.xml", "Emotions");
	ValidateXmlElement(xMainNode, "Emotion", "");

	for (int nodeCount = 0; nodeCount < xMainNode.nChildNode(); nodeCount++)
	{
		ITCXMLNode childNode = xMainNode.getChildNode(nodeCount);
		ValidateXmlElement(childNode, "Rule", "name,oppositeEmotionName");
		RegisterNewType(childNode);
	}
}

void EmotionModel::RegisterNewType(const ITCXMLNode& xmlNode)
{
	Emotion *newEmotion = new Emotion();
	newEmotion->m_name = GetEmotionTypeFromString(ParseXmlAttribute(xmlNode, "name", (std::string)"ERROR_NO_NAME_FOUND"));
	newEmotion->m_oppositeEmotionName = ParseXmlAttribute(xmlNode, "oppositeEmotionName", (std::string)"ERROR_NO_OPPOSITE_FOUND");
	int numRules = xmlNode.nChildNode("Rule");
	newEmotion->m_rules.resize(numRules);
	for (int ruleIndex = 0; ruleIndex < numRules; ruleIndex++)
	{
		ITCXMLNode ruleNode = xmlNode.getChildNode(ruleIndex);
		ValidateXmlElement(ruleNode, "", "hasAgent,isPositiveEffect,isPositiveAttitude,dc,isPositivePraise,isPraisedAgent");

		EmotionRule &currentRule = newEmotion->m_rules[ruleIndex];
		currentRule.m_hasAgent = ParseXmlAttribute(ruleNode, "hasAgent", false);
		currentRule.m_isPositiveEffect = ParseXmlAttribute(ruleNode, "isPositiveEffect", TRI_NONE);
		currentRule.m_isPositiveAttitude = ParseXmlAttribute(ruleNode, "isPositiveAttitude", TRI_NONE);
		currentRule.m_isPositivePraise = ParseXmlAttribute(ruleNode, "isPositivePraise", TRI_NONE);
		currentRule.m_isPraisedAgent = ParseXmlAttribute(ruleNode, "isPraisedAgent", TRI_NONE);
		currentRule.m_dcRange = ParseXmlAttribute(ruleNode, "dc", Range(0.f, 0.f));
	}
	s_emotionRegistry[newEmotion->m_name] = newEmotion;
}

void EmotionModel::DestroyRegistry()
{
	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		delete s_emotionRegistry[emotionCount];
		s_emotionRegistry[emotionCount] = nullptr;
	}
}
