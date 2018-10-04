#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix4.hpp"

class Camera2D
{
public:
	Vector2 m_position;
	float m_speed;

	Camera2D();
	~Camera2D();

	void SetCameraPosition(const Vector2& newPos);
	void Update(float deltaSeconds);
	void KeyPress(float deltaSeconds);
};