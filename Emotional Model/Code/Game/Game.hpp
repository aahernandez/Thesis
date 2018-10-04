#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Game/AffectiveArchitecture/Event.hpp"
#include "Game/AffectiveArchitecture/AffectiveCommon.hpp"

#include <string>
#include <vector>
#include <map>

const AABB2D ORIGINAL_ORTHO_SIZE(0.f, 0.f, 60.f, 33.75f);

class ShaderProgram;
class ShaderDatabase;
class Font;
class MainMenu;
class StructuredBuffer;
class ConstantBuffer;
class Texture2D;
class Sampler;
class Camera2D;
class Entity;
class Event;
class Map;
class Log;
class Character;
class EmotionStatsScreen;

enum AffectiveStep
{
	NONE,
	GENERATE_EVENT,
	OCC_APPRAISAL,
	CHANGE_EMOTION_STATE,
	BEHAVIOR,
	NUM_AFFECTIVE_STATES,
};

class Game
{
public:
	static Game *s_gameInstance;

	bool m_isPaused;
	bool m_isShowingControls;
	static int s_windowWidth;
	static int s_windowHeight;
	bool m_isRenderingMeshBuilder;
	bool m_isCollectingAnalytics;
	float m_deltaSecondsThisFrame;
	Map *m_gameMap;
	float m_scrollSpeed;
	float m_zoomSpeed;
	AABB2D m_orthoSize;
	Vector2 m_mousePositionOnWindow;
	Character *m_hoveredCharacter;
	Character *m_selectedCharacter;
	EmotionStatsScreen *m_emotionStats;

	AffectiveStep m_affectiveStep;
	Sampler *m_pointSampler;
	ShaderDatabase *m_shaderDatabase;
	ShaderProgram *m_myShader;
	StructuredBuffer *m_structuredBuffer;
	Camera2D *m_camera2D;
	Texture2D* m_faceTextures[NUM_BASIC_EMOTIONS];

	Game();
	~Game();

	void Initialize();
	void InitVariables();
	void InitEmotionFaces();
	void InitEmotionStats();
	void InitRenderer();
	void InitDevConsole();
	void InitDevConsoleFunctions();
	void InitGameMap();
	void InitAnalyticsFiles();
	void InitXMLs();
	void InitCharactersFromXML();

	void Update(float deltaSeconds);
	void UpdateDeltaSeconds(float& deltaSeconds);
	void UpdateKeyControls();
	void UpdateSimulation(float deltaSeconds);
	void UpdateGameAnalytics();
	void UpdateMousePosition();
	void UpdateDeveloperConsole(float deltaSeconds);
	void UpdateQuit();
	
	void Render();
	void RenderDefaults();
	void Render2DDefaults() const;
	void Render2D() const;
	void RenderSimulation() const;
	void RenderSimpleRendererMeshBuilder() const;
	void RenderControls() const;
	void RenderControl(std::string controlText, float& renderYPos) const;
	void RenderDevConsole() const;
	void ResetOrtho() const;
	void Present() const;
	void Destroy();

	float ChangeSimulationSpeed(float deltaSeconds) const;
	void KeyDown();
	void KeyUp();

	void FitMapInBoundsNorth();
	void FitMapInBoundsSouth();
	void FitMapInBoundsEast();
	void FitMapInBoundsWest();
	void FitMapInBounds();
	void ZoomOnMap(int scrollDelta);
	void CenterCameraOnSelectedCharacter();

	void CreateAndStoreShaderFromConfig();
	void ChangeToNextShader();
	void ChangeToPrevShader();
	void CreateAnalyticsSocialRelationsForCharacter(Character* character);
	void CreateAnalyticsStartingAttributesForCharacter(Character* character);
	void CreateAnalyticsEventsForCharacter(Character* character);
};