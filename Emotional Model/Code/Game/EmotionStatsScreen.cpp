#include "Game/EmotionStatsScreen.hpp"
#include "Game/Game.hpp"
#include "Game/Character.hpp"
#include "Engine/RHI/SimpleRenderer.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Game/Behavior.hpp"
#include "Game/Map.hpp"
#include "Engine/RHI/Font.hpp"
#include "Game/AffectiveArchitecture/Personality.hpp"
#include "Game/AffectiveArchitecture/SocialRelation.hpp"

const float START_RENDER_POS_X = 1290.f;
const float START_RENDER_POS_Y = 890.f;
const float RENDER_Y_DELTA = 20.f;
const float RENDER_Y_DIVIDER_DELTA = 5.f;
const float TEXT_SIZE = 0.75f;
const RGBA TITLE_COLOR = RGBA::TEAL;
const RGBA TITLE_SECONDARY = RGBA::ORANGE;
const RGBA TEXT_COLOR = RGBA::PURPLE;
const RGBA TEXT_SECONDAY_COLOR = RGBA::ORANGE;
const RGBA TEXT_TERTIARY_COLOR = {204,255,102};
const RGBA NUMBER_COLOR = RGBA::WHITE;
const RGBA POSITIVE_COLOR = RGBA::GREEN;
const RGBA NEGATIVE_COLOR = RGBA::RED;
const RGBA SATURATION_COLOR = RGBA::YELLOW;
const RGBA UNSATURATION_COLOR = RGBA::PURPLE;
const RGBA PREDATOR_COLOR = RGBA::RED;
const RGBA PREY_COLOR = RGBA::BLUE_LIGHT;
const RGBA SELECTION_COLOR = RGBA::YELLOW;

EmotionStatsScreen::EmotionStatsScreen()
	: m_widthScreenPercentage(0.2f)
	, m_statsOption(RENDER_OPTION_EMOTION_DATA)
	, m_overviewOption(RENDER_OPTION_EMOTIONS)
	, m_isSelecting(false)
{
	m_renderPosX = START_RENDER_POS_X;
	m_startRenderPosY = START_RENDER_POS_Y;
	m_renderPosYDelta = RENDER_Y_DELTA;
	m_renderPosYDividerDelta = RENDER_Y_DIVIDER_DELTA;
	m_textSize = TEXT_SIZE;
}

EmotionStatsScreen::~EmotionStatsScreen()
{

}

void EmotionStatsScreen::Update(float deltaSeconds)
{
	deltaSeconds;
	KeyUp();
}

void EmotionStatsScreen::Render() const
{
	AABB2D newOtherSize = Game::s_gameInstance->m_orthoSize;
	float newOtherSizeWidth = newOtherSize.CalcSize().x;
	float renderPosY = m_startRenderPosY;
	newOtherSize.mins.x += newOtherSizeWidth * (1.f - Game::s_gameInstance->m_emotionStats->m_widthScreenPercentage);

	g_theSimpleRenderer->DrawQuad2D(newOtherSize.mins, newOtherSize.maxs, RGBA::BLACK);
	g_theSimpleRenderer->SetOrthoProjection(Vector2::ZERO, Vector2(Game::s_gameInstance->s_windowWidth, Game::s_gameInstance->s_windowHeight));
	RenderEmotionData(renderPosY);
}

void EmotionStatsScreen::RenderOverview(float& renderPosY) const
{
	if (m_overviewOption == RENDER_OPTION_EMOTIONS)
	{
		RenderEmotionsOverview(renderPosY);
	}
	else if (m_overviewOption == RENDER_OPTION_BEHAVIOR)
	{
		RenderBehaviorsOverview(renderPosY);
	}
}

void EmotionStatsScreen::RenderEmotionsOverview(float& renderPosY) const
{
	int firstTabSpaces = (int)(Game::s_gameInstance->m_gameMap->GetLongestCharacterNameSize(m_textSize) / TAB_SPACES) + 1;// 12;
	int secondTabSpaces = firstTabSpaces + 2;
	int thirdTabSpacesPrey = secondTabSpaces + 8;
	int thirdTabSpacesPredator = secondTabSpaces + 13;

	g_theSimpleRenderer->DrawText2D("Emotions Overview", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	std::vector<Character*>::iterator characterIter;
	for (characterIter = Game::s_gameInstance->m_gameMap->m_characters.begin(); characterIter != Game::s_gameInstance->m_gameMap->m_characters.end(); ++characterIter)
	{
		Character *currentCharacter = *characterIter;

		std::string textToRender = Stringf("%s", currentCharacter->m_name.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY)
			, m_isSelecting && currentCharacter == *m_selectionTarget ? SELECTION_COLOR : TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i-", firstTabSpaces);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), RGBA::WHITE, m_textSize);
		textToRender = Stringf("\t%i%s:", secondTabSpaces, currentCharacter->m_faction.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY)
			, currentCharacter->m_faction == "predator" ? PREDATOR_COLOR : PREY_COLOR, m_textSize);
		textToRender = Stringf("\t%i%s", currentCharacter->m_faction == "predator" ? thirdTabSpacesPredator : thirdTabSpacesPrey, Emotion::GetBasicEmotionAsString(currentCharacter->GetHighestEmotion()).c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta;
	}
}

void EmotionStatsScreen::RenderBehaviorsOverview(float& renderPosY) const
{
	int firstTabSpaces = (int)(Game::s_gameInstance->m_gameMap->GetLongestCharacterNameSize(m_textSize) / TAB_SPACES) + 1;// 12;
	int secondTabSpaces = firstTabSpaces + 2;
	int thirdTabSpacesPrey = secondTabSpaces + 8;
	int thirdTabSpacesPredator = secondTabSpaces + 13;

	g_theSimpleRenderer->DrawText2D("Behaviors Overview", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	std::vector<Character*>::iterator characterIter;
	for (characterIter = Game::s_gameInstance->m_gameMap->m_characters.begin(); characterIter != Game::s_gameInstance->m_gameMap->m_characters.end(); ++characterIter)
	{
		Character *currentCharacter = *characterIter;

		std::string textToRender = Stringf("%s", currentCharacter->m_name.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY)
			, m_isSelecting && currentCharacter == *m_selectionTarget ? SELECTION_COLOR : TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i-", firstTabSpaces);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), RGBA::WHITE, m_textSize);
		textToRender = Stringf("\t%i%s:", secondTabSpaces, currentCharacter->m_faction.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY)
			, currentCharacter->m_faction == "predator" ? PREDATOR_COLOR : PREY_COLOR, m_textSize);
		textToRender = Stringf("\t%i%s", currentCharacter->m_faction == "predator" ? thirdTabSpacesPredator : thirdTabSpacesPrey
			, currentCharacter->m_currentBehavior->m_name.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta;
	}
}

void EmotionStatsScreen::RenderNameAndStats(float& renderPosY) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;
	if (currentCharacter == nullptr)
		return;

	std::string textToRender = Stringf("%s", currentCharacter->m_name.c_str());
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), RGBA::GREEN, m_textSize);
	renderPosY -= m_renderPosYDelta;

	textToRender = Stringf("{1.0, 0.5, 0.0}Current Behavior: {-}%s", currentCharacter->m_currentBehavior->m_name.c_str());
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), RGBA::WHITE, m_textSize);
	renderPosY -= m_renderPosYDelta;

	textToRender = Stringf("{1.0, 0.5, 1.0}Position: {-}%.2f, %.2f", currentCharacter->m_bounds.m_center.x, currentCharacter->m_bounds.m_center.y);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), RGBA::WHITE, m_textSize);
	renderPosY -= m_renderPosYDelta;

	textToRender = Stringf("{1.0, 0.5, 1.0}Target:    {-}%i,      %i", currentCharacter->m_target.x, currentCharacter->m_target.y);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), RGBA::WHITE, m_textSize);
	renderPosY -= m_renderPosYDelta + m_renderPosYDividerDelta;
	renderPosY -= m_renderPosYDelta + m_renderPosYDividerDelta;
}

void EmotionStatsScreen::RenderEmotionData(float& renderPosY) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;
	if (currentCharacter == nullptr)
	{
		RenderOverview(renderPosY);
	}
	else
	{
		RenderNameAndStats(renderPosY);
		if (m_statsOption == RENDER_OPTION_EMOTION_DATA)
		{
			RenderPersonality(renderPosY);
			RenderEmotionStates(renderPosY);
			RenderEmotionAverages(renderPosY);
			RenderEmotionIntensities(renderPosY);
			RenderEmotionSaturationIntensities(renderPosY);
		}
		else if (m_statsOption == RENDER_OPTION_SOCIAL_RELATIONS)
		{
			RenderSocialRelations(renderPosY);
		}
		else if (m_statsOption == RENDER_OPTION_ACTION_EFFECTS)
		{
			RenderActionEffects(renderPosY);
		}
		else if (m_statsOption == RENDER_OPTION_ACTION_ATTITUDES)
		{
			RenderActionAttitudes(renderPosY);
		}
		else if (m_statsOption == RENDER_OPTION_EVENTS)
		{
			RenderEventsWitnessed(renderPosY);
		}
		else if (m_statsOption == RENDER_OPTION_BEHAVIOR_UTILITIES)
		{
			RenderBehaviorUtilities(renderPosY);
		}
	}

}

void EmotionStatsScreen::RenderPersonality(float& renderPosY) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;

	g_theSimpleRenderer->DrawText2D("Personality", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	for (int personalityCount = 0; personalityCount < (int)NUM_PERSONALITY_TRAITS; personalityCount++)
	{
		std::string textToRender = Stringf("%s:", Personality::GetPersonalityTraitAsString((PersonalityTrait)personalityCount).c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_TERTIARY_COLOR, m_textSize);

		textToRender = Stringf("\t24 %.2f", currentCharacter->m_personality->m_traits[personalityCount]);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta;
	}
	renderPosY -= m_renderPosYDividerDelta;
}

void EmotionStatsScreen::RenderEmotionStates(float& renderPosY) const
{
	g_theSimpleRenderer->DrawText2D("Emotional State", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	RenderEmotionStatePair(renderPosY, ADMIRATION, ANGER);
	RenderEmotionStatePair(renderPosY, PRIDE, SHAME);
	RenderEmotionStatePair(renderPosY, JOY, DISTRESS);
	RenderEmotionStatePair(renderPosY, HOPE, FEAR);
	RenderEmotionStatePair(renderPosY, RELIEF, DISAPPOINTMENT);
	renderPosY -= m_renderPosYDividerDelta;
}

void EmotionStatsScreen::RenderEmotionState(float& renderPosY, BasicEmotion basicEmotion, int startTabSpaces, int extraNumTabSpaces) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;
	int numTabSpaces = 15;

	std::string textToRender = Stringf("\t%i", startTabSpaces);
	textToRender += Emotion::GetBasicEmotionAsString(basicEmotion);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY)
		, currentCharacter->m_isReactingExtremely[basicEmotion] ? SATURATION_COLOR : UNSATURATION_COLOR, m_textSize);

	textToRender = Stringf("\t%i %.2f", startTabSpaces + numTabSpaces + extraNumTabSpaces, currentCharacter->m_emotionState[basicEmotion]);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY)
		, currentCharacter->m_hasEmotionStateGoneUp[basicEmotion] ? POSITIVE_COLOR : NEGATIVE_COLOR, m_textSize);
}

void EmotionStatsScreen::RenderEmotionStatePair(float& renderPosY, BasicEmotion firstEmotion, BasicEmotion secondEmotion) const
{
	int emotionTabSpace = 23;
	int secondNumTabSpaces = 6;
	RenderEmotionState(renderPosY, firstEmotion);
	RenderEmotionState(renderPosY, secondEmotion, emotionTabSpace, secondNumTabSpaces);
	renderPosY -= m_renderPosYDelta;
}

void EmotionStatsScreen::RenderEmotionAverages(float& renderPosY) const
{
	g_theSimpleRenderer->DrawText2D("Emotion Average", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	RenderEmotionAveragePair(renderPosY, ADMIRATION, ANGER);
	RenderEmotionAveragePair(renderPosY, PRIDE, SHAME);
	RenderEmotionAveragePair(renderPosY, JOY, DISTRESS);
	RenderEmotionAveragePair(renderPosY, HOPE, FEAR);
	RenderEmotionAveragePair(renderPosY, RELIEF, DISAPPOINTMENT);
	renderPosY -= m_renderPosYDividerDelta;
}

void EmotionStatsScreen::RenderEmotionAveragePair(float& renderPosY, BasicEmotion firstEmotion, BasicEmotion secondEmotion) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;
	int numTabSpaces = 15;
	int emotionTabSpaces = 23;
	int numTabSpaces2 = 44;

	std::string textToRender = Stringf("%s", Emotion::GetBasicEmotionAsString(firstEmotion).c_str());
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);

	textToRender = Stringf("\t%i %.2f", numTabSpaces, currentCharacter->m_emotionsAverage[firstEmotion]);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);

	textToRender = Stringf("\t%i%s", emotionTabSpaces, Emotion::GetBasicEmotionAsString(secondEmotion).c_str());
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);

	textToRender = Stringf("\t%i %.2f", numTabSpaces2, currentCharacter->m_emotionsAverage[secondEmotion]);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;
}

void EmotionStatsScreen::RenderEmotionIntensities(float& renderPosY) const
{
	g_theSimpleRenderer->DrawText2D("Intensity Thresholds", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	RenderEmotionIntensityPair(renderPosY, ADMIRATION, ANGER);
	RenderEmotionIntensityPair(renderPosY, PRIDE, SHAME);
	RenderEmotionIntensityPair(renderPosY, JOY, DISTRESS);
	RenderEmotionIntensityPair(renderPosY, HOPE, FEAR);
	RenderEmotionIntensityPair(renderPosY, RELIEF, DISAPPOINTMENT);

	renderPosY -= m_renderPosYDividerDelta;
}

void EmotionStatsScreen::RenderEmotionIntensityPair(float& renderPosY, BasicEmotion firstEmotion, BasicEmotion secondEmotion) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;
	int numTabSpaces = 15;
	int emotionTabSpaces = 23;
	int numTabSpaces2 = 44;

	std::string textToRender = Stringf("%s", Emotion::GetBasicEmotionAsString(firstEmotion).c_str());
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_SECONDAY_COLOR, m_textSize);

	textToRender = Stringf("\t%i %.2f", numTabSpaces, currentCharacter->m_intensityThreshold[firstEmotion]);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);

	textToRender = Stringf("\t%i%s", emotionTabSpaces, Emotion::GetBasicEmotionAsString(secondEmotion).c_str());
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_SECONDAY_COLOR, m_textSize);

	textToRender = Stringf("\t%i %.2f", numTabSpaces2, currentCharacter->m_intensityThreshold[secondEmotion]);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;
}

void EmotionStatsScreen::RenderEmotionSaturationIntensities(float& renderPosY) const
{
	g_theSimpleRenderer->DrawText2D("Saturation Thresholds", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	RenderEmotionSaturationIntensityPair(renderPosY, ADMIRATION, ANGER);
	RenderEmotionSaturationIntensityPair(renderPosY, PRIDE, SHAME);
	RenderEmotionSaturationIntensityPair(renderPosY, JOY, DISTRESS);
	RenderEmotionSaturationIntensityPair(renderPosY, HOPE, FEAR);
	RenderEmotionSaturationIntensityPair(renderPosY, RELIEF, DISAPPOINTMENT);

	renderPosY -= m_renderPosYDividerDelta;
}

void EmotionStatsScreen::RenderEmotionSaturationIntensityPair(float& renderPosY, BasicEmotion firstEmotion, BasicEmotion secondEmotion) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;
	int numTabSpaces = 15;
	int emotionTabSpaces = 23;
	int numTabSpaces2 = 44;

	std::string textToRender = Stringf("%s", Emotion::GetBasicEmotionAsString(firstEmotion).c_str());
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_SECONDAY_COLOR, m_textSize);

	textToRender = Stringf("\t%i %.2f", numTabSpaces, currentCharacter->m_saturationThreshold[firstEmotion]);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);

	textToRender = Stringf("\t%i%s", emotionTabSpaces, Emotion::GetBasicEmotionAsString(secondEmotion).c_str());
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_SECONDAY_COLOR, m_textSize);

	textToRender = Stringf("\t%i %.2f", numTabSpaces2, currentCharacter->m_saturationThreshold[secondEmotion]);
	g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;
}

void EmotionStatsScreen::RenderSocialRelations(float& renderPosY) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;
	int firstTabSpaces = 15;
	int secondTabSpaces = 24;
	int thirdTabSpaces = 40;

	g_theSimpleRenderer->DrawText2D("Relations", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	for (int relationIndex = 0; relationIndex < (int)Entity::s_entityRegistry.size(); relationIndex++)
	{
		Entity *currentRelationCharacter = Entity::s_entityRegistry[relationIndex];
		if (currentCharacter == currentRelationCharacter)
			continue;

		SocialRelation *socialRelation = currentCharacter->m_socialRelations[currentRelationCharacter->m_name];

		std::string textToRender = Stringf("%s", currentRelationCharacter->m_name.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TITLE_SECONDARY, m_textSize);
		renderPosY -= m_renderPosYDelta;

		textToRender = Stringf("Liking:");
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i %.2f", firstTabSpaces, socialRelation->m_liking);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		textToRender = Stringf("\t%iDominance:", secondTabSpaces);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i %.2f", thirdTabSpaces, socialRelation->m_dominance);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta;

		textToRender = Stringf("Familiarity:");
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i %.2f", firstTabSpaces, socialRelation->m_familiarity);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		textToRender = Stringf("\t%iSolidarity:", secondTabSpaces);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i %.2f", thirdTabSpaces, socialRelation->m_solidarity);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta + m_renderPosYDividerDelta;
	}
}

void EmotionStatsScreen::RenderActionAttitudes(float& renderPosY) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;

	g_theSimpleRenderer->DrawText2D("Action Attitudes", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	for (std::map<std::string, float>::iterator actionIter = currentCharacter->m_actionPraises.begin(); actionIter != currentCharacter->m_actionPraises.end(); ++actionIter)
	{
		std::string textToRender = Stringf("%s:", actionIter->first.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);

		textToRender = Stringf("\t%i %.2f", actionIter->second < 0 ? 21 : 22, actionIter->second);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta;
	}

	renderPosY -= m_renderPosYDelta + m_renderPosYDividerDelta;
}

void EmotionStatsScreen::RenderActionEffects(float& renderPosY) const
{
	g_theSimpleRenderer->DrawText2D("Action Effects", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	for (std::map<std::string, Action*>::iterator actionIter = Action::s_actionRegistry.begin(); actionIter != Action::s_actionRegistry.end(); ++actionIter)
	{
		Action* currentAction = actionIter->second;
// 		std::string textToRender = Stringf("{1.0, 1.0, 0.25}%s: {-}%.2f"
// 			, currentAction->m_name.c_str(), currentAction->m_effect);
// 		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, m_renderPosY), RGBA::WHITE, m_textSize);
// 		m_renderPosY -= m_renderPosYDelta + m_renderPosYDividerDelta;

		std::string textToRender = Stringf("%s:", currentAction->m_name.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);

		textToRender = Stringf("\t%i %.2f", currentAction->m_effect < 0 ? 21 : 22, currentAction->m_effect);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta;
	}
}

void EmotionStatsScreen::RenderEventsWitnessed(float& renderPosY) const
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;
	int firstTabSpaces = 10;
	int secondTabSpaces = 21;
	int thirdTabSpaces = 34;

	g_theSimpleRenderer->DrawText2D("Events Witnessed", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	EventIterator eventIter;
	for (eventIter = currentCharacter->m_perceivedEvents.begin(); eventIter != currentCharacter->m_perceivedEvents.end(); ++eventIter)
	{
		EventQuad& currentEvent = *eventIter;
		std::string agentName = currentEvent.first->m_agent->m_name;

		std::string textToRender = Stringf("Agent:");
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i%s", firstTabSpaces, agentName.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		textToRender = Stringf("\t%iAction:", secondTabSpaces);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i %s", thirdTabSpaces, currentEvent.first->m_action->m_name.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta;

		textToRender = Stringf("Patient:");
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i%s", firstTabSpaces, currentEvent.first->m_patient->m_name.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		textToRender = Stringf("\t%iCertainty:", secondTabSpaces);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);
		textToRender = Stringf("\t%i %.2f", thirdTabSpaces, currentEvent.second);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta + m_renderPosYDividerDelta;
	}
}

void EmotionStatsScreen::RenderBehaviorUtilities(float& renderPosY) const
{
	Character* selectedCharacter = Game::s_gameInstance->m_selectedCharacter;

	g_theSimpleRenderer->DrawText2D("Behavior Utilities", Vector2(m_renderPosX, renderPosY), TITLE_COLOR, m_textSize);
	renderPosY -= m_renderPosYDelta;

	std::vector<Behavior*>::iterator behaviorIter;
	for (behaviorIter = selectedCharacter->m_behaviors.begin(); behaviorIter != selectedCharacter->m_behaviors.end(); ++behaviorIter)
	{
		Behavior* currentBehavior = *behaviorIter;

		std::string textToRender = Stringf("%s:", currentBehavior->m_name.c_str());
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY), TEXT_COLOR, m_textSize);

		textToRender = Stringf("\t17 %.2f:", currentBehavior->m_lastCalculatedUtility);
		g_theSimpleRenderer->DrawText2D(textToRender.c_str(), Vector2(m_renderPosX, renderPosY)
			, currentBehavior == selectedCharacter->m_currentBehavior ? POSITIVE_COLOR : NUMBER_COLOR, m_textSize);
		renderPosY -= m_renderPosYDelta;
	}
}

void EmotionStatsScreen::KeyUp()
{
	Character *currentCharacter = Game::s_gameInstance->m_selectedCharacter;
	if (currentCharacter == nullptr)
	{
		if (g_theInput->WasKeyJustPressed('E'))
		{
			int renderOption = (int)m_overviewOption;
			renderOption++;
			if (renderOption == (int)NUM_OVERVIEW_RENDER_OPTION)
			{
				renderOption = 0;
			}
			m_overviewOption = (AffectiveModelOverviewRenderOption)renderOption;
		}
		else if (g_theInput->WasKeyJustPressed('Q'))
		{
			int renderOption = (int)m_overviewOption;
			renderOption--;
			if (renderOption == -1)
			{
				renderOption = (int)NUM_OVERVIEW_RENDER_OPTION - 1;
			}
			m_overviewOption = (AffectiveModelOverviewRenderOption)renderOption;
		}

		if (m_isSelecting)
		{
			if (g_theInput->WasKeyJustPressed('S'))
			{
				++m_selectionTarget;
				if (m_selectionTarget == Game::s_gameInstance->m_gameMap->m_characters.end())
				{
					m_selectionTarget = Game::s_gameInstance->m_gameMap->m_characters.begin();
				}
			}
			else if (g_theInput->WasKeyJustPressed('W'))
			{
				if (m_selectionTarget == Game::s_gameInstance->m_gameMap->m_characters.begin())
				{
					m_selectionTarget = Game::s_gameInstance->m_gameMap->m_characters.end();
				}
				--m_selectionTarget;
			}
			else if (g_theInput->WasKeyJustPressed(KEY_ENTER))
			{
				Character* selectedCharacter = *m_selectionTarget;
				Game::s_gameInstance->m_selectedCharacter = selectedCharacter;

				m_isSelecting = false;
			}
			else if (g_theInput->WasKeyJustPressed('R'))
			{
				m_isSelecting = false;
			}
		}
		else
		{
			if (g_theInput->WasKeyJustPressed('R'))
			{
				m_isSelecting = true;
				m_selectionTarget = Game::s_gameInstance->m_gameMap->m_characters.begin();
			}
		}
	}
	else
	{
		if (g_theInput->WasKeyJustPressed('E'))
		{
			int renderOption = (int)m_statsOption;
			renderOption++;
			if (renderOption == (int)NUM_STATS_RENDER_OPTIONS)
			{
				renderOption = 0;
			}
			m_statsOption = (AffectiveModelStatsRenderOption)renderOption;
		}
		else if (g_theInput->WasKeyJustPressed('Q'))
		{
			int renderOption = (int)m_statsOption;
			renderOption--;
			if (renderOption == -1)
			{
				renderOption = (int)NUM_STATS_RENDER_OPTIONS - 1;
			}
			m_statsOption = (AffectiveModelStatsRenderOption)renderOption;
		}
	}
}

AABB2D EmotionStatsScreen::GetAlteredMapOrtho()
{
	AABB2D newOtherSize = Game::s_gameInstance->m_orthoSize;
	float newOtherSizeWidth = newOtherSize.CalcSize().x;
	newOtherSize.maxs.x -= newOtherSizeWidth * Game::s_gameInstance->m_emotionStats->m_widthScreenPercentage;
	return newOtherSize;
}

