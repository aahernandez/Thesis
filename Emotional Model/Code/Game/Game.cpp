#include "Game/Game.hpp"
#include "Game/Camera2D.hpp"
#include "Game/GameCommon.hpp"
#include "Game/AffectiveArchitecture/Action.hpp"
#include "Game/AffectiveArchitecture/EmotionModel.hpp"
#include "Game/AffectiveArchitecture/Personality.hpp"
#include "Game/AffectiveArchitecture/Event.hpp"
#include "Game/AffectiveArchitecture/SocialRelation.hpp"
#include "Game/AffectiveArchitecture/SocialRole.hpp"
#include "Game/Character.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapGenerator.hpp"
#include "Game/MapBuilder.hpp"
#include "Game/Map.hpp"
#include "Game/CharacterBuilder.hpp"
#include "Game/FeatureBuilder.hpp"
#include "Game/EmotionStatsScreen.hpp"
#include "Game/Behavior.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Core/Config.hpp"
#include "Engine/Core/AnalyticsSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DeveloperConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/MeshBuilder.hpp"
#include "Engine/RHI/SimpleRenderer.hpp"
#include "Engine/RHI/ShaderDatabase.hpp"
#include "Engine/Memory/Memory.hpp"
#include "Engine/Profiling/Profiler.hpp"
#include "Engine/Core/EngineConfig.hpp"

Game *Game::s_gameInstance = nullptr;
int Game::s_windowWidth = 0;
int Game::s_windowHeight = 0;

Game::Game()
	: m_isPaused(false)
	, m_isRenderingMeshBuilder(false)
	, m_isShowingControls(false)
	, m_pointSampler(nullptr)
	, m_myShader(nullptr)
	, m_shaderDatabase(nullptr)
	, m_camera2D(nullptr)
	, m_isCollectingAnalytics(false)
	, m_deltaSecondsThisFrame(0.f)
	, m_affectiveStep(NONE)
	, m_orthoSize(ORIGINAL_ORTHO_SIZE)
	, m_scrollSpeed(5.f)
	, m_zoomSpeed(0.1f)
	, m_hoveredCharacter(nullptr)
	, m_selectedCharacter(nullptr)
{
	s_gameInstance = this;
	Initialize();
}

Game::~Game()
{
	Destroy();
}

void Game::Initialize()
{
	InitVariables();
	InitRenderer();
	InitDevConsole();
	InitEmotionFaces();
	InitEmotionStats();
	InitXMLs();
	InitGameMap();
	InitCharactersFromXML();
	InitAnalyticsFiles();
}

void Game::InitVariables()
{
	g_theConfig->ConfigGetInt(&s_windowWidth, "window_res_x");
	g_theConfig->ConfigGetInt(&s_windowHeight, "window_res_y");
}

void Game::InitDevConsoleFunctions()
{
	std::function<void(std::string args)> Func;
	Func = [&](std::string args)
	{
		if (args == "instance")
		{ 
			wchar_t buffer[MAX_PATH];
			GetModuleFileName(NULL, buffer, MAX_PATH);
			std::wstring ws(buffer);
			std::string appFilePath(ws.begin(), ws.end());

			PROCESS_INFORMATION ProcessInfo;
			STARTUPINFO StartupInfo;
			ZeroMemory(&StartupInfo, sizeof(StartupInfo));
			StartupInfo.cb = sizeof StartupInfo;
			CreateProcess(buffer, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);
		}
	};
	g_theDevConsole->AddFunctionCommand("Launch", "Opens another instance of the current program if the parameter 'instance' is used.", Func);
}

void Game::InitEmotionFaces()
{
	m_faceTextures[ADMIRATION]			= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Admiration.png");
	m_faceTextures[ANGER]				= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Anger.png");
	m_faceTextures[DISAPPOINTMENT]		= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Disappointment.png");
	m_faceTextures[DISTRESS]			= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Distress.png");
	m_faceTextures[FEAR]				= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Fear.png");
	m_faceTextures[HOPE]				= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Hope.png");
	m_faceTextures[JOY]					= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Joy.png");
	m_faceTextures[NO_BASIC_EMOTION]	= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/None.png");
	m_faceTextures[PRIDE]				= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Pride.png");
	m_faceTextures[RELIEF]				= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Relief.png");
	m_faceTextures[SHAME]				= new Texture2D(g_theSimpleRenderer->m_rhiDevice, "Data/Images/Shame.png");
}

void Game::InitEmotionStats()
{
	m_emotionStats = new EmotionStatsScreen();
}

void Game::InitRenderer()
{
	m_shaderDatabase = new ShaderDatabase();
	CreateAndStoreShaderFromConfig();
	m_myShader = m_shaderDatabase->GetUntexturedShaderInMap();
	m_pointSampler = new Sampler(g_theSimpleRenderer->m_rhiDevice, FILTER_POINT, FILTER_POINT);
}

void Game::InitDevConsole()
{
	g_theDevConsole = new DeveloperConsole();
	g_theDevConsole->SetProjectionSize(Vector2(s_windowWidth, s_windowHeight), Vector2(s_windowWidth, s_windowHeight));

	InitDevConsoleFunctions();
	DebuggerPrintf("");
}

void Game::InitGameMap()
{
	m_gameMap = new Map(IntVector2(60, 60));
	MapBuilder::BuildMap("TestBuilder", m_gameMap);
	m_gameMap->Initialize();
}

void Game::InitAnalyticsFiles()
{
	if (!m_isCollectingAnalytics)
		return;

	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		std::string basicEmotionString = Emotion::GetBasicEmotionAsString((BasicEmotion)emotionCount);
		std::string emotionData = ",";

		g_theAnalyticsSystem->CreateAnalyticsFile(basicEmotionString.c_str());

		std::vector<Character*>::iterator characterIter;
		for (characterIter = m_gameMap->m_characters.begin(); characterIter != m_gameMap->m_characters.end(); ++characterIter)
		{
			Character* currentCharacter = *characterIter;
			emotionData += Stringf("%s,", currentCharacter->m_name.c_str());
		}
		g_theAnalyticsSystem->AnalyticsPrint(basicEmotionString.c_str(), emotionData.c_str());
	}

	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_gameMap->m_characters.begin(); characterIter != m_gameMap->m_characters.end(); ++characterIter)
	{
		Character* currentCharacter = *characterIter;
		CreateAnalyticsEventsForCharacter(currentCharacter);
		CreateAnalyticsStartingAttributesForCharacter(currentCharacter);
		CreateAnalyticsSocialRelationsForCharacter(currentCharacter);
	}
}

void Game::Update(float deltaSeconds)
{
	UpdateProfileMemory();
	UpdateDeltaSeconds(deltaSeconds);
	UpdateMousePosition();
	UpdateKeyControls();

	UpdateSimulation(deltaSeconds);
	UpdateGameAnalytics();
	UpdateDeveloperConsole(deltaSeconds);
}

void Game::UpdateSimulation(float deltaSeconds)
{
	Entity::RemoveSolidarityEvents();
	EntityVector::iterator entityIter;
	for (entityIter = Entity::s_entityRegistry.begin(); entityIter != Entity::s_entityRegistry.end(); ++entityIter)
	{
		Entity* currentEntity = *entityIter;
		currentEntity->RemovePerceivedEvents();
	}

	m_gameMap->Update(deltaSeconds);
	Entity::UpdateSolidarity(deltaSeconds);
	m_emotionStats->Update(deltaSeconds);

}

void Game::UpdateGameAnalytics()
{
	if (!m_isCollectingAnalytics)
		return;

	std::string timeStamp = Stringf("%f,", (float)GetCurrentTimeSeconds());

	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		std::string basicEmotionString = Emotion::GetBasicEmotionAsString((BasicEmotion)emotionCount);
		std::string emotionData = timeStamp;
		
		std::vector<Character*>::iterator characterIter;
		for (characterIter = m_gameMap->m_characters.begin(); characterIter != m_gameMap->m_characters.end(); ++characterIter)
		{
			Character* currentCharacter = *characterIter;
			emotionData += Stringf("%f,", currentCharacter->m_emotionState[emotionCount]);
		}
		g_theAnalyticsSystem->AnalyticsPrint(basicEmotionString.c_str(), emotionData.c_str());
	}

	std::vector<Character*>::iterator characterIter;
	for (characterIter = m_gameMap->m_characters.begin(); characterIter != m_gameMap->m_characters.end(); ++characterIter)
	{
		std::string socialRelationData = timeStamp;
		Character* currentCharacter = *characterIter;
		std::string fileName = Stringf("%sSocialRelations", currentCharacter->m_name.c_str());



		std::vector<Character*>::iterator socialRelationCharacterIter;
		for (socialRelationCharacterIter = m_gameMap->m_characters.begin(); socialRelationCharacterIter != m_gameMap->m_characters.end(); ++socialRelationCharacterIter)
		{
			Character* socialRelationCharacter = *socialRelationCharacterIter;
			if (currentCharacter != socialRelationCharacter)
			{
				SocialRelation* socialRelation = currentCharacter->m_socialRelations[socialRelationCharacter->m_name];
				socialRelationData += Stringf("%f,%f,%f,%f,", socialRelation->m_liking, socialRelation->m_dominance, socialRelation->m_solidarity, socialRelation->m_familiarity);
			}
		}

		g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), socialRelationData.c_str());
	}
}

void Game::UpdateMousePosition()
{
	m_mousePositionOnWindow = g_theInput->GetMouseWindowCoords();
	m_mousePositionOnWindow.y = RangeMap1D(m_mousePositionOnWindow.y, Vector2(0.f, (float)s_windowHeight), Vector2((float)s_windowHeight, 0.f));
	m_mousePositionOnWindow = RangeMap2D(m_mousePositionOnWindow, Vector2(0.f, (float)s_windowWidth), Vector2(0.f, (float)s_windowHeight), Vector2(m_orthoSize.mins.x, m_orthoSize.maxs.x), Vector2(m_orthoSize.mins.y, m_orthoSize.maxs.y));

	if (m_hoveredCharacter != nullptr)
	{
		m_hoveredCharacter = nullptr;
	}

	std::vector<Character*>::iterator characterIter = m_gameMap->m_characters.begin();
	for (characterIter = m_gameMap->m_characters.begin(); characterIter != m_gameMap->m_characters.end(); ++characterIter)
	{
		Character *currentCharacter = *characterIter;
		if (currentCharacter->m_bounds.IsPointInside(m_mousePositionOnWindow))
		{
			m_hoveredCharacter = currentCharacter;
		}
	}
}

void Game::UpdateDeveloperConsole(float deltaSeconds)
{
	g_theDevConsole->UpdateCaret(deltaSeconds);
}

void Game::UpdateQuit()
{
	g_theApp->OnExitRequested();
}

void Game::Render()
{
	RenderDefaults();
	Render2D();
	RenderSimpleRendererMeshBuilder();
	Present();
}

void Game::RenderDefaults()
{
	g_theSimpleRenderer->SetRenderTarget(nullptr);
	g_theSimpleRenderer->ClearColor(RGBA::BLACK);
	g_theSimpleRenderer->ClearDepth();
	g_theSimpleRenderer->SetViewport(0, 0, (unsigned int)s_windowWidth, (unsigned int)s_windowHeight);
	g_theSimpleRenderer->SetShader(g_theSimpleRenderer->m_unlitShader);
	g_theSimpleRenderer->SetTexture(g_theSimpleRenderer->m_whiteTexture);
	g_theSimpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
}

void Game::Render2DDefaults() const
{
	g_theSimpleRenderer->SetShader(g_theSimpleRenderer->m_lightShader);
	g_theSimpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);

	g_theSimpleRenderer->SetViewMatrixTranslationAndRotation(Vector3(), 0.f, 0.f, 0.f);
	g_theSimpleRenderer->ClearDepth();
	g_theSimpleRenderer->EnableDepthTest(false, false);
	g_theSimpleRenderer->SetOrthoProjection(m_orthoSize.mins, m_orthoSize.maxs);
}

void Game::Render2D() const
{
	Render2DDefaults();
	RenderSimulation();
	ResetOrtho();
	RenderControls();
	RenderDevConsole();
}

void Game::RenderSimulation() const
{
	m_gameMap->Render();
	m_emotionStats->Render();
}

void Game::RenderSimpleRendererMeshBuilder() const
{
	g_theSimpleRenderer->DrawMeshBuilder();
}

void Game::RenderControls() const
{
	float renderYPos = 895.f;
	if (!m_isShowingControls)
	{
		g_theSimpleRenderer->DrawQuad2D(Vector2(0.f, 865.f), Vector2(190.f, 900.f), RGBA::OVERLAY_DARK);

		RenderControl("F1 - Toggle Controls", renderYPos);
	}
	else
	{
		g_theSimpleRenderer->DrawQuad2D(Vector2(0.f, 525.f), Vector2(290.f, 900.f), RGBA::OVERLAY_DARK);

		RenderControl("F1 - Toggle Controls", renderYPos);
		RenderControl("", renderYPos);
		RenderControl("Q - Scroll backward through tabs", renderYPos);
		RenderControl("E - Scroll forward through tabs", renderYPos);
		RenderControl("", renderYPos);
		RenderControl("R - Begin selecting character", renderYPos);
		RenderControl("W - Scroll up through characters", renderYPos);
		RenderControl("S - Scroll down through characters", renderYPos);
		RenderControl("Space - Center on selected character", renderYPos);
		RenderControl("Enter - Select character", renderYPos);
		RenderControl("", renderYPos);
		RenderControl("Up - Scroll map up", renderYPos);
		RenderControl("Down - Scroll map down", renderYPos);
		RenderControl("Left - Scroll map left", renderYPos);
		RenderControl("Right - Scroll map right", renderYPos);
		RenderControl("Mouse Wheel - Zoom in/out", renderYPos);
		RenderControl("", renderYPos);
		RenderControl("P - Pause simulation", renderYPos);
	}
}

void Game::RenderControl(std::string controlText, float& renderYPos) const
{
	float renderPosX = 10.f;
	float renderPosYDelta = 20.f;
	float textSize = 0.75f;

	g_theSimpleRenderer->DrawText2D(controlText.c_str(), Vector2(renderPosX, renderYPos), RGBA::WHITE, textSize);
	renderYPos -= renderPosYDelta;
}

void Game::RenderDevConsole() const
{
	if (g_theDevConsole->m_isConsoleActive)
	{
		g_theDevConsole->Render();
		g_theDevConsole->RenderMemoryProfiler();
	}
}

void Game::ResetOrtho() const
{
	g_theSimpleRenderer->SetOrthoProjection(Vector2(0.f, 0.f), Vector2(s_windowWidth, s_windowHeight));
}

void Game::Present() const
{
	g_theSimpleRenderer->Present();
	g_theSimpleRenderer->m_meshBuilder->Clear();
}

void Game::Destroy()
{
	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	if (m_pointSampler != nullptr)
		delete m_pointSampler;
	m_pointSampler = nullptr;

	m_myShader = nullptr;

	delete m_shaderDatabase;
	m_shaderDatabase = nullptr;
}

void Game::InitXMLs()
{
	Action::LoadActionsFromXML();
	EmotionModel::LoadEmotionsFromXML();
	SocialRole::LoadSocialRolesFromXML();
	TileDefinition::LoadTilesFromXML();
	CharacterBuilder::LoadCharactersFromXML();
	MapGenerator::LoadGeneratorsFromFile();
	MapBuilder::LoadMapBuildersFromXML();
	FeatureBuilder::LoadFeaturesFromXML();
}

void Game::InitCharactersFromXML()
{
	CharacterBuilder::BuildCharacters(m_gameMap->m_characters, m_gameMap);
	
	if (m_gameMap->m_characters.size() == 3)
	{
		m_gameMap->m_characters[0]->m_emotionsAverage[ANGER] = 1.f;
		m_gameMap->m_characters[0]->m_emotionState[ANGER] = 1.f;
		m_gameMap->m_characters[0]->m_saturationThreshold[ADMIRATION] = 0.87f;
		Vector2 position = Vector2(13.5f, 42.5f);
		m_gameMap->m_characters[0]->m_bounds.m_center = position;
		Tile* tile = m_gameMap->GetTileAtPosition(position);
		m_gameMap->m_characters[0]->PutSelfOnNewTile(tile);
		m_gameMap->m_characters[0]->m_currentBehavior->OnEnd(m_gameMap->m_characters[0]);
		m_gameMap->m_characters[0]->m_currentBehavior = nullptr;

		position = Vector2(25.5f, 42.5f);
		m_gameMap->m_characters[1]->m_bounds.m_center = position;
		tile = m_gameMap->GetTileAtPosition(position);
		m_gameMap->m_characters[1]->PutSelfOnNewTile(tile);
		m_gameMap->m_characters[1]->m_currentBehavior = m_gameMap->m_characters[1]->m_behaviors[2];
		m_gameMap->m_characters[1]->m_emotionState[ANGER] = 0.9f;
		m_gameMap->m_characters[1]->m_emotionState[ADMIRATION] = 0.5f;

		position = Vector2(24.5f, 45.5f);
		m_gameMap->m_characters[2]->m_bounds.m_center = position;
		tile = m_gameMap->GetTileAtPosition(position);
		m_gameMap->m_characters[2]->PutSelfOnNewTile(tile);
		m_gameMap->m_characters[2]->m_currentBehavior = m_gameMap->m_characters[2]->m_behaviors[2];
		m_gameMap->m_characters[2]->m_emotionState[SHAME] = 0.7f;
		m_gameMap->m_characters[2]->m_emotionState[RELIEF] = 0.7f;
		m_gameMap->m_characters[2]->m_emotionState[ADMIRATION] = 0.71f;
		m_gameMap->m_characters[2]->m_emotionState[PRIDE] = 0.845f;
		m_gameMap->m_characters[2]->m_intensityThreshold[SHAME] = 0.f;
		m_gameMap->m_characters[2]->m_emotionState[ANGER] = 0.84f;
		
		m_selectedCharacter = m_gameMap->m_characters[2];

		m_deltaSecondsThisFrame = 1.f;
		ZoomOnMap(215);
		m_deltaSecondsThisFrame = 4.95f;
		FitMapInBoundsNorth();
		m_deltaSecondsThisFrame = 1.8f;
		FitMapInBoundsWest();
	}
	else
	{
		//Zooms in camera and centers it on first character
		m_selectedCharacter = m_gameMap->m_characters[0];
		m_deltaSecondsThisFrame = 1.f;
		ZoomOnMap(50);
		CenterCameraOnSelectedCharacter();
	}

}

void Game::UpdateDeltaSeconds(float& deltaSeconds)
{
	deltaSeconds = ChangeSimulationSpeed(deltaSeconds);
	m_deltaSecondsThisFrame = deltaSeconds;
}

void Game::UpdateKeyControls()
{
	KeyDown();
	KeyUp();
}

float Game::ChangeSimulationSpeed(float deltaSeconds) const
{
	if (m_isPaused)
		deltaSeconds = 0.f;

	return deltaSeconds;
}

void Game::KeyDown()
{
	if (g_theDevConsole->m_isConsoleActive)
	{
		int textChar = g_theInput->GetLastKeyCode();
		g_theDevConsole->AddCharacterToCurrentLine(textChar, g_theInput->IsKeyDown(KEY_SHIFT) ? false : true);
	}
	else
	{
		if (g_theInput->IsKeyDown(KEY_UPARROW))
		{
			FitMapInBoundsNorth();
		}
		if (g_theInput->IsKeyDown(KEY_DOWNARROW))
		{
			FitMapInBoundsSouth();
		}
		if (g_theInput->IsKeyDown(KEY_RIGHTARROW))
		{
			FitMapInBoundsEast();
		}
		if (g_theInput->IsKeyDown(KEY_LEFTARROW))
		{
			FitMapInBoundsWest();
		}

		if (g_theInput->IsKeyDown(' '))
		{
			CenterCameraOnSelectedCharacter();
		}
	}
}

void Game::KeyUp()
{
	if (g_theInput->WasKeyJustPressed(KEY_TILDE))
	{
		g_theDevConsole->ToggleConsole();
	}

	if (g_theDevConsole->m_isConsoleActive)
	{
		if (g_theInput->WasKeyJustPressed(KEY_BACKSPACE))
		{
			g_theDevConsole->RemoveLastCharacterInCurrentLine();
		}

		if (g_theInput->WasKeyJustPressed(KEY_ENTER))
		{
			g_theDevConsole->RunCommandFromCurrentLine();
		}

		if (g_theInput->IsKeyDown(KEY_CTRL))
		{
			if (g_theInput->WasKeyJustPressed('V'))
			{
				g_theDevConsole->PasteFromWindowsClipboard();
			}
		}
	}
	else
	{
		if (g_theInput->WasKeyJustPressed(MOUSE_LEFT))
		{
			if (m_selectedCharacter != nullptr)
			{
				m_selectedCharacter = nullptr;
			}

			if (m_hoveredCharacter != nullptr)
			{
				m_selectedCharacter = m_hoveredCharacter;
			}
		}

		if (g_theInput->WasKeyJustPressed(KEY_F1))
		{
			m_isShowingControls = !m_isShowingControls;
		}

		if (g_theInput->WasKeyJustPressed('P'))
		{
			m_isPaused = !m_isPaused;
		}

		if (g_theInput->WasKeyJustPressed('C'))
		{
			m_gameMap->m_maxNumCarrots++;
		}
		
		if (g_theInput->WasKeyJustPressed('X'))
		{
			m_gameMap->m_maxNumCarrots--;
		}

		if (g_theInput->WasMouseJustScrolled())
		{
			int scrollDelta = g_theInput->PopScrollDelta();
			ZoomOnMap(scrollDelta);
		}
	}
}

void Game::FitMapInBoundsNorth()
{
	m_orthoSize.maxs.y += m_scrollSpeed * m_deltaSecondsThisFrame;
	m_orthoSize.mins.y += m_scrollSpeed * m_deltaSecondsThisFrame;

	if (m_orthoSize.maxs.y > (float)m_gameMap->m_dimensions.y)
	{
		m_orthoSize.mins.y -= m_orthoSize.maxs.y - (float)m_gameMap->m_dimensions.y;
		m_orthoSize.maxs.y = (float)m_gameMap->m_dimensions.y;
	}
}

void Game::FitMapInBoundsSouth()
{
	m_orthoSize.maxs.y -= m_scrollSpeed * m_deltaSecondsThisFrame;
	m_orthoSize.mins.y -= m_scrollSpeed * m_deltaSecondsThisFrame;

	if (m_orthoSize.mins.y < 0.f)
	{
		m_orthoSize.maxs.y += -m_orthoSize.mins.y;
		m_orthoSize.mins.y = 0.f;
	}
}

void Game::FitMapInBoundsEast()
{
	m_orthoSize.maxs.x += m_scrollSpeed * m_deltaSecondsThisFrame;
	m_orthoSize.mins.x += m_scrollSpeed * m_deltaSecondsThisFrame;
	AABB2D newOrthoSize = m_emotionStats->GetAlteredMapOrtho();

	if (newOrthoSize.maxs.x > (float)m_gameMap->m_dimensions.x)
	{
		float xSize = m_orthoSize.CalcSize().x;
		m_orthoSize.maxs.x = (float)m_gameMap->m_dimensions.x + (m_orthoSize.CalcSize().x - newOrthoSize.CalcSize().x);
		m_orthoSize.mins.x = m_orthoSize.maxs.x - xSize;
	}
}

void Game::FitMapInBoundsWest()
{
	m_orthoSize.maxs.x -= m_scrollSpeed * m_deltaSecondsThisFrame;
	m_orthoSize.mins.x -= m_scrollSpeed * m_deltaSecondsThisFrame;

	if (m_orthoSize.mins.x < 0.f)
	{
		m_orthoSize.maxs.x += -m_orthoSize.mins.x;
		m_orthoSize.mins.x = 0.f;
	}
}

void Game::FitMapInBounds()
{
	FitMapInBoundsNorth();
	FitMapInBoundsSouth();
	FitMapInBoundsEast();
	FitMapInBoundsWest();
}

void Game::ZoomOnMap(int scrollDelta)
{
	m_orthoSize.mins.x += scrollDelta * m_deltaSecondsThisFrame * m_zoomSpeed;
	m_orthoSize.mins.y += scrollDelta * m_deltaSecondsThisFrame * m_zoomSpeed;
	m_orthoSize.maxs.x -= scrollDelta * m_deltaSecondsThisFrame * m_zoomSpeed;
	m_orthoSize.maxs.y -= scrollDelta * m_deltaSecondsThisFrame * m_zoomSpeed;
	m_orthoSize.SetAspectRatioPreservingWidth(16, 9);
	AABB2D newOrthoSize = m_emotionStats->GetAlteredMapOrtho();

	if (newOrthoSize.CalcSize().x > m_gameMap->m_dimensions.x)
	{
		Vector2 size = m_orthoSize.CalcSize();
		m_orthoSize.maxs.x = (float)m_gameMap->m_dimensions.x + (m_orthoSize.CalcSize().x - newOrthoSize.CalcSize().x);;
		m_orthoSize.mins.x = m_orthoSize.maxs.x - size.x;

		if (m_orthoSize.mins.x < 0.f)
		{
			m_orthoSize.mins.x = 0.f;
			m_orthoSize.maxs.x = 75.f;
		}
		m_orthoSize.SetAspectRatioPreservingWidth(16, 9);
	}
	else if (m_orthoSize.CalcSize().y > m_gameMap->m_dimensions.y)
	{
		m_orthoSize.SetCenterAndRadii(m_orthoSize.CalcCenter(), 0.f, (float)m_gameMap->m_dimensions.y * 0.5f);
		m_orthoSize.SetAspectRatioPreservingHeight(16, 9);
	}

	if (m_orthoSize.mins.y < 0.f)
	{
		m_orthoSize.maxs.y += -m_orthoSize.mins.y;
		m_orthoSize.mins.y = 0.f;
	}
	if (m_orthoSize.mins.x < 0.f)
	{
		m_orthoSize.maxs.x += -m_orthoSize.mins.x;
		m_orthoSize.mins.x = 0.f;
	}
	if (m_orthoSize.maxs.y > (float)m_gameMap->m_dimensions.y)
	{
		m_orthoSize.mins.y -= m_orthoSize.maxs.y - (float)m_gameMap->m_dimensions.y;
		m_orthoSize.maxs.y = (float)m_gameMap->m_dimensions.y;
	}

	newOrthoSize = m_emotionStats->GetAlteredMapOrtho();
	if (newOrthoSize.maxs.x > (float)m_gameMap->m_dimensions.x)
	{
		float xSize = m_orthoSize.CalcSize().x;
		m_orthoSize.maxs.x = (float)m_gameMap->m_dimensions.x + (m_orthoSize.CalcSize().x - newOrthoSize.CalcSize().x);
		m_orthoSize.mins.x = m_orthoSize.maxs.x - xSize;
	}
}

void Game::CenterCameraOnSelectedCharacter()
{
	if (m_selectedCharacter != nullptr)
	{
		m_orthoSize.SetCenter(m_selectedCharacter->m_bounds.m_center);
		FitMapInBounds();
	}
}

void Game::CreateAndStoreShaderFromConfig()
{
	int varNum = 1;
	std::string shaderString = "shader";
	std::string varName = shaderString;
	varName += std::to_string(varNum);
	while (g_theConfig->IsConfigSet(varName.c_str()))
	{
		char const *shaderFileName;
		g_theConfig->ConfigGetString(&shaderFileName, varName.c_str());
		m_shaderDatabase->CreateShaderFromFile(g_theSimpleRenderer->m_rhiDevice, shaderFileName);
		
		varName = shaderString;
		varNum++;
		varName += std::to_string(varNum);
	}
}

void Game::ChangeToNextShader()
{
	m_myShader = m_shaderDatabase->GetNextShaderInMap();
}

void Game::ChangeToPrevShader()
{
	m_myShader = m_shaderDatabase->GetPreviousShaderInMap();
}

void Game::CreateAnalyticsSocialRelationsForCharacter(Character* character)
{
	std::string fileName = Stringf("%sSocialRelations", character->m_name.c_str());
	g_theAnalyticsSystem->CreateAnalyticsFile(fileName.c_str());
	std::string socialRelationData = ",";

	std::vector<Character*>::iterator socialRelationCharacterIter;
	for (socialRelationCharacterIter = m_gameMap->m_characters.begin(); socialRelationCharacterIter != m_gameMap->m_characters.end(); ++socialRelationCharacterIter)
	{
		Character* socialRelationCharacter = *socialRelationCharacterIter;
		if (character != socialRelationCharacter)
		{
			socialRelationData += Stringf("%s Liking,%s Dominance,%s Solidarity,%s Familiarity,", socialRelationCharacter->m_name.c_str(), socialRelationCharacter->m_name.c_str(), socialRelationCharacter->m_name.c_str(), socialRelationCharacter->m_name.c_str());
		}
	}

	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), socialRelationData.c_str());
}

void Game::CreateAnalyticsStartingAttributesForCharacter(Character* character)
{
	std::string fileName = Stringf("%sAttributes", character->m_name.c_str());
	g_theAnalyticsSystem->CreateAnalyticsFile(fileName.c_str());
	std::string firstLineData = "Faction,";
	std::string secondLineData = Stringf("%s,", character->m_faction.c_str());

	for (int traitCount = 0; traitCount < (int)NUM_PERSONALITY_TRAITS; traitCount++)
	{
		firstLineData += Stringf("%s,", Personality::GetPersonalityTraitAsString((PersonalityTrait)traitCount).c_str());
		secondLineData += Stringf("%f,", character->m_personality->m_traits[traitCount]);
	}
	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), firstLineData.c_str());
	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), secondLineData.c_str());
	firstLineData.clear();
	secondLineData.clear();

	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		std::string basicEmotionString = Emotion::GetBasicEmotionAsString((BasicEmotion)emotionCount);

		firstLineData += Stringf("%s Average,", basicEmotionString.c_str());
		secondLineData += Stringf("%f,", character->m_emotionsAverage[emotionCount]);
	}
	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), firstLineData.c_str());
	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), secondLineData.c_str());
	firstLineData.clear();
	secondLineData.clear();

	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		std::string basicEmotionString = Emotion::GetBasicEmotionAsString((BasicEmotion)emotionCount);
		
		firstLineData += Stringf("%s Intensity,", basicEmotionString.c_str());
		secondLineData += Stringf("%f,", character->m_intensityThreshold[emotionCount]);
	}
	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), firstLineData.c_str());
	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), secondLineData.c_str());
	firstLineData.clear();
	secondLineData.clear();

	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		std::string basicEmotionString = Emotion::GetBasicEmotionAsString((BasicEmotion)emotionCount);

		firstLineData += Stringf("%s Saturation,", basicEmotionString.c_str());
		secondLineData += Stringf("%f,", character->m_saturationThreshold[emotionCount]);
	}
	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), firstLineData.c_str());
	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), secondLineData.c_str());
	firstLineData.clear();
	secondLineData.clear();
}

void Game::CreateAnalyticsEventsForCharacter(Character* character)
{
	std::string fileName = Stringf("%sEvents", character->m_name.c_str());
	g_theAnalyticsSystem->CreateAnalyticsFile(fileName.c_str());
	std::string emotionData = ",Actor,Action,Patient,DegreeOfCertainty,Emotion,Intensity,Saturation,";

	for (int emotionCount = 0; emotionCount < (int)NUM_BASIC_EMOTIONS; emotionCount++)
	{
		emotionData += Emotion::GetBasicEmotionAsString((BasicEmotion)emotionCount);
		emotionData += ',';
	}

	g_theAnalyticsSystem->AnalyticsPrint(fileName.c_str(), emotionData.c_str());
}
