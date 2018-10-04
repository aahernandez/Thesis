#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/RHI/SimpleRenderer.hpp"
#include "Engine/Core/Config.hpp"
#include "Engine/Core/DeveloperConsole.hpp"
#include "Game/GameConfig.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Memory/Callstack.hpp"
#include "Engine/Memory/Memory.hpp"
#include "Engine/Profiling/Profiler.hpp"
#include "Engine/Memory/JobConsumer.hpp"
#include "Engine/Memory/JobSystem.hpp"
#include "Engine/Network/NetObjectSystem.hpp"

App* g_theApp = nullptr;

const float MIN_FRAMES_PER_SECOND = 10.f;
const float MAX_FRAMES_PER_SECOND = 60.f;
const float MIN_SECONDS_PER_FRAME = (1.f / MAX_FRAMES_PER_SECOND);
const float MAX_SECONDS_PER_FRAME = (1.f / MIN_FRAMES_PER_SECOND);

App::App()
	: m_isQuitting(false)
	, m_isRapidUpdatingWithoutRendering(false)
{
	SetTagColor("debug", RGBA::GREEN);
	SetTagColor("warning", RGBA::RED);
	
	g_theConfig = new Config("Data/Configs/game.config");

	#if TRACK_MEMORY == TRACK_MEMORY_VERBOSE
		CallstackSystemInit();
	#endif

	ProfilerSystemStartup();

	g_theSimpleRenderer = new SimpleRenderer();
	int windowWidth;
	int windowHeight;
	int outputMode;
	const char *fontName;

	g_theConfig->ConfigGetInt(&windowWidth, "window_res_x");
	g_theConfig->ConfigGetInt(&windowHeight, "window_res_y");
	g_theConfig->ConfigGetInt(&outputMode, "window_style");
	g_theConfig->ConfigGetString(&fontName, "font");
	g_theSimpleRenderer->Setup((unsigned int) windowWidth, (unsigned int) windowHeight, (e_RHIOutputMode) outputMode, fontName);

	char const *title;
	g_theConfig->ConfigGetString(&title, "window_title");
	g_theSimpleRenderer->SetWindowTitle(title);

	g_theNetObjectSystem = new NetObjectSystem();

	g_theInput = new InputSystem();
// 	g_theInput->HideMouseCursor();
	Game::s_gameInstance = new Game();
// 	g_theGame = new Game();

	std::function<void(std::string args)> Func;
	Func = [&](std::string args) { OnExitRequested(); };
	g_theDevConsole->AddFunctionCommand("quit", "Exits the app", Func);

}

App::~App()
{
	delete Game::s_gameInstance;
	Game::s_gameInstance = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete g_theNetObjectSystem;
	g_theNetObjectSystem = nullptr;

	delete g_theSimpleRenderer;
	g_theSimpleRenderer = nullptr;

	delete g_theConfig;
	g_theConfig = nullptr;

	ProfilerSystemShutdown();

	#if TRACK_MEMORY == TRACK_MEMORY_VERBOSE
		MemoryProfilerLogLiveAllocations();
		CallstackSystemDestroy();
	#endif
}

void App::RunFrame()
{
	PrintReport();
	Input();
	float deltaSeconds = GetDeltaSeconds();
	Update(deltaSeconds);
	Render();
}

void App::Update(float deltaSeconds)
{
	g_theJobSystem->JobSystemMainStep();
	g_theJobSystem->JobSystemRenderStep();

	if (Game::s_gameInstance)
		Game::s_gameInstance->Update(deltaSeconds);
}

void App::Input()
{
	if (g_theInput)
		g_theInput->UpdateInputState();
	
	if (g_theInput->WasKeyJustPressed(KEY_ESCAPE))
	{
		if (g_theDevConsole->m_isConsoleActive)
		{
			if (g_theDevConsole->IsCurrentLineEmpty())
				g_theDevConsole->ToggleConsole();
			else
				g_theDevConsole->ClearCurrentLine();
		}
		else 
		{
			OnExitRequested();
		}
	}
	else if (g_theInput->WasKeyJustPressed(KEY_F9))
	{
		m_isRapidUpdatingWithoutRendering = !m_isRapidUpdatingWithoutRendering;
	}
}

void App::OnExitRequested()
{
	m_isQuitting = true;
}

bool App::IsQuitting()
{
	return m_isQuitting;
}

void App::Render()
{
	if (Game::s_gameInstance)
	{
		if (!m_isRapidUpdatingWithoutRendering)
		{
			Game::s_gameInstance->Render();
		}
		else
		{
			Game::s_gameInstance->RenderDefaults();
			Game::s_gameInstance->Present();
		}
	}
}

float App::GetDeltaSeconds()
{
	double timeNow = GetCurrentTimeSeconds();
	static double lastFrameTime = timeNow;
	double deltaSeconds = timeNow - lastFrameTime;

	if (!m_isRapidUpdatingWithoutRendering)
	{
		while (deltaSeconds < MIN_SECONDS_PER_FRAME * 0.999f)
		{
			timeNow = GetCurrentTimeSeconds();
			deltaSeconds = timeNow - lastFrameTime;
		}
		if (deltaSeconds > MAX_SECONDS_PER_FRAME)
			deltaSeconds = MAX_SECONDS_PER_FRAME;
	}
	else
	{
		deltaSeconds = MAX_SECONDS_PER_FRAME;
	}
	lastFrameTime = timeNow;

	return (float) deltaSeconds;
}
