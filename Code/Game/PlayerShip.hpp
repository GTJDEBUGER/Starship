#pragma once
#include "Game/Entity.hpp"

struct PlayerUpgradeItem;
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
	void LoseHealth(float damage);
	void GetHit(Vec2& hitTargetCenter, float hitTargetRadius);
	void GainExp(float expAmount);
	void GainUpgrade(PlayerUpgradeItem upgradeItem);
	void ShockWaveAttack();

public:
	float m_acceleration = 0;
	float m_rotationSpeed = 0;
	float m_curSpeed;
	static const int m_vertexNum = 18;
	float m_flashFraction = 0.f;
	float m_fireTimer = 0.f;

	float m_expBarVal = 0.f;
	float m_nextLevelExpVal = 5.f;
	int m_curLevel = 1;
	int m_upgradeTimes = 0;

	float m_healthBarVal = 100.f;
	float m_healthBarMax = 100.f;
	float m_shieldBarVal = 0.f;
	float m_shieldBarMax = 0.f;
	float m_shieldRecoverSpeed = 1.f;
	float m_shieldExplosionRange = 0.f;
	float m_fireInterval = 0.5f;
	float m_fireBranch = 1.f;
	float m_bulletExplosionRange = 0.f;
	float m_bulletTrackDuration = 0.f;

	float m_fireAngle = 90.f;
	float m_randomFireAngle = 5.f;

	float m_invincibleDuration = 5.f;
	float m_invincibleTimer = m_invincibleDuration;
private:
	void BounceCheck();
	void BurstDebris(int numMin, int numMax, Vec2 burstDirection,
		float burstAngle, Rgba8 color, float scale);
	void BurstShockWave(Vec2 position, float duration, float spreadDistance, Rgba8 waveColor);

private:
	float m_runTimer = 0;
	Vec2 m_lastFramePosition;

	int m_debrisNumMin = 5;
	int m_debrisNumMax = 30;

	float m_dieScreenShakeAmp = 5.f;
	float m_bounceShakeAmp = 1.f;

	float m_flashFractionDecay = 2.f;
	float m_invincibleFlashSpeed = 720.f;
};