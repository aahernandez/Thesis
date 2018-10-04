#pragma once

class Game;

class App
{
public:
	App();
	~App();

	void RunFrame();
	void Input();
	void OnExitRequested();
	bool IsQuitting();

private:
	bool m_isQuitting;
	bool m_isRapidUpdatingWithoutRendering;

	void Update(float deltaSeconds);
	void Render();
	float GetDeltaSeconds();
};

extern App* g_theApp;