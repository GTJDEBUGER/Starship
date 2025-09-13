#pragma once
#include "Game/Entity.hpp"

class Asteroid;

//-----------------------------------------------------------------------------------------------
class PlayerShip : public Entity {
public:
	PlayerShip(Game* game);
	~PlayerShip() override = default;
	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;

	bool IsCollidingWithAsteroid(Asteroid* asteroid);

public:
	float m_acceleration = 0;
	float m_rotationSpeed = 0;

private:
	Vertex m_localMesh[15];
	Vec2 lastFramePosition;
};