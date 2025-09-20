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
	void CollideTest();

	static void GetLocalMesh(int vertexNum, Vertex* mesh);

public:
	static const int m_vertexNum = 48;

private:
	void BurstDebris(int numMin, int numMax, Vec2 burstDirection,
		float burstAngle, Rgba8 color, float scale);

private:
	int m_debrisNumMin = 3;
	int m_debrisNumMax = 12;
};