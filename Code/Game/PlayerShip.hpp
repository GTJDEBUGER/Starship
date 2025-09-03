#pragma once
#include "Engine/Math/Vec2.hpp"

class PlayerShip
{
public:
	Vec2 m_position;
	Vec2 m_velocity;

public:
	PlayerShip(Vec2 const& startingPosition, Vec2 const& startingVelocity);
	~PlayerShip() = default;
	void Update(float deltaSeconds);
	void Render();
};