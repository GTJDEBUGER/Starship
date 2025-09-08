#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex.hpp"

//-----------------------------------------------------------------------------------------------
class Bullet : public Entity {
public:
	Bullet(Game* game, Vec2 position, Vec2 spawnDirction);
	~Bullet() override = default;
	void Update(float deltaSeconds) override;
	void Render() override;
	void Die() override;

private:
	Vertex m_localMesh[6];
	Vertex m_worldMesh[6];
	float m_lifeTime;
};