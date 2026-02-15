#include "Game/PlayerShip.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Game.hpp"
#include "Game/Debris.hpp"
#include "Game/ShockWave.hpp"
#include "Game/App.hpp"
#include "Game/BeetleEnemy.hpp"
#include "Game/WaspEnemy.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"

//-----------------------------------------------------------------------------------------------
PlayerShip::PlayerShip(Game* game)
	: Entity(game, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y))
{
	m_velocity = Vec2(0,0);
	m_physicsRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;
	m_angularVelocity = PLAYER_SHIP_TURN_SPEED;
	m_health = 3;

	m_localMesh = new Vertex[m_vertexNum];
	GetLocalMesh(m_vertexNum, m_localMesh);

}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Update()
{
	//--------------------------------------------------------------------------------
	if (m_isDead) {
		return;
	}

	//--------------------------------------------------------------------------------
	if (m_shieldBarVal > m_shieldBarMax * 0.5f && m_shieldBarMax > 0) {
		if (TeleportFromBoundary()) {
			BurstShockWave(m_position, 0.2f, 20.f, Rgba8(255, 0, 200, 255));
			BurstShockWave(m_position, 0.4f, 10.f, Rgba8(200, 0, 145, 255));
		}
	}
	else {
		CheckBounce();
	}
	//--------------------------------------------------------------------------------
	m_flashFraction = GetClamped(m_flashFraction - m_flashFractionDecay * (float)m_game->m_gameClock->GetDeltaSeconds(), 0.f, 1.f);
	m_fireTimer = GetClamped(m_fireTimer + (float)m_game->m_gameClock->GetDeltaSeconds(), 0.f, m_fireInterval);
	m_runTimer += (float)m_game->m_gameClock->GetDeltaSeconds();
	m_invincibleTimer = GetClamped(m_invincibleTimer - (float)m_game->m_gameClock->GetDeltaSeconds(), 0.f, m_invincibleDuration);
	if (m_shieldBarMax > 0) {
		m_shieldBarVal = GetClamped(m_shieldBarVal + m_shieldRecoverSpeed * (float)m_game->m_gameClock->GetDeltaSeconds(), 0.f, m_shieldBarMax);
	}
	//--------------------------------------------------------------------------------
	m_velocity += GetForwardVector() * m_acceleration * (float)m_game->m_gameClock->GetDeltaSeconds();
	m_curSpeed = m_velocity.GetLength();
	if (m_curSpeed > PLAYER_MAX_SPEED) {
		m_velocity = m_velocity / m_curSpeed * PLAYER_MAX_SPEED;
		m_curSpeed = PLAYER_MAX_SPEED;
	}
	m_position += m_velocity * (float)m_game->m_gameClock->GetDeltaSeconds();
	m_orientationDegrees += m_rotationSpeed * (float)m_game->m_gameClock->GetDeltaSeconds();
	g_engine->m_audio->SetSoundPlaybackVolume(g_app->m_accelerateSoundPlaybackID, 0.5f * m_acceleration / PLAYER_SHIP_ACCELERATION);

	//--------------------------------------------------------------------------------
	if (!IsOffWorld()) {
		m_lastFramePosition = m_position;
	}
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::Render() const
{
	Vertex m_worldMesh[m_vertexNum];
	for (int i = 0; i < m_vertexNum; i++)
	{
		m_worldMesh[i] = m_localMesh[i];
		if (m_shieldBarVal <= 0 && m_invincibleTimer == 0) {
			m_worldMesh[i].m_color = Rgba8((unsigned char)Interpolate(m_worldMesh[i].m_color.r, 255.f, m_flashFraction),
											(unsigned char)Interpolate(m_worldMesh[i].m_color.g, 255.f, m_flashFraction),
											(unsigned char)Interpolate(m_worldMesh[i].m_color.b, 255.f, m_flashFraction),
											m_worldMesh[i].m_color.a);
		}
		else if (m_invincibleTimer > 0) {
			m_worldMesh[i].m_color = Rgba8((unsigned char)Interpolate(m_worldMesh[i].m_color.r, 255.f, abs(SinDegrees(m_invincibleTimer*m_invincibleFlashSpeed))),
											(unsigned char)Interpolate(m_worldMesh[i].m_color.g, 255.f, abs(SinDegrees(m_invincibleTimer *m_invincibleFlashSpeed))),
											(unsigned char)Interpolate(m_worldMesh[i].m_color.b, 255.f, abs(SinDegrees(m_invincibleTimer *m_invincibleFlashSpeed))),
											m_worldMesh[i].m_color.a);
		}
	}
	float flameLength = -2.f - 2.f*(m_acceleration/ PLAYER_SHIP_ACCELERATION)*(m_game->m_randomGenerator->RollRandomFloatZeroToOne()*0.5f+1.f);
	m_worldMesh[15] = Vertex(Vec3(-2.f, 1.f, 0.f), Rgba8(255, 200, 0, 255), Vec2(0.f, 0.f));
	m_worldMesh[16] = Vertex(Vec3(-2.f, -1.f, 0.f), Rgba8(255, 200, 0, 255), Vec2(0.f, 0.f));
	m_worldMesh[17] = Vertex(Vec3(flameLength, 0.f, 0.f), Rgba8(255, 0, 0, 0), Vec2(0.f, 0.f));
	TransformVertexArrayXY3D(m_vertexNum, m_worldMesh, 1.f, m_orientationDegrees, m_position);
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray(m_vertexNum, m_worldMesh);

	if (m_shieldBarVal > 0) {
		if (m_shieldBarVal != m_shieldBarMax) {
			DrawDisc(m_position, 5.f, Rgba8(102, 153, 204, (unsigned char)(128.f * m_shieldBarVal / m_shieldBarMax)), Rgba8(102, 153, 204, 32));
		}
		else {
			DrawDisc(m_position, 5.f, Rgba8(102, 153, 204, 200), Rgba8(102, 153, 204, 32));
		}
	}
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
		BurstShockWave(m_position, 0.2f, 40.f, Rgba8(255, 255, 255, 255));
		BurstShockWave(m_position, 0.4f, 20.f, Rgba8(200, 200, 200, 255));

		m_velocity = Vec2(0,0);

		if (m_health <= 0 && !m_isDead) {
			SoundID quitSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/GameLose.mp3");
			g_engine->m_audio->StartSound(quitSound, false, 1.1f, 0.f, 2.f);
			m_game->DelayQuit(8.f);
		}

		m_game->AddCameraShake(m_dieScreenShakeAmp);

		SoundID dieSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/DieExplode.wav");
		g_engine->m_audio->StartSound(dieSound, false, 1.0f, 0.f, 0.8f);
		g_engine->m_audio->SetSoundPlaybackVolume(g_app->m_accelerateSoundPlaybackID, 0.f);

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
void PlayerShip::GetDamage(float damage) {
	if (m_isDead || m_invincibleTimer>0.f) {
		return;
	}

	if (damage < m_shieldBarVal) {
		if (m_shieldBarVal > m_shieldBarMax * 0.7f && m_shieldExplosionRange > 0) {
			ReleaseShockWave();
			BurstShockWave(m_position, 0.2f, m_shieldExplosionRange, Rgba8(102, 153, 204, 128));
			BurstShockWave(m_position, 0.4f, m_shieldExplosionRange*2.f/3.f, Rgba8(102, 153, 204, 200));
		}
		m_shieldBarVal = m_shieldBarVal - damage;
		return;
	}

	float overDamage = damage - m_shieldBarVal;
	m_shieldBarVal = 0;


	m_healthBarVal = GetClamped(m_healthBarVal - overDamage, 0.f, m_healthBarMax);
	if (m_healthBarVal <= 0) {
		Die();
	}

}

//-----------------------------------------------------------------------------------------------
void PlayerShip::GetHit(Vec2& hitTargetCenter, float hitTargetRadius) {
	PushDiscsOutOfEachOther2D(m_position, m_physicsRadius, hitTargetCenter, hitTargetRadius);
	m_velocity = m_velocity.GetReflected((m_position - hitTargetCenter).GetNormalized());
	if (m_velocity.GetLength() < 0.5f * PLAYER_MAX_SPEED) {
		m_velocity = m_velocity.GetNormalized() * PLAYER_MAX_SPEED * 0.5f;
	}
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::GainExp(float expAmount) {
	m_expBarVal += expAmount;
	if (m_expBarVal > m_nextLevelExpVal) {
		m_upgradeTimes += RoundDownToInt(m_expBarVal / m_nextLevelExpVal);
		m_expBarVal -= (float)m_upgradeTimes * m_nextLevelExpVal;
		m_curLevel += m_upgradeTimes;
		m_nextLevelExpVal = (float)m_curLevel * 10.f;

		SoundID levelUpSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/PlayerLevelUp.mp3");
		g_engine->m_audio->StartSound(levelUpSound, false, 1.5f, 0.f, 1.f);

		m_game->SetNextGameState(GAME_PLAYER_UPGRADE_MODE);
	}
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::GainUpgrade(PlayerUpgradeItem upgradeItem) {
	if (upgradeItem.hpUpgrade != NONE_HEALTH_UPGRADE) {
		switch (upgradeItem.hpUpgrade)
		{
		case HEALTH_MAX_UPGRADE:
			m_healthBarMax += upgradeItem.upgradeVal;
			break;
		default:
			break;
		}

		m_healthBarVal = m_healthBarMax;
	}

	if (upgradeItem.shieldUpgrade != NONE_SHIELD_UPGRADE) {
		switch (upgradeItem.shieldUpgrade)
		{
		case SHIELD_MAX_UPGRADE:
			m_shieldBarMax += upgradeItem.upgradeVal;
			break;
		case SHIELD_RECOVER_UPGREADE:
			m_shieldRecoverSpeed += upgradeItem.upgradeVal;
			break;
		case SHIELD_EXPLOSION_RANGE_UPGRADE:
			m_shieldExplosionRange += upgradeItem.upgradeVal;
			break;
		default:
			break;
		}

		m_shieldBarVal = m_shieldBarMax;
	}

	if (upgradeItem.bulletUpgrade != NONE_BULLET_UPGRADE) {
		switch (upgradeItem.bulletUpgrade)
		{
		case BULLET_INTERVAL_UPGRADE:
			m_fireInterval -= upgradeItem.upgradeVal;
			if (m_fireInterval < 0.f) m_fireInterval = 0;
			break;
		case BULLET_BRANCH_UPGRADE:
			m_fireBranch += upgradeItem.upgradeVal;
			break;
		case BULLET_EXPLOSION_RANGE_UPGRADE:
			m_bulletExplosionRange += upgradeItem.upgradeVal;
			break;
		case BULLET_TRACK_UPGRADE:
			m_bulletTrackDuration += upgradeItem.upgradeVal;
			break;
		default:
			break;
		}
	}

	m_upgradeTimes--;
	m_game->RollUpgrades();
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::ReleaseShockWave() {
	for (int i = 0; i < MAX_BEETLES; i++) {
		if (m_game->m_beetleEnemy[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_game->m_player->m_shieldExplosionRange,
				m_game->m_beetleEnemy[i]->m_position, m_game->m_beetleEnemy[i]->m_physicsRadius)) {
				m_game->m_beetleEnemy[i]->m_health--;
				m_game->m_beetleEnemy[i]->m_flashFraction = 1.f;
				m_game->m_beetleEnemy[i]->m_finalHitDir = (m_game->m_beetleEnemy[i]->m_position - m_position);
			}
		}
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_game->m_waspEnemy[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_game->m_player->m_shieldExplosionRange,
				m_game->m_waspEnemy[i]->m_position, m_game->m_waspEnemy[i]->m_physicsRadius)) {
				m_game->m_waspEnemy[i]->m_health--;
				m_game->m_waspEnemy[i]->m_flashFraction = 1.f;
				m_game->m_waspEnemy[i]->m_finalHitDir = (m_game->m_waspEnemy[i]->m_position - m_position);
			}
		}
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++) {
		if (m_game->m_asteroids[i] != nullptr) {
			if (DoDiscsOverlap(m_position, m_game->m_player->m_shieldExplosionRange,
				m_game->m_asteroids[i]->m_position, m_game->m_asteroids[i]->m_physicsRadius)) {
				m_game->m_asteroids[i]->m_health--;
				m_game->m_asteroids[i]->m_flashFraction = 1.f;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
void PlayerShip::CheckBounce() {
	if (m_position.x - m_physicsRadius < 0) {
		m_position = m_lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(1, 0) * m_velocity.x;
		m_game->AddCameraShake(m_bounceShakeAmp);
		SoundID shootSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/HitWall.wav");
		g_engine->m_audio->StartSound(shootSound, false, 1.0f, 0.f, m_game->m_randomGenerator->RollRandomFloatInRange(0.8f, 1.1f));
	}

	if (m_position.x + m_physicsRadius > WORLD_SIZE_X) {
		m_position = m_lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(-1, 0) * (-m_velocity.x);
		m_game->AddCameraShake(m_bounceShakeAmp);
		SoundID shootSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/HitWall.wav");
		g_engine->m_audio->StartSound(shootSound, false, 1.0f, 0.f, m_game->m_randomGenerator->RollRandomFloatInRange(0.8f, 1.1f));
	}

	if (m_position.y - m_physicsRadius < 0) {
		m_position = m_lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(0, 1) * m_velocity.y;
		m_game->AddCameraShake(m_bounceShakeAmp);
		SoundID shootSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/HitWall.wav");
		g_engine->m_audio->StartSound(shootSound, false, 1.0f, 0.f, m_game->m_randomGenerator->RollRandomFloatInRange(0.8f, 1.1f));
	}

	if (m_position.y + m_physicsRadius > WORLD_SIZE_Y) {
		m_position = m_lastFramePosition;
		m_velocity = m_velocity - 2.0f * Vec2(0, -1) * (-m_velocity.y);
		m_game->AddCameraShake(m_bounceShakeAmp);
		SoundID shootSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/HitWall.wav");
		g_engine->m_audio->StartSound(shootSound, false, 1.0f, 0.f, m_game->m_randomGenerator->RollRandomFloatInRange(0.8f, 1.1f));
	}
}

//----------------------------------------------------------------------------------------------
void PlayerShip::BurstDebris(int numMin, int numMax, Vec2 burstDirection, float burstAngle, Rgba8 color, float scale) {
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
			float randomAngle = m_game->m_randomGenerator->RollRandomFloatInRange(-burstAngle/2.f, burstAngle/2.f);
			float randomSpeed = m_game->m_randomGenerator->RollRandomFloatInRange(m_velocity.GetLength()*0.2f, m_velocity.GetLength());
			m_game->m_debris[freeDebrisIndex] = new Debris(m_game, m_position,
				burstDirection.GetRotatedByDegrees(randomAngle),
				randomSpeed, color, scale);
		}
	}

}

//----------------------------------------------------------------------------------------------
void PlayerShip::BurstShockWave(Vec2 position, float duration, float spreadDistance, Rgba8 waveColor) {

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