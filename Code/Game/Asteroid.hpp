#pragma once
#include "Game/Entity.hpp"

//-----------------------------------------------------------------------------------------------
class Asteroid : public Entity {
public:
	Asteroid(Game* game);
	~Asteroid() override = default;
	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;

private:
	Vertex m_localMesh[48];
};