#pragma once
#include "Entity.hpp"
#include "Engine/Core/Vertex.hpp"

//-----------------------------------------------------------------------------------------------
class Asteroid : public Entity {
public:
	Asteroid(Game* game);
	~Asteroid() override = default;
	void Update(float deltaSeconds) override;
	void Render() override;
	void Die() override;

private:
	Vertex m_localMesh[48];
	Vertex m_worldMesh[48];
};