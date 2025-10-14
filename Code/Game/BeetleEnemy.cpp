#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Game/BeetleEnemy.hpp"
#include "Game/WaspEnemy.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Playership.hpp"
#include "Game/Bullet.hpp"
#include "Game/Game.hpp"
#include "Game/Debris.hpp"
#include "Game/ShockWave.hpp"

//-----------------------------------------------------------------------------------------------
BeetleEnemy::BeetleEnemy(Game* game)
	: Entity(game, Vec2(0, 0))
{
	m_physicsRadius = ENEMY_BEETLE_PHYSICS_RADIUS * m_meshScale;
	m_cosmeticRadius = ENEMY_BEETLE_COSMETIC_RADIUS * m_meshScale;
	m_nearbyRadius = ENEMY_BEETLE_NEARBY_RADIUS * m_meshScale;
	m_health = 3;
	m_maxSpeed = m_randomGenerator.RollRandomFloatInRange(ENEMY_BEETLE_SPEED*0.8, ENEMY_BEETLE_SPEED*1.2);

	//SetPositionRandomOffWorld();
	SetPositionRandomOffScreen(m_game->m_player->m_position);
	
	m_localMesh = new Vertex[m_vertexNum];
	GetLocalMesh(m_vertexNum, m_localMesh);
}

//-----------------------------------------------------------------------------------------------
void BeetleEnemy::Update(float deltaSeconds)
{
	if (m_health <= 0) {
		Die();
		return;
	}
	m_deltaSeconds = deltaSeconds;
	//--------------------------------------------------------------------------------
	CollideTest();
	FindNearbyEnemy();
	BoidSimulation();

	//--------------------------------------------------------------------------------
	m_curFrame = (m_curFrame + 1) % m_maxFrame;
	m_flashFraction = GetClamped(m_flashFraction - m_flashFractionDecay * deltaSeconds, 0.f, 1.f);

	//--------------------------------------------------------------------------------
	m_orientationDegrees = m_velocity.GetOrientationDegrees();
	m_position += m_velocity * deltaSeconds;
	m_beforeTeleportPos = m_position;

	if (BoundaryTeleport()) {
		BurstShockWave(m_beforeTeleportPos, 0.2f, 20.f, Rgba8(255, 0, 200, 255));
		BurstShockWave(m_beforeTeleportPos, 0.4f, 10.f, Rgba8(200, 0, 145, 255));
	}
	m_velocity = m_nextVelocity;
}

//-----------------------------------------------------------------------------------------------
void BeetleEnemy::Render() const
{
	Vertex m_worldMesh[m_vertexNum];
	for (int i = 0; i < m_vertexNum - 6; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
		m_worldMesh[i].m_position *= m_meshScale;
		m_worldMesh[i].m_color = Rgba8((unsigned char)Interpolate(m_worldMesh[i].m_color.r, 255.f, m_flashFraction),
									   (unsigned char)Interpolate(m_worldMesh[i].m_color.g, 255.f, m_flashFraction),
									   (unsigned char)Interpolate(m_worldMesh[i].m_color.b, 255.f, m_flashFraction),
									   m_worldMesh[i].m_color.a);
	}

	float frameFraction = static_cast<float>(m_curFrame) / static_cast<float>(m_maxFrame);

	if (m_curFrame <= m_maxFrame / 2) {
		for (int i = m_vertexNum - 12; i < m_vertexNum - 7; i++) {
			m_worldMesh[i] = Interpolate(m_localMesh[i], m_localMesh[i + 6], frameFraction * 2.f);
		}
	}
	else {
		for (int i = m_vertexNum - 12; i < m_vertexNum - 7; i++) {
			m_worldMesh[i] = Interpolate(m_localMesh[i+6], m_localMesh[i], (frameFraction - 0.5f) * 2.f);
		}
	}

	TransformVertexArrayXY3D(m_vertexNum-6, m_worldMesh, m_meshScale, m_orientationDegrees, m_position);
	g_engine->m_renderer->DrawVertexArray(m_vertexNum-6, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void BeetleEnemy::Die()
{
	if (!m_isDead) {
		m_game->m_player->GainExp(2.f);
		BurstDebris(m_debrisNumMin, m_debrisNumMax, m_finalHitDir, 180.f, Rgba8(161, 67, 5, 255), 1.f);
		BurstDebris(m_debrisNumMin, m_debrisNumMax, m_finalHitDir, 90.f, Rgba8(255, 0, 0, 255), 0.5f);
		BurstShockWave(m_position, 0.2f, 20.f, Rgba8(255, 0, 0, 255));
		BurstShockWave(m_position, 0.4f, 10.f, Rgba8(200, 0, 0, 255));
		SoundID dieSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/EnemyDie.wav");
		g_engine->m_audio->StartSound(dieSound, false, 1.1f, 0.f, m_randomGenerator.RollRandomFloatInRange(0.8f,1.1f));
		m_isDead = true;
	}
}

//-----------------------------------------------------------------------------------------------
void BeetleEnemy::GetLocalMesh(int vertexNum, Vertex* mesh) {
	GUARANTEE_OR_DIE(vertexNum == m_vertexNum, "The array you provided can not save beetles mesh!");

	// Define the Beetle mesh (in local space)
	Rgba8 carapaceColor = Rgba8(161, 67, 5, 255);
	Rgba8 muscleColor = Rgba8(245, 153, 92, 255);
	Rgba8 wingColor = Rgba8(233, 233, 233, 128);
	// Head_Base
	mesh[0] = Vertex(Vec3(0.5f, 0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[1] = Vertex(Vec3(1.18f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[2] = Vertex(Vec3(0.5f, -0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Head_Horn_Base
	mesh[3] = Vertex(Vec3(1.f, 0.24f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[4] = Vertex(Vec3(1.7f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[5] = Vertex(Vec3(1.f, -0.24f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Head_Horn_Up
	mesh[6] = Vertex(Vec3(1.4f, 0.1f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[7] = Vertex(Vec3(1.94f, 0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[8] = Vertex(Vec3(1.7f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Head_Horn_Down
	mesh[9] = Vertex(Vec3(1.4f, -0.1f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[10] = Vertex(Vec3(1.7f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[11] = Vertex(Vec3(1.94f, -0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Body_Base
	mesh[12] = Vertex(Vec3(-1.5f, 0.f, 0.f), muscleColor, Vec2(0.f, 0.f));
	mesh[13] = Vertex(Vec3(0.5f, 0.9f, 0.f), muscleColor, Vec2(0.f, 0.f));
	mesh[14] = Vertex(Vec3(0.5f, -0.9f, 0.f), muscleColor, Vec2(0.f, 0.f));
	// Leg_Front_Left
	mesh[15] = Vertex(Vec3(0.5f, 0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[16] = Vertex(Vec3(1.f, 1.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[17] = Vertex(Vec3(0.67f, 0.67f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Front_Left
	mesh[18] = Vertex(Vec3(1.f, 1.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[19] = Vertex(Vec3(1.29f, 0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[20] = Vertex(Vec3(0.86f, 0.86f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Front_Right
	mesh[21] = Vertex(Vec3(0.5f, -0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[22] = Vertex(Vec3(1.f, -1.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[23] = Vertex(Vec3(0.67f, -0.67f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Front_Right
	mesh[24] = Vertex(Vec3(1.f, -1.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[25] = Vertex(Vec3(1.29f, -0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[26] = Vertex(Vec3(0.86f, -0.86f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Left
	mesh[27] = Vertex(Vec3(-0.4f, 0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[28] = Vertex(Vec3(-0.8f, 0.31f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[29] = Vertex(Vec3(-1.1f, 0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Left
	mesh[30] = Vertex(Vec3(-1.1f, 0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[31] = Vertex(Vec3(-0.8f, 0.31f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[32] = Vertex(Vec3(-1.11f, 0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Left
	mesh[33] = Vertex(Vec3(-1.1f, 0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[34] = Vertex(Vec3(-1.11f, 0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[35] = Vertex(Vec3(-1.8f, 0.49f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Right
	mesh[36] = Vertex(Vec3(-0.4f, -0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[37] = Vertex(Vec3(-0.8f, -0.31f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[38] = Vertex(Vec3(-1.1f, -0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Right
	mesh[39] = Vertex(Vec3(-1.1f, -0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[40] = Vertex(Vec3(-0.8f, -0.31f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[41] = Vertex(Vec3(-1.11f, -0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Leg_Back_Right
	mesh[42] = Vertex(Vec3(-1.1f, -0.7f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[43] = Vertex(Vec3(-1.11f, -0.5f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[44] = Vertex(Vec3(-1.8f, -0.49f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Wing_Left_Front
	mesh[45] = Vertex(Vec3(0.5f, 0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[46] = Vertex(Vec3(0.5f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[47] = Vertex(Vec3(-0.5f, 1.94f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Wing_Right_Front
	mesh[48] = Vertex(Vec3(0.5f, -0.9f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[49] = Vertex(Vec3(0.5f, 0.f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	mesh[50] = Vertex(Vec3(-0.5f, -1.94f, 0.f), carapaceColor, Vec2(0.f, 0.f));
	// Wing_Left_Back_Frame_1
	mesh[51] = Vertex(Vec3(0.5f, 0.f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[52] = Vertex(Vec3(-1.3f, 1.51f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[53] = Vertex(Vec3(0.f, 0.97f, 0.f), wingColor, Vec2(0.f, 0.f));
	// Wing_Right_Back_Frame_1
	mesh[54] = Vertex(Vec3(0.5f, 0.f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[55] = Vertex(Vec3(-1.3f, -1.51f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[56] = Vertex(Vec3(0.f, -0.97f, 0.f), wingColor, Vec2(0.f, 0.f));
	// Wing_Left_Back_Frame_2
	mesh[57] = Vertex(Vec3(0.5f, 0.f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[58] = Vertex(Vec3(-1.79f, 0.88f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[59] = Vertex(Vec3(0.f, 0.97f, 0.f), wingColor, Vec2(0.f, 0.f));
	// Wing_Right_Back_Frame_2
	mesh[60] = Vertex(Vec3(0.5f, 0.f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[61] = Vertex(Vec3(-1.79f, -0.88f, 0.f), wingColor, Vec2(0.f, 0.f));
	mesh[62] = Vertex(Vec3(0.f, -0.97f, 0.f), wingColor, Vec2(0.f, 0.f));
}

//-----------------------------------------------------------------------------------------------
void BeetleEnemy::CollideTest() {
	if (!m_game->m_player->m_isDead) {
		if (DoDiscsOverlap(m_position, m_physicsRadius,
			m_game->m_player->m_position, m_game->m_player->m_physicsRadius)) {
			float playerVelocity = m_game->m_player->m_velocity.GetLength();
			m_game->m_player->LoseHealth(5.f);
			m_game->m_player->GetHit(m_position,m_physicsRadius);
			m_game->m_player->m_flashFraction = 1.f;

			m_game->AddCameraShake(m_bounceShakeAmp);
			SoundID shootSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/HitWall.wav");
			g_engine->m_audio->StartSound(shootSound, false, 1.0f, 0.f, m_randomGenerator.RollRandomFloatInRange(0.8f, 1.1f));
			if (playerVelocity > m_hitDieSpeed) {
				m_finalHitDir = (m_position - m_game->m_player->m_position).GetNormalized();
				Die();
			}
		}
	}
}

//----------------------------------------------------------------------------------------------
void BeetleEnemy::BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale) {
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

//----------------------------------------------------------------------------------------------
void BeetleEnemy::BurstShockWave(Vec2 position, float duration, float spreadDistance, Rgba8 waveColor) {

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

//----------------------------------------------------------------------------------------------
void BeetleEnemy::FindNearbyEnemy() {
	m_nearbyEnemyCount = 0;
	for (int i = 0; i < MAX_BEETLES; i++) {
		if (m_game->m_beetleEnemy[i] != nullptr && m_game->m_beetleEnemy[i] != this) {
			if ((m_game->m_beetleEnemy[i]->m_position - m_position).GetLengthSquared() < ENEMY_BEETLE_NEARBY_RADIUS * ENEMY_BEETLE_NEARBY_RADIUS) {
				m_nearbyEnemy[m_nearbyEnemyCount] = m_game->m_beetleEnemy[i];
				m_nearbyEnemyCount++;
			}
		}
	}

	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_game->m_waspEnemy[i] != nullptr) {
			if ((m_game->m_waspEnemy[i]->m_position - m_position).GetLengthSquared() < ENEMY_BEETLE_NEARBY_RADIUS * ENEMY_BEETLE_NEARBY_RADIUS) {
				m_nearbyEnemy[m_nearbyEnemyCount] = m_game->m_waspEnemy[i];
				m_nearbyEnemyCount++;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------
void BeetleEnemy::BoidSimulation() {
	Vec2 separation(0.f, 0.f);
	Vec2 alignment(0.f, 0.f);
	Vec2 cohesion(0.f, 0.f);

	if (m_nearbyEnemyCount > 0) {
		// Separation
		for (int i = 0; i < m_nearbyEnemyCount; i++) {
			Vec2 away = m_position - m_nearbyEnemy[i]->m_position;
			float sqrDistance = away.GetLengthSquared();
			if (sqrDistance < m_boidSeprationBoundery*m_boidSeprationBoundery) {
				separation += away / sqrDistance;
			}
		}
	    separation *= m_boidSeprationWeight;

		// Alignment
		for (int i = 0; i < m_nearbyEnemyCount; i++) {
			alignment += m_nearbyEnemy[i]->m_velocity;
		}
		alignment /= (float)m_nearbyEnemyCount;
		alignment *= m_boidsAlignmentWeight;

		// Cohesion
		for (int i = 0; i < m_nearbyEnemyCount; i++) {
			cohesion += m_nearbyEnemy[i]->m_position;
		}
		cohesion /= (float)m_nearbyEnemyCount;
		cohesion = (cohesion - m_position) * m_boidsCohesionWeight;
	}
	else {
		Vec2 toPlayer = m_game->m_player->m_position - m_position;
		toPlayer.Normalize();
		Vec2 playerForce = toPlayer * m_boidPlayerPullWeight;

		m_nextVelocity = m_velocity + playerForce * m_deltaSeconds;
		m_nextVelocity = m_nextVelocity.GetClamped(m_maxSpeed);
		return;
	}

	Vec2 toPlayer = m_game->m_player->m_position - m_position;
	toPlayer.Normalize();
	Vec2 playerForce = toPlayer * m_boidPlayerPullWeight;

	m_nextVelocity = m_velocity + (separation + alignment + cohesion + playerForce) * m_deltaSeconds;
	m_nextVelocity = m_nextVelocity.GetClamped(m_maxSpeed);
}
