#include "Game/Asteroid.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/Debris.hpp"
#include "Game/ShockWave.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Audio/AudioSystem.hpp"

//-----------------------------------------------------------------------------------------------
Asteroid::Asteroid(Game* game)
	: Entity(game, Vec2(0,0))
{
	m_velocity = Vec2(m_game->m_randomGenerator->RollRandomFloatZeroToOne()*2.f-1.f, 
					  m_game->m_randomGenerator->RollRandomFloatZeroToOne()*2.f-1.f).GetNormalized();
	m_randomScale = m_game->m_randomGenerator->RollRandomFloatInRange(m_randomScaleMin, m_randomScaleMax);
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS * m_randomScale;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS * m_randomScale;
	m_angularVelocity = m_game->m_randomGenerator->RollRandomFloatInRange(-200.f,200.f);
	m_health = (int)(3.f * m_randomScale);
	//SetPositionRandomOffWorld();
	SetPositionRandomOffScreen(m_game->m_player->m_position);
	m_localMesh = new Vertex[m_vertexNum];
	float randomLengths[16];
	for (int i = 0; i < 16; i++) {
		randomLengths[i] = m_game->m_randomGenerator->RollRandomFloatInRange(ASTEROID_PHYSICS_RADIUS * m_randomScale,
			ASTEROID_COSMETIC_RADIUS * m_randomScale);
	}

	for (int i = 0; i < 45; i += 3) {
		m_localMesh[i] = Vertex(Vec3(randomLengths[i / 3 + 1] * CosDegrees((i / 3 + 1) / 16.f * 360.f),
			randomLengths[i / 3 + 1] * SinDegrees((i / 3 + 1) / 16.f * 360.f),
			0.f),
			m_outerColor,
			Vec2(0.f, 0.f)
		);
		m_localMesh[i + 1] = Vertex(Vec3(randomLengths[i / 3] * CosDegrees((i / 3) / 16.f * 360.f),
			randomLengths[i / 3] * SinDegrees((i / 3) / 16.f * 360.f),
			0.f),
			m_outerColor,
			Vec2(0.f, 0.f)
		);
		m_localMesh[i + 2] = Vertex(Vec3(0.f, 0.f, 0.f),
			m_innerColor,
			Vec2(0.f, 0.f)
		);
	}


	m_localMesh[45] = Vertex(Vec3(randomLengths[0], 0.f, 0.f),
		m_outerColor,
		Vec2(0.f, 0.f)
	);
	m_localMesh[46] = Vertex(Vec3(randomLengths[15] * CosDegrees(15.f / 16.f * 360.f),
		randomLengths[15] * SinDegrees(15.f / 16.f * 360.f),
		0.f),
		m_outerColor,
		Vec2(0.f, 0.f)
	);
	m_localMesh[47] = Vertex(Vec3(0.f, 0.f, 0.f),
		m_innerColor,
		Vec2(0.f, 0.f)
	);
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
	CheckCollide();
	TeleportFromBoundary();
	//--------------------------------------------------------------------------------
	m_flashFraction = GetClamped(m_flashFraction - m_flashFractionDecay * deltaSeconds, 0.f, 1.f);

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
		m_worldMesh[i].m_color = Rgba8((unsigned char)Interpolate(m_worldMesh[i].m_color.r, 255.f, m_flashFraction),
			(unsigned char)Interpolate(m_worldMesh[i].m_color.g, 255.f, m_flashFraction),
			(unsigned char)Interpolate(m_worldMesh[i].m_color.b, 255.f, m_flashFraction),
			m_worldMesh[i].m_color.a);
	}
	TransformVertexArrayXY3D(48, m_worldMesh, 1.f, m_orientationDegrees, m_position);
	g_engine->m_renderer->DrawVertexArray(48, m_worldMesh);
}

//-----------------------------------------------------------------------------------------------
void Asteroid::Die()
{
	if (!m_isDead) {
		m_velocity = m_velocity.GetNormalized() * 10.f;
		BurstDebris(m_debrisNumMin, m_debrisNumMax, 
					-GetForwardVector(), 330.f, 
			Rgba8(100, 100, 100, 255), 1.1f * m_randomScale);
		BurstShockWave(m_position, 0.2f, 20.f * m_randomScale, Rgba8(255, 255, 255, 255));
		BurstShockWave(m_position, 0.4f, 10.f * m_randomScale, Rgba8(200, 200, 200, 255));
		m_isDead = true;
	}
}

//-----------------------------------------------------------------------------------------------
void Asteroid::CheckCollide() {
	if (!m_game->m_player->m_isDead && m_game->m_player->m_invincibleTimer == 0) {
		if (DoDiscsOverlap(m_position, m_physicsRadius,
			m_game->m_player->m_position, m_game->m_player->m_physicsRadius)) {
			float playerVelocity = m_game->m_player->m_velocity.GetLength();
			m_game->m_player->GetDamage(5.f);
			m_game->m_player->GetHit(m_position, m_physicsRadius);
			m_game->m_player->m_flashFraction = 1.f;

			m_game->AddCameraShake(m_bounceShakeAmp);
			SoundID shootSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/HitWall.wav");
			g_engine->m_audio->StartSound(shootSound, false, 1.0f, 0.f, m_game->m_randomGenerator->RollRandomFloatInRange(0.8f, 1.1f));
			if (playerVelocity > m_killSpeed) {
				Die();
			}
		}
	}
}

//----------------------------------------------------------------------------------------------
void Asteroid::BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale) {
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
void Asteroid::BurstShockWave(Vec2 position, float duration, float spreadDistance, Rgba8 waveColor) {

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