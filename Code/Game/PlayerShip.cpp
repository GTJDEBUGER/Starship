#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Game/PlayerShip.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Game.hpp"
#include "Game/Debris.hpp"

//-----------------------------------------------------------------------------------------------
PlayerShip::PlayerShip(Game* game)
	: Entity(game, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y))
{
	m_velocity = Vec2(0,0);
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_angularVelocity = PLAYER_SHIP_TURN_SPEED;

	m_localMesh = new Vertex[m_vertexNum];
	m_randomGenerator = RandomNumberGenerator();
	GetLocalMesh(m_vertexNum, m_localMesh);
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Update(float deltaSeconds)
{
	//--------------------------------------------------------------------------------
	if (m_isDead) {
		return;
	}

	//--------------------------------------------------------------------------------
	BounceCheck();
	//--------------------------------------------------------------------------------
	m_velocity += GetForwardVector() * m_acceleration * deltaSeconds;
	m_position += m_velocity * deltaSeconds;
	m_orientationDegrees += m_rotationSpeed * deltaSeconds;

	//--------------------------------------------------------------------------------
	if (!IsOffScreen()) {
		m_lastFramePosition = m_position;
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
	if (!m_isDead) {
		if (m_velocity.GetLength() > 10.f) {
			BurstDebris(m_debrisNumMin, m_debrisNumMax, GetForwardVector(), 90.f, Rgba8(102, 153, 204, 255), 1.f);
		}
		else {
			m_velocity = GetForwardVector() * 10.f;
			BurstDebris(m_debrisNumMin, m_debrisNumMax, GetForwardVector(), 360.f, Rgba8(102, 153, 204, 255), 1.f);
		}
		m_isDead = true;
	}
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::GetLocalMesh(int vertexNum, Vertex* mesh) {
	GUARANTEE_OR_DIE(vertexNum == m_vertexNum, "The array you provided can not save playerships mesh!");

	// Define the ship mesh (in local space)
	// A
	mesh[0] = Vertex(Vec3(0.f, 2.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[1] = Vertex(Vec3(2.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[2] = Vertex(Vec3(-2.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	// B
	mesh[3] = Vertex(Vec3(-2.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[4] = Vertex(Vec3(0.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[5] = Vertex(Vec3(-2.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	// C
	mesh[6] = Vertex(Vec3(0.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[7] = Vertex(Vec3(0.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[8] = Vertex(Vec3(-2.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	// D
	mesh[9] = Vertex(Vec3(0.f, 1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[10] = Vertex(Vec3(1.f, 0.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[11] = Vertex(Vec3(0.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	// E
	mesh[12] = Vertex(Vec3(-2.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[13] = Vertex(Vec3(2.f, -1.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
	mesh[14] = Vertex(Vec3(0.f, -2.f, 0.f), Rgba8(102, 153, 204, 255), Vec2(0.f, 0.f));
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::BounceCheck() {
	if (m_position.x - m_physicsRadius < 0) {
		m_position = m_lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(1, 0) * m_velocity.x;
	}

	if (m_position.x + m_physicsRadius > WORLD_SIZE_X) {
		m_position = m_lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(-1, 0) * (-m_velocity.x);
	}

	if (m_position.y - m_physicsRadius < 0) {
		m_position = m_lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(0, 1) * m_velocity.y;
	}

	if (m_position.y + m_physicsRadius > WORLD_SIZE_Y) {
		m_position = m_lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(0, -1) * (-m_velocity.y);
	}
}

//----------------------------------------------------------------------------------------------
void PlayerShip::BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale) {
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
			float randomAngle = m_randomGenerator.RollRandomFloatInRange(-burstAngle/2.f, burstAngle/2.f);
			float randomSpeed = m_randomGenerator.RollRandomFloatInRange(m_velocity.GetLength()*0.2f, m_velocity.GetLength());
			m_game->m_debris[freeDebrisIndex] = new Debris(m_game, m_position,
				burstDirection.GetRotatedByDegrees(randomAngle),
				randomSpeed, color, scale);
		}
	}

}