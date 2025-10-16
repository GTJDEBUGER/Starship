#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Debris.hpp"
#include "Game/ShockWave.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/BeetleEnemy.hpp"
#include "Game/WaspEnemy.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//-----------------------------------------------------------------------------------------------
Bullet::Bullet(Game* game, Vec2 startPos, Vec2 spawnDirction)
	: Entity(game, startPos)
{
	m_velocity = spawnDirction * BULLET_SPEED;
	m_orientationDegrees = spawnDirction.GetOrientationDegrees();
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_lifeTime = BULLET_LIFETIME_SECONDS;
	m_health = 1;
	m_trackTime = m_game->m_player->m_bulletTrackDuration;

	m_localMesh = new Vertex[m_vertexNum];
	GetLocalMesh(m_vertexNum, m_localMesh);
}

//-----------------------------------------------------------------------------------------------
void Bullet::Update(float deltaSeconds)
{
	m_lifeTime -= deltaSeconds;
	m_trackTime -= deltaSeconds;
	if (m_trackTime < 0.f) {
		m_trackTime = 0.f;
	}

	if(m_lifeTime <= 0.f || IsOffWorld() || m_health<=0) {
		Die();
		return;
	}
	if (m_trackTime > 0.f) {
		TrackNearestEnemy(deltaSeconds);
	}
	m_position += m_velocity * deltaSeconds;
	CheckCollide();
}

//-----------------------------------------------------------------------------------------------
void Bullet::Render() const
{
	Vertex m_worldMesh[6];
	for (int i = 0; i < 6; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
	}
	TransformVertexArrayXY3D(6, m_worldMesh, 1.f, m_orientationDegrees, m_position);
	if (m_trackTime > 0.f) {
		Rgba8 curTargetColor = Rgba8(0, 255, 0, (unsigned char)(128.f * (m_trackTime / m_game->m_player->m_bulletTrackDuration)));
		DrawRing(m_nearestPos, 5.f, curTargetColor, 0.7f);
		DrawLine(m_nearestPos + Vec2(-7.5, -7.5), m_nearestPos + Vec2(7.5, 7.5), curTargetColor, 0.7f);
		DrawLine(m_nearestPos + Vec2(-7.5, 7.5), m_nearestPos + Vec2(7.5, -7.5), curTargetColor, 0.7f);
	}
	g_engine->m_renderer->DrawVertexArray(6, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void Bullet::Die()
{
	if (!m_isDead) {
		if (m_lifeTime > 0.f && !IsOffWorld()) {
			BurstDebris(m_debrisNumMin, m_debrisNumMax, -GetForwardVector(), 30.f, Rgba8(255, 255, 0, 255), 0.2f);
			BurstShockWave(m_position, 0.2f, m_game->m_player->m_bulletExplosionRange, Rgba8(255, 200, 128, 255));
			BurstShockWave(m_position, 0.3f, m_game->m_player->m_bulletExplosionRange * 2.f / 3.f, Rgba8(255, 200, 128, 255));

			if (m_game->m_player->m_bulletExplosionRange > m_physicsRadius) {
				ReleaseShockWave();
				SoundID shockWaveSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/ShockWaveSpawn.mp3");
				g_engine->m_audio->StartSound(shockWaveSound, false, 2.0f, 0.f, 1.f);
			}

			SoundID hitSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/EnemyGetHurt.wav");
			g_engine->m_audio->StartSound(hitSound, false, 1.0f, 0.f, m_game->m_randomGenerator->RollRandomFloatInRange(0.5f,1.1f));
		}
		m_isDead = true;
	}
}

//-----------------------------------------------------------------------------------------------
void Bullet::ReleaseShockWave() {
	for (int i = 0; i < MAX_BEETLES; i++) {
		if (m_game->m_beetleEnemy[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_game->m_player->m_bulletExplosionRange,
				m_game->m_beetleEnemy[i]->m_position, m_game->m_beetleEnemy[i]->m_physicsRadius)) {
				m_game->m_beetleEnemy[i]->m_health--;
				m_game->m_beetleEnemy[i]->m_flashFraction = 1.f;
				m_game->m_beetleEnemy[i]->m_finalHitDir = (m_game->m_beetleEnemy[i]->m_position - m_position);
			}
		}
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_game->m_waspEnemy[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_game->m_player->m_bulletExplosionRange,
				m_game->m_waspEnemy[i]->m_position, m_game->m_waspEnemy[i]->m_physicsRadius)) {
				m_game->m_waspEnemy[i]->m_health--;
				m_game->m_waspEnemy[i]->m_flashFraction = 1.f;
				m_game->m_waspEnemy[i]->m_finalHitDir = (m_game->m_waspEnemy[i]->m_position - m_position);
			}
		}
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++) {
		if (m_game->m_asteroids[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_game->m_player->m_bulletExplosionRange,
				m_game->m_asteroids[i]->m_position, m_game->m_asteroids[i]->m_physicsRadius)) {
				m_game->m_asteroids[i]->m_health--;
				m_game->m_asteroids[i]->m_flashFraction = 1.f;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
void Bullet::CheckCollide() {
	for (int i = 0; i < MAX_BEETLES; i++) {
		if (m_game->m_beetleEnemy[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_physicsRadius,
				m_game->m_beetleEnemy[i]->m_position, m_game->m_beetleEnemy[i]->m_physicsRadius)) {
				m_game->m_beetleEnemy[i]->m_health--;
				m_game->m_beetleEnemy[i]->m_flashFraction = 1.f;
				m_game->m_beetleEnemy[i]->m_finalHitDir = (m_game->m_beetleEnemy[i]->m_position - m_position);
				m_health--;
			}
		}
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_game->m_waspEnemy[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_physicsRadius,
				m_game->m_waspEnemy[i]->m_position, m_game->m_waspEnemy[i]->m_physicsRadius)) {
				m_game->m_waspEnemy[i]->m_health--;
				m_game->m_waspEnemy[i]->m_flashFraction = 1.f;
				m_game->m_waspEnemy[i]->m_finalHitDir = (m_game->m_waspEnemy[i]->m_position - m_position);
				m_health--;
			}
		}
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++) {
		if (m_game->m_asteroids[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_physicsRadius,
				m_game->m_asteroids[i]->m_position, m_game->m_asteroids[i]->m_physicsRadius)) {
				m_game->m_asteroids[i]->m_health--;
				m_game->m_asteroids[i]->m_flashFraction = 1.f;
				m_health--;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
void Bullet::TrackNearestEnemy(float deltaSeconds) {
	Vec2 nearestDir = Vec2(WORLD_SIZE_X, WORLD_SIZE_Y);
	for (int i = 0; i < MAX_BEETLES; i++) {
		if (m_game->m_beetleEnemy[i] != nullptr) {
			if ((m_game->m_beetleEnemy[i]->m_position - m_position).GetLengthSquared() < nearestDir.GetLengthSquared()) {
				nearestDir = m_game->m_beetleEnemy[i]->m_position - m_position;
				m_nearestPos = m_game->m_beetleEnemy[i]->m_position;
			}
		}
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_game->m_waspEnemy[i] != nullptr) {
			if ((m_game->m_waspEnemy[i]->m_position - m_position).GetLengthSquared() < nearestDir.GetLengthSquared()) {
				nearestDir = m_game->m_waspEnemy[i]->m_position - m_position;
				m_nearestPos = m_game->m_waspEnemy[i]->m_position;
			}
		}
	}
	if (nearestDir != Vec2(WORLD_SIZE_X, WORLD_SIZE_Y)) {
		m_velocity += nearestDir.GetNormalized() * m_trackForce * deltaSeconds;
	}
}

//-----------------------------------------------------------------------------------------------
void Bullet::GetLocalMesh(int vertexNum, Vertex* mesh) {
	GUARANTEE_OR_DIE(vertexNum == m_vertexNum, "The array you provided can not save bullets mesh!");

	// define the bullet mesh (in local space)
	// A
	mesh[0] = Vertex(Vec3(0.f, 0.5f, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	mesh[1] = Vertex(Vec3(0.5f, 0.f, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	mesh[2] = Vertex(Vec3(0.f, -0.5f, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	// B
	mesh[3] = Vertex(Vec3(-2.f, 0.f, 0.f), Rgba8(255, 0, 0, 0), Vec2(0.f, 0.f));
	mesh[4] = Vertex(Vec3(0.f, 0.5f, 0.f), Rgba8(255, 0, 0, 255), Vec2(0.f, 0.f));
	mesh[5] = Vertex(Vec3(0.f, -0.5f, 0.f), Rgba8(255, 0, 0, 255), Vec2(0.f, 0.f));
}

//----------------------------------------------------------------------------------------------
void Bullet::BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale) {
	int debrisNum = m_game->m_randomGenerator->RollRandomIntInRange(numMin, numMax);
	for (int i = 0; i < debrisNum; i++) {
		int freeDebrisIndex = -1;
		for (int j = 0; j < MAX_DEBRIS; j++) {
			if (m_game->m_debris[j] == nullptr) {
				freeDebrisIndex = j;
				break;
			}
		}
		if (freeDebrisIndex > -1) {
			float randomAngle = m_game->m_randomGenerator->RollRandomFloatInRange(-burstAngle / 2.f, burstAngle / 2.f);
			float randomSpeed = m_game->m_randomGenerator->RollRandomFloatInRange(m_velocity.GetLength() * 0.2f, m_velocity.GetLength());
			m_game->m_debris[freeDebrisIndex] = new Debris(m_game, m_position,
				burstDirection.GetRotatedByDegrees(randomAngle),
				randomSpeed, color, scale);
		}
	}

}

//----------------------------------------------------------------------------------------------
void Bullet::BurstShockWave(Vec2 position, float duration, float spreadDistance, Rgba8 waveColor) {

	int freeShockWaveIndex = -1;
	for (int i = 0; i < MAX_SHOCKWAVE; i++) {
		if (m_game->m_shockWaves[i] == nullptr) {
			freeShockWaveIndex = i;
			break;
		}
	}
	if (freeShockWaveIndex > -1) {
		m_game->m_shockWaves[freeShockWaveIndex] = new ShockWave(m_game, position, duration, spreadDistance, waveColor);
	}

}