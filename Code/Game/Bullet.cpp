#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"

//-----------------------------------------------------------------------------------------------
Bullet::Bullet(Game* game, Vec2 startPos, Vec2 spawnDirction)
	: Entity(game, startPos)
{
	m_velocity = spawnDirction * BULLET_SPEED;
	m_orientationDegrees = spawnDirction.GetOrientationDegrees();
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_lifeTime = BULLET_LIFETIME_SECONDS;

	// define the bullet mesh (in local space)
	// A
	m_localMesh[0] = Vertex(Vec3(0.f, 0.5f, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	m_localMesh[1] = Vertex(Vec3(0.5f, 0.f, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	m_localMesh[2] = Vertex(Vec3(0.f, -0.5f, 0.f), Rgba8(255, 255, 0, 255), Vec2(0.f, 0.f));
	// B
	m_localMesh[3] = Vertex(Vec3(-2.f, 0.f, 0.f), Rgba8(255, 0, 0, 0), Vec2(0.f, 0.f));
	m_localMesh[4] = Vertex(Vec3(0.f, 0.5f, 0.f), Rgba8(255, 0, 0, 255), Vec2(0.f, 0.f));
	m_localMesh[5] = Vertex(Vec3(0.f, -0.5f, 0.f), Rgba8(255, 0, 0, 255), Vec2(0.f, 0.f));
}

//-----------------------------------------------------------------------------------------------
void Bullet::Update(float deltaSeconds)
{
	m_lifeTime -= deltaSeconds;
	if(m_lifeTime <= 0.f || IsOffScreen()) {
		Die();
		return;
	}

	for (int i = 0; i < MAX_ASTEROIDS; i++) {
		if (m_game->m_asteroids[i] != nullptr && !m_game->m_asteroids[i]->m_isDead) {
			if (IsCollidingWithAsteroid(m_game->m_asteroids[i])) {
				m_game->m_asteroids[i]->m_health -= 1;
				Die();
				break;
			}
		}
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
	m_isDead = true;
}

//-----------------------------------------------------------------------------------------------
bool Bullet::IsCollidingWithAsteroid(Asteroid* asteroid)
{
	if (DoDiscsOverlap(m_position, m_physicsRadius, asteroid->m_position, asteroid->m_physicsRadius)) {
		return true;
	}
	return false;
}