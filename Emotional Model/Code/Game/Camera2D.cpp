#include "Game/Camera2D.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DeveloperConsole.hpp"
#include "Engine/Math/MathUtilities.hpp"

Camera2D::Camera2D()
	: m_position(0.f, 0.f)
	, m_speed(1000.f)
{
}

Camera2D::~Camera2D()
{

}

void Camera2D::SetCameraPosition(const Vector2& newPos)
{
	m_position = newPos;
}

void Camera2D::Update(float deltaSeconds)
{
	KeyPress(deltaSeconds);
}

void Camera2D::KeyPress(float deltaSeconds)
{
	if (g_theDevConsole->m_isConsoleActive)
		return;

	if (g_theInput->IsKeyDown(KEY_UPARROW))
	{
		m_position.y += m_speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEY_DOWNARROW))
	{
		m_position.y -= m_speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEY_LEFTARROW))
	{
		m_position.x -= m_speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown(KEY_RIGHTARROW))
	{
		m_position.x += m_speed * deltaSeconds;
	}
}
