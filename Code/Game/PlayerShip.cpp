#include "PlayerShip.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "GameCommon.hpp"
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
	m_velocity += GetForwardVector() * PLAYER_SHIP_ACCELERATION * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Render()
{
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