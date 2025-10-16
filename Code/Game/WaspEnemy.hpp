#pragma once
#include "Game/Entity.hpp"

//-----------------------------------------------------------------------------------------------
class WaspEnemy : public Entity {
public:
	WaspEnemy(Game* game);
	~WaspEnemy() override = default;

	void             Update(float deltaSeconds) override;
	void             Render() const override;
	void             Die() override;
	void             CheckCollide();
				     
	static void      GetLocalMesh(int vertexNum, Vertex* mesh);

public:
	float            m_rotationSpeed                         = 0.f;
	static const int m_vertexNum                             = 126;
	float            m_nearbyRadius                          = 0.f;
	float            m_flashFraction                         = 0.f;
	Vec2             m_finalHitDir                           = Vec2(0,0);

private:
	void             BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale);
	void             BurstShockWave(Vec2 position, float duration, float spreadDistance, Rgba8 waveColor);
	void             FindNearbyEnemy();
	void             BoidSimulation();

private:
	float            m_deltaSeconds                          = 0;
			         
	Entity*          m_nearbyEnemy[MAX_BEETLES + MAX_WASPS];
	int              m_nearbyEnemyCount                      = 0;
	float            m_boidSeprationBoundery                 = 20.f;
	float            m_boidSeprationWeight                   = 100.f;
	float            m_boidsAlignmentWeight                  = 2.f;
	float            m_boidsCohesionWeight                   = 10.f;
	float            m_boidPlayerPullWeight                  = 60.f;
	Vec2             m_nextVelocity                          = Vec2(0, 0);
	float            m_maxSpeed                              = 0;
			         
	float            m_meshScale                             = 1.2f;
	int              m_curFrame                              = 0;
	int              m_maxFrame                              = 16;
	int              m_debrisNumMin                          = 3;
	int              m_debrisNumMax                          = 12;
	float            m_killSpeed                             = 50.f;
	float            m_bounceShakeAmp                        = 1.f;
	float            m_flashFractionDecay                    = 2.f;
	Vec2             m_beforeTeleportPos                     = Vec2(0,0);
};