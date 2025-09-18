#pragma once
#include "Game/Entity.hpp"

//-----------------------------------------------------------------------------------------------
class BeetleEnemy : public Entity {
public:
	BeetleEnemy(Game* game);
	~BeetleEnemy() override = default;
	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;

public:
	float m_rotationSpeed = 0;

private:
	Vertex m_localMesh[63];
	int m_curFrame;
	int m_maxFrame;
};
