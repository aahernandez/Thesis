#include "Engine/Renderer/Window.hpp"

#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include <shellapi.h>
#include <atlstr.h>
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/AnalyticsSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DeveloperConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Memory/Thread.hpp"
#include "Engine/Profiling/Profiler.hpp"
#include "Engine/Memory/JobSystem.hpp"
#include "Engine/Memory/LogSystem.hpp"
#include "Engine/Memory/Memory.hpp"
#include "Game/App.hpp"
#include "Engine/RHI/DX11.hpp"

#define UNUSED(x) (void)(x);

void Initialize()
{
	srand(19);
	JobSystemStartup();
	AnalyticsSystemStartup();
	LogSystemStartup();
	g_theApp = new App();
}

void Shutdown()
{
	delete g_theApp;
	g_theApp = nullptr;

	LogSystemShutdown();
	AnalyticsSystemShutdown();
	JobSystemShutdown();
}

int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE previousInstanceHandle, LPSTR commandLineString, int showCommand)
{

	UNUSED(commandLineString);
	UNUSED(applicationInstanceHandle);
	UNUSED(previousInstanceHandle);
	UNUSED(showCommand);
	Initialize();

	while (!g_theApp->IsQuitting())
	{
		g_theApp->RunFrame();
	}
	Shutdown();
	return 0;
}