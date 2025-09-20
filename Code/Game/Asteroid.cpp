#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include "Game/Asteroid.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/Debris.hpp"

//-----------------------------------------------------------------------------------------------
Asteroid::Asteroid(Game* game)
	: Entity(game, Vec2(0,0))
{
	m_randomGenerator = RandomNumberGenerator();
	m_velocity = Vec2(m_randomGenerator.RollRandomFloatZeroToOne()*2.f-1.f, 
					  m_randomGenerator.RollRandomFloatZeroToOne()*2.f-1.f).GetNormalized();
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_angularVelocity = m_randomGenerator.RollRandomFloatInRange(-200.f,200.f);
	m_health = 3;
	SetPositionRandomOffscreen();

	m_localMesh = new Vertex[m_vertexNum];
	GetLocalMesh(m_vertexNum, m_localMesh);
}

//-----------------------------------------------------------------------------------------------
void Asteroid::Update(float deltaSeconds)
{
	//--------------------------------------------------------------------------------
	if (m_health <= 0) {
		Die();
		return;
	}
	//--------------------------------------------------------------------------------
	CollideTest();
	//--------------------------------------------------------------------------------
	if (m_position.x + m_cosmeticRadius < 0) {
		m_position.x = WORLD_SIZE_X;
	}
	else if (m_position.x - m_cosmeticRadius > WORLD_SIZE_X) {
		m_position.x = 0;
	}
	if (m_position.y + m_cosmeticRadius < 0) {
		m_position.y = WORLD_SIZE_Y;
	}
	else if (m_position.y - m_cosmeticRadius > WORLD_SIZE_Y) {
		m_position.y = 0;
	}
	//--------------------------------------------------------------------------------
	m_position += m_velocity * ASTEROID_SPEED * deltaSeconds;
	m_orientationDegrees += m_angularVelocity * deltaSeconds;
}

//-----------------------------------------------------------------------------------------------
void Asteroid::Render() const
{

	Vertex m_worldMesh[48];
	for (int i = 0; i < 48; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
	}
	TransformVertexArrayXY3D(48, m_worldMesh, 1.f, m_orientationDegrees, m_position);
	g_engine->m_renderer->DrawVertexArray(48, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void Asteroid::Die()
{
	if (!m_isDead) {
		m_velocity = m_velocity.GetNormalized() * 10.f;
		BurstDebris(m_debrisNumMin, m_debrisNumMax, -GetForwardVector(), 330.f, Rgba8(100, 100, 100, 255), 1.1f);
		m_isDead = true;
	}
}

//-----------------------------------------------------------------------------------------------
void Asteroid::GetLocalMesh(int vertexNum, Vertex* mesh) {
	GUARANTEE_OR_DIE(vertexNum == m_vertexNum, "The array you provided can not save asteroids mesh!");
	
	float randomLengths[16];
	for (int i = 0; i < 16; i++) {
		randomLengths[i] = RandomNumberGenerator().RollRandomFloatInRange(ASTEROID_PHYSICS_RADIUS,
			ASTEROID_COSMETIC_RADIUS);
	}

	// Define the asteroid mesh (in local space)
	for (int i = 0; i < 45; i += 3) {
		mesh[i] = Vertex(Vec3(randomLengths[i / 3 + 1] * CosDegrees((i / 3 + 1) / 16.f * 360.f),
			randomLengths[i / 3 + 1] * SinDegrees((i / 3 + 1) / 16.f * 360.f),
			0.f),
			Rgba8(100, 100, 100, 255),
			Vec2(0.f, 0.f)
		);
		mesh[i + 1] = Vertex(Vec3(randomLengths[i / 3] * CosDegrees((i / 3) / 16.f * 360.f),
			randomLengths[i / 3] * SinDegrees((i / 3) / 16.f * 360.f),
			0.f),
			Rgba8(100, 100, 100, 255),
			Vec2(0.f, 0.f)
		);
		mesh[i + 2] = Vertex(Vec3(0.f, 0.f, 0.f),
			Rgba8(100, 100, 100, 255),
			Vec2(0.f, 0.f)
		);
	}


	mesh[45] = Vertex(Vec3(randomLengths[0], 0.f, 0.f),
		Rgba8(100, 100, 100, 255),
		Vec2(0.f, 0.f)
	);
	mesh[46] = Vertex(Vec3(randomLengths[15] * CosDegrees(15.f / 16.f * 360.f),
		randomLengths[15] * SinDegrees(15.f / 16.f * 360.f),
		0.f),
		Rgba8(100, 100, 100, 255),
		Vec2(0.f, 0.f)
	);
	mesh[47] = Vertex(Vec3(0.f, 0.f, 0.f),
		Rgba8(100, 100, 100, 255),
		Vec2(0.f, 0.f)
	);
}

//-----------------------------------------------------------------------------------------------
void Asteroid::CollideTest() {
	if (DoDiscsOverlap(m_position, m_physicsRadius, 
					   m_game->m_player->m_position, m_game->m_player->m_physicsRadius)) {
		m_game->m_player->Die();
	}
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (m_game->m_bullets[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_physicsRadius,
				m_game->m_bullets[i]->m_position, m_game->m_bullets[i]->m_physicsRadius)) {
				m_game->m_bullets[i]->Die();
				m_health--;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------
void Asteroid::BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale) {
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