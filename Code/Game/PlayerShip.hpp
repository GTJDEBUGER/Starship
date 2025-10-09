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

	static void GetLocalMesh(int vertexNum, Vertex* mesh);

public:
	float m_acceleration = 0;
	float m_rotationSpeed = 0;
	static const int m_vertexNum = 18;

private:
	void BounceCheck();
	void BurstDebris(int numMin, int numMax, Vec2 burstDirection,
		float burstAngle, Rgba8 color, float scale);

private:
	Vec2 m_lastFramePosition;
	int m_debrisNumMin = 5;
	int m_debrisNumMax = 30;
	float m_dieScreenShakeAmp = 5.f;
	float m_bounceShakeAmp = 1.f;
	size_t m_accelerateSoundPlaybackID;
};