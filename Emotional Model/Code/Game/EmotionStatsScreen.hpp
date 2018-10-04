#pragma once
#include "Game/AffectiveArchitecture/AffectiveCommon.hpp"
#include "Engine/Math/AABB2D.hpp"
#include <vector>

class Character;

enum AffectiveModelStatsRenderOption
{
	RENDER_OPTION_EMOTION_DATA,
	RENDER_OPTION_SOCIAL_RELATIONS,
	RENDER_OPTION_ACTION_EFFECTS,
	RENDER_OPTION_ACTION_ATTITUDES,
	RENDER_OPTION_EVENTS,
	RENDER_OPTION_BEHAVIOR_UTILITIES,
	NUM_STATS_RENDER_OPTIONS
};

enum AffectiveModelOverviewRenderOption
{
	RENDER_OPTION_EMOTIONS,
	RENDER_OPTION_BEHAVIOR,
	NUM_OVERVIEW_RENDER_OPTION
};

class EmotionStatsScreen
{
public:
	float m_widthScreenPercentage;
	float m_renderPosX;
	float m_startRenderPosY;
	float m_renderPosYDelta;
	float m_renderPosYDividerDelta;
	float m_textSize;
	bool m_isSelecting;
	std::vector<Character*>::iterator m_selectionTarget;
	AffectiveModelStatsRenderOption m_statsOption;
	AffectiveModelOverviewRenderOption m_overviewOption;

	EmotionStatsScreen();
	~EmotionStatsScreen();

	void Update(float deltaSeconds);
	void Render() const;
	void RenderOverview(float& renderPosY) const;
	void RenderEmotionsOverview(float& renderPosY) const;
	void RenderBehaviorsOverview(float& renderPosY) const;
	void RenderNameAndStats(float& renderPosY) const;
	void RenderEmotionData(float& renderPosY) const;
	void RenderPersonality(float& renderPosY) const;
	void RenderEmotionStates(float& renderPosY) const;
	void RenderEmotionState(float& renderPosY, BasicEmotion basicEmotion, int startTabSpaces = 0, int extraNumTabSpaces = 0) const;
	void RenderEmotionStatePair(float& renderPosY, BasicEmotion firstEmotion, BasicEmotion secondEmotion) const;
	void RenderEmotionAverages(float& renderPosY) const;
	void RenderEmotionAveragePair(float& renderPosY, BasicEmotion firstEmotion, BasicEmotion secondEmotion) const;
	void RenderEmotionIntensities(float& renderPosY) const;
	void RenderEmotionIntensityPair(float& renderPosY, BasicEmotion firstEmotion, BasicEmotion secondEmotion) const;
	void RenderEmotionSaturationIntensities(float& renderPosY) const;
	void RenderEmotionSaturationIntensityPair(float& renderPosY, BasicEmotion firstEmotion, BasicEmotion secondEmotion) const;
	void RenderSocialRelations(float& renderPosY) const;
	void RenderActionAttitudes(float& renderPosY) const;
	void RenderActionEffects(float& renderPosY) const;
	void RenderEventsWitnessed(float& renderPosY) const;
	void RenderBehaviorUtilities(float& renderPosY) const;

	void KeyUp();

	AABB2D GetAlteredMapOrtho();
};