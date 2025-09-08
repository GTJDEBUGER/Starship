#pragma once
#include "Game.hpp"
#include "Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Vertex.hpp"

//-----------------------------------------------------------------------------------------------
class PlayerShip : public Entity {
public:
	PlayerShip(Game* game);
	~PlayerShip() override = default;
	void Update(float deltaSeconds) override;
	void Render() override;
	void Die() override;

public:
	float m_acceleration = 0;
	float m_rotationSpeed = 0;

private:
	Vertex m_localMesh[15];
	Vertex m_worldMesh[15];
	Vec2 lastFramePosition;
};