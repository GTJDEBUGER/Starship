#pragma once
#include "Game/Entity.hpp"

class Asteroid;

//-----------------------------------------------------------------------------------------------
class Bullet : public Entity {
public:
	Bullet(Game* game, Vec2 position, Vec2 spawnDirction);
	~Bullet() override = default;

	void             Update(float deltaSeconds) override;
	void             Render() const override;
	void             Die() override;
				     
	static void      GetLocalMesh(int vertexNum, Vertex* mesh);
				     
private:		     
	void             BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale);
	void             BurstShockWave(Vec2 position, float duration, float spreadDistance, Rgba8 waveColor);
	void             ReleaseShockWave();
	void             CheckCollide();
	void             TrackNearestEnemy(float deltaSeconds);

private:
	float            m_lifeTime     = 1.f;
	static const int m_vertexNum    = 6;
	int              m_debrisNumMin = 1;
	int              m_debrisNumMax = 3;
	float            m_trackTime    = 0.f;
	float            m_trackForce   = 200.f;
	Vec2             m_nearestPos   = m_position;
};