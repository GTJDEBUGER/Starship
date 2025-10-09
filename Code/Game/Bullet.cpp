#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Debris.hpp"

//-----------------------------------------------------------------------------------------------
Bullet::Bullet(Game* game, Vec2 startPos, Vec2 spawnDirction)
	: Entity(game, startPos)
{
	m_velocity = spawnDirction * BULLET_SPEED;
	m_orientationDegrees = spawnDirction.GetOrientationDegrees();
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_lifeTime = BULLET_LIFETIME_SECONDS;

	m_localMesh = new Vertex[m_vertexNum];
	GetLocalMesh(m_vertexNum, m_localMesh);
}

//-----------------------------------------------------------------------------------------------
void Bullet::Update(float deltaSeconds)
{
	m_lifeTime -= deltaSeconds;
	if(m_lifeTime <= 0.f || IsOffScreen()) {
		Die();
		return;
	}

	m_position += m_velocity * deltaSeconds;
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
	g_engine->m_renderer->DrawVertexArray(6, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void Bullet::Die()
{
	if (!m_isDead) {
		if (m_lifeTime > 0.f && !IsOffScreen()) {
			BurstDebris(m_debrisNumMin, m_debrisNumMax, -GetForwardVector(), 30.f, Rgba8(255, 255, 0, 255), 0.2f);
			SoundID hitSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/EnemyGetHurt.wav");
			g_engine->m_audio->StartSound(hitSound, false, 1.0f, 0.f, m_randomGenerator.RollRandomFloatInRange(0.5f,1.1f));
		}
		m_isDead = true;
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
	int debrisNum = m_randomGenerator.RollRandomIntInRange(numMin, numMax);
	for (int i = 0; i < debrisNum; i++) {
		int freeDebrisIndex = -1;
		for (int j = 0; j < MAX_DEBRIS; j++) {
			if (m_game->m_debris[j] == nullptr) {
				freeDebrisIndex = j;
				break;
			}
		}
		if (freeDebrisIndex > -1) {
			float randomAngle = m_randomGenerator.RollRandomFloatInRange(-burstAngle / 2.f, burstAngle / 2.f);
			float randomSpeed = m_randomGenerator.RollRandomFloatInRange(m_velocity.GetLength() * 0.2f, m_velocity.GetLength());
			m_game->m_debris[freeDebrisIndex] = new Debris(m_game, m_position,
				burstDirection.GetRotatedByDegrees(randomAngle),
				randomSpeed, color, scale);
		}
	}

}