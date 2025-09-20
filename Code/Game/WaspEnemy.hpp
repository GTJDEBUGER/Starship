#pragma once
#include "Game/Entity.hpp"

//-----------------------------------------------------------------------------------------------
class WaspEnemy : public Entity {
public:
	WaspEnemy(Game* game);
	~WaspEnemy() override = default;
	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;
	void CollideTest();

	static void GetLocalMesh(int vertexNum, Vertex* mesh);

public:
	float m_rotationSpeed = 0;
	static const int m_vertexNum = 126;

private:
	void BurstDebris(int numMin, int numMax, Vec2 burstDirection,
		float burstAngle, Rgba8 color, float scale);

private:
	float m_meshScale = 1.2f;
	int m_curFrame;
	int m_maxFrame;
	int m_debrisNumMin = 3;
	int m_debrisNumMax = 12;
};