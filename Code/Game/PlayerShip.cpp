#include "PlayerShip.hpp"
#include "GameCommon.hpp"
#include "Asteroid.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"

//-----------------------------------------------------------------------------------------------
PlayerShip::PlayerShip(Game* game)
	: Entity(game, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y))
{
	m_velocity = Vec2(0,0);
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_angularVelocity = PLAYER_SHIP_TURN_SPEED;

	// Define the ship mesh (in local space)
	// A
	m_localMesh[0] = Vertex(Vec3(0.f, 2.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[1] = Vertex(Vec3(2.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[2] = Vertex(Vec3(-2.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	// B
	m_localMesh[3] = Vertex(Vec3(-2.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[4] = Vertex(Vec3(0.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[5] = Vertex(Vec3(-2.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	// C
	m_localMesh[6] = Vertex(Vec3(0.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[7] = Vertex(Vec3(0.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[8] = Vertex(Vec3(-2.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	// D
	m_localMesh[9] = Vertex(Vec3(0.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[10] = Vertex(Vec3(1.f, 0.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[11] = Vertex(Vec3(0.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	// E
	m_localMesh[12] = Vertex(Vec3(-2.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[13] = Vertex(Vec3(2.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	m_localMesh[14] = Vertex(Vec3(0.f, -2.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Update(float deltaSeconds)
{
	//--------------------------------------------------------------------------------
	for (int i = 0; i < MAX_ASTEROIDS; i++) {
		if (m_game->m_asteroids[i] != nullptr && !m_game->m_asteroids[i]->m_isDead) {
			if (IsCollidingWithAsteroid(m_game->m_asteroids[i])) {
				Die();
				return;
			}
		}
	}

	//--------------------------------------------------------------------------------
	if (m_position.x - m_physicsRadius<0) {
		m_position = lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(1, 0) * m_velocity.x;
	}
	else if (m_position.x + m_physicsRadius>WORLD_SIZE_X) {
		m_position = lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(-1, 0) * (-m_velocity.x);
	}
	else if (m_position.y-m_physicsRadius<0) {
		m_position = lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(0, 1) * m_velocity.y;
	}
	else if (m_position.y+m_physicsRadius>WORLD_SIZE_Y) {
		m_position = lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(0, -1) * (-m_velocity.y);
	}

	//--------------------------------------------------------------------------------
	m_velocity += GetForwardVector() * m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_rotationSpeed * deltaSeconds;

	//--------------------------------------------------------------------------------
	if (!IsOffScreen()) {
		lastFramePosition = m_position;
	}
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Render() const
{
	Vertex m_worldMesh[15];
	for (int i = 0; i < 15; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
	}
	TransformVertexArrayXY3D(15, m_worldMesh, 1.f, m_orientationDegrees, m_position);
	g_engine->m_renderer->DrawVertexArray(15, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Die()
{
	m_isDead = true;
}

//-----------------------------------------------------------------------------------------------
bool PlayerShip::IsCollidingWithAsteroid(Asteroid* asteroid)
{
	if (DoDiscsOverlap(m_position, m_physicsRadius, asteroid->m_position, asteroid->m_physicsRadius)) {
		return true;
	}
	return false;
}