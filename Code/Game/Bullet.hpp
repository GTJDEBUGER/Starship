#pragma once
#include "Game/Entity.hpp"
#include "Game/Asteroid.hpp"

//-----------------------------------------------------------------------------------------------
class Bullet : public Entity {
public:
	Bullet(Game* game, Vec2 position, Vec2 spawnDirction);
	~Bullet() override = default;
	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;

	bool IsCollidingWithAsteroid(Asteroid* asteroid);

private:
	Vertex m_localMesh[6];
	float m_lifeTime;
};