#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Bullet.hpp"
#include "Game/BeetleEnemy.hpp"
#include "Game/WaspEnemy.hpp"
#include "Game/App.hpp"
#include "Game/Debris.hpp"
#include "Game/ShockWave.hpp"

#include "Engine/Core/Engine.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"

//-----------------------------------------------------------------------------------------------
Game::Game()
{
	m_player = new PlayerShip(this);
	InitialObjectPools();
	LoadNextWave(m_curLevelIndex);
	m_worldCamera = new Camera(Vec2(WORLD_CENTER_X - VIEW_CENTER_X, WORLD_CENTER_Y - VIEW_CENTER_Y), Vec2(WORLD_CENTER_X+VIEW_CENTER_X, WORLD_CENTER_Y+VIEW_CENTER_Y));
	m_viewCamera = new Camera(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_randomGenerator = new RandomNumberGenerator();
	m_gameClock = new Clock();

	m_attractModeBoids[0].boidsType = 0;
	m_attractModeBoids[0].isActive = true;
	m_attractModeBoids[0].position = Vec2(SCREEN_CENTER_X,SCREEN_CENTER_Y);
	m_attractModeBoids[0].velocity = Vec2(m_randomGenerator->RollRandomFloatInRange(-1.f, 1.f),
		m_randomGenerator->RollRandomFloatInRange(-1.f, 1.f));
	m_attractModeBoids[0].velocity.Normalize();
	m_attractModeBoids[0].velocity *= m_boidBeetleMaxSpeed * m_randomGenerator->RollRandomFloatInRange(25.f, 75.f);
	m_attractModeBoids[0].bounderyRadius = 80.f;
	m_attractModeBoids[0].nearbyRadius = 200.f;

	for (int i = 1; i < MAX_ATTRACTMODE_BOIDS; i++) {
		m_attractModeBoids[i].boidsType = m_randomGenerator->RollRandomIntInRange(1,2);
		m_attractModeBoids[i].isActive = true;
		m_attractModeBoids[i].position = Vec2(m_randomGenerator->RollRandomFloatInRange(0.f, SCREEN_SIZE_X),
			m_randomGenerator->RollRandomFloatInRange(0.f, SCREEN_SIZE_Y));
		m_attractModeBoids[i].velocity = Vec2(m_randomGenerator->RollRandomFloatInRange(-1.f, 1.f),
			m_randomGenerator->RollRandomFloatInRange(-1.f, 1.f));
		m_attractModeBoids[i].velocity.Normalize();
		m_attractModeBoids[i].velocity *= m_boidBeetleMaxSpeed * m_randomGenerator->RollRandomFloatInRange(25.f, 75.f);
	}

	m_attractModeStarsMesh = new Vertex[MAX_ATTRACTMODE_STAR * 24];
	m_attractModeNearStarsMesh = new Vertex[MAX_NEAR_STAR*24];
	m_attractModeFarStarsMesh = new Vertex[MAX_FAR_STAR * 24];
	GenerateStarsMesh(MAX_ATTRACTMODE_STAR, m_attractModeStarsMesh, 
		Vec2(0,0), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y), 0.5f, 1.f, Rgba8(200, 200, 200, 255), Rgba8(255, 255, 255, 255));
	GenerateStarsMesh(MAX_NEAR_STAR, m_attractModeNearStarsMesh, 
		Vec2(-m_attractModeStarExpandDistance, -m_attractModeStarExpandDistance), Vec2(WORLD_SIZE_X + m_attractModeStarExpandDistance, WORLD_SIZE_Y + m_attractModeStarExpandDistance), 
		0.3f, 0.6f, Rgba8(200, 200, 200, 255), Rgba8(255, 255, 255, 255));
	GenerateStarsMesh(MAX_FAR_STAR, m_attractModeFarStarsMesh, 
		Vec2(-m_attractModeStarExpandDistance, -m_attractModeStarExpandDistance), Vec2(WORLD_SIZE_X + m_attractModeStarExpandDistance, WORLD_SIZE_Y + m_attractModeStarExpandDistance), 
		0.2f, 0.3f, Rgba8(200, 200, 200, 255), Rgba8(255, 255, 255, 255));
}

//-----------------------------------------------------------------------------------------------
Game::~Game()
{
	delete m_gameClock;
	m_gameClock = nullptr;

	delete m_randomGenerator;
	m_randomGenerator = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;

	delete m_viewCamera;
	m_viewCamera = nullptr;

	delete m_player;
	m_player = nullptr;
	DeleteObjectPools();
}

//-----------------------------------------------------------------------------------------------
void Game::Update()
{
	//-----------------------------------------------------------------------------------------------
	if (m_nextGameState != m_curGameState) {
		m_curGameState = m_nextGameState;
	}

	//-----------------------------------------------------------------------------------------------
	if (m_curGameState == GAME_PLAYER_UPGRADE_MODE) {
		m_playModeSoundSpeed = GetClamped(
			m_playModeSoundSpeed - m_playModeSoundDecaySpeed * (float)m_gameClock->GetDeltaSeconds(), 
			0.5f* (float)m_gameClock->GetTimeScale(), 
			(float)m_gameClock->GetTimeScale()
		);
		g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_gameSoundPlaybackID, m_playModeSoundSpeed);
		if (m_playerUpgradeStartFlag) {
			RollUpgrades();
			m_playerUpgradeStartFlag = false;
		}

		if (m_player->m_upgradeTimes <= 0) {
			SetNextGameState(GAME_PLAYING_MODE);
		}
	}

	//-----------------------------------------------------------------------------------------------
	if (m_curGameState == GAME_PLAYING_MODE) {
		m_attractModeStartFlag = true;
		m_playerUpgradeStartFlag = true;
		m_playModeSoundSpeed = GetClamped(m_playModeSoundSpeed + m_playModeSoundDecaySpeed * (float)m_gameClock->GetDeltaSeconds(), 0.f, (float)m_gameClock->GetTimeScale());
		g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_gameSoundPlaybackID, m_playModeSoundSpeed);

		if (m_gameModeStartFlag) {
			g_engine->m_audio->SetSoundPlaybackRestart(g_app->m_gameSoundPlaybackID);
			g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_gameSoundPlaybackID, (float)m_gameClock->GetTimeScale());
			g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_attractSoundPlaybackID, 0.f);
			g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_accelerateSoundPlaybackID, (float)m_gameClock->GetTimeScale());
			m_gameModeStartFlag = false;
		}

		//-----------------------------------------------------------------------------------------------
		if (m_delayQuitFlag) {
			m_delayQuitWaitedTime += (float)m_gameClock->GetDeltaSeconds();
			if (m_delayQuitWaitedTime > m_delayQuitDuration) {
				m_nextGameState = GAME_ATTRACT_MODE;
			}

			g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_gameSoundPlaybackID, (1.f - m_delayQuitWaitedTime / m_delayQuitDuration)* (float)m_gameClock->GetTimeScale());
		}
		//-----------------------------------------------------------------------------------------------
		if (m_player != nullptr && !m_player->m_isDead) {
			m_player->Update();
		}
		UpdateEntities();
		HandleEntitiesCollide();
		//-----------------------------------------------------------------------------------------------
		HandleFiringInput();
		//-----------------------------------------------------------------------------------------------
		HandleRespawnAsteroidInput();
		//-----------------------------------------------------------------------------------------------
		HandleRespawnPlayerInput();
		//-----------------------------------------------------------------------------------------------
		CheckWaveProgress();
		//-----------------------------------------------------------------------------------------------
		UpdateCameras();

		//-----------------------------------------------------------------------------------------------
		m_waveAnimationTimeCount += (float)m_gameClock->GetDeltaSeconds();
	}
	
	//-----------------------------------------------------------------------------------------------
	if(m_curGameState == GAME_ATTRACT_MODE) {
		m_gameModeStartFlag = true;
		if (m_attractModeStartFlag) {
			g_engine->m_audio->SetSoundPlaybackRestart(g_app->m_attractSoundPlaybackID);
			g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_attractSoundPlaybackID, (float)m_gameClock->GetTimeScale());
			g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_gameSoundPlaybackID, 0.f);
			g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_accelerateSoundPlaybackID, 0.f);
			m_attractModeStartFlag = false;
		}
		else {
			g_engine->m_audio->SetSoundPlaybackSpeed(g_app->m_attractSoundPlaybackID, (float)m_gameClock->GetTimeScale());
		}

		UpdateAllAttractModeBoids();

		m_titleAnimationTimeCount += (float)m_gameClock->GetDeltaSeconds();
		if (m_titleAnimationTimeCount > m_titleAnimationTotalTime) {
			m_titleAnimationTimeCount = 0;
		}

		m_boidsAnimationTimeCount += (float)m_gameClock->GetDeltaSeconds();
		if (m_boidsAnimationTimeCount > m_boidsAnimationTotalTime) {
			m_boidsAnimationTimeCount = 0;
		}

		m_boidsPlayerAnimationTimeCount += (float)m_gameClock->GetDeltaSeconds();
		if (m_boidsPlayerAnimationTimeCount > m_boidsPlayerAnimationTotalTime) {
			m_boidsPlayerAnimationTimeCount = 0;
		}
	}
}

//---------------------------------------------------------------------------------------------------
void Game::Render() const
{
	//Clear Screen
	g_engine->m_renderer->ClearScreen(Rgba8(0, 0, 0));
	//----------------------------------------------------------------------------------------------
	if (m_curGameState == GAME_PLAYING_MODE || m_curGameState == GAME_PLAYER_UPGRADE_MODE) {
		//-------------------------------------------------------------------------------------------
		g_engine->m_renderer->BeginCamera(*m_worldCamera);
		RenderStars();

		RenderPoolEntitys();

		if (!m_player->m_isDead) {
			m_player->Render();
		}

		RenderWorldBoundary();

		if (g_app->m_isDebugDraw) {
			RenderDebugThings();
		}

		g_engine->m_renderer->EndCamera(*m_worldCamera);

		//------------------------------------------------------------------------------------------

		g_engine->m_renderer->BeginCamera(*m_viewCamera);

		RenderUI();


		if (m_delayQuitFlag) {
			DrawAABB(Vec2(0, 0), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y),
				Rgba8(0, 0, 0, (unsigned char)(255.f * (m_delayQuitWaitedTime / m_delayQuitDuration))),
				Rgba8(0, 0, 0, (unsigned char)(255.f * (m_delayQuitWaitedTime / m_delayQuitDuration))),
				Rgba8(0, 0, 0, (unsigned char)(255.f * (m_delayQuitWaitedTime / m_delayQuitDuration))));
		}

		g_engine->m_renderer->EndCamera(*m_viewCamera);
	}

	//----------------------------------------------------------------------------------------------
	if (m_curGameState == GAME_ATTRACT_MODE) {
		g_engine->m_renderer->BeginCamera(*m_viewCamera);
		RenderStars();
		if (g_app->m_isDebugDraw) {
			RenderDebugThings();
		}
		RenderAttractMode();

		g_engine->m_renderer->EndCamera(*m_viewCamera);
	}
}

//---------------------------------------------------------------------------------------------------
void Game::InitialObjectPools() {
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		m_asteroids[i] = nullptr;
	}
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		m_bullets[i] = nullptr;
	}
	for (int i = 0; i < MAX_BEETLES; i++) {
		m_beetleEnemy[i] = nullptr;
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		m_waspEnemy[i] = nullptr;
	}
	for (int i = 0; i < MAX_DEBRIS; i++) {
		m_debris[i] = nullptr;
	}
	for (int i = 0; i < MAX_SHOCKWAVE; i++) {
		m_shockWaves[i] = nullptr;
	}
}

//---------------------------------------------------------------------------------------------------
void Game::DeleteObjectPools() {
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr) {
			delete m_asteroids[i];
			m_asteroids[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr) {
			delete m_bullets[i];
			m_bullets[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetleEnemy[i] != nullptr) {
			delete m_beetleEnemy[i];
			m_beetleEnemy[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_waspEnemy[i] != nullptr) {
			delete m_waspEnemy[i];
			m_waspEnemy[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_DEBRIS; i++) {
		if (m_debris[i] != nullptr) {
			delete m_debris[i];
			m_debris[i] = nullptr;
		}
	}
	for (int i = 0; i < MAX_SHOCKWAVE; i++) {
		if (m_shockWaves[i] != nullptr) {
			delete m_shockWaves[i];
			m_shockWaves[i] = nullptr;
		}
	}
}

//---------------------------------------------------------------------------------------------------
void Game::UpdateEntities() {
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr) {
			m_asteroids[i]->Update();
			if (m_asteroids[i]->m_isDead) {
				delete m_asteroids[i];
				m_asteroids[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr) {
			m_bullets[i]->Update();
			if (m_bullets[i]->m_isDead) {
				delete m_bullets[i];
				m_bullets[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetleEnemy[i] != nullptr) {
			m_beetleEnemy[i]->Update();
			if (m_beetleEnemy[i]->m_isDead) {
				delete m_beetleEnemy[i];
				m_beetleEnemy[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_waspEnemy[i] != nullptr) {
			m_waspEnemy[i]->Update();
			if (m_waspEnemy[i]->m_isDead) {
				m_waspEnemy[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_DEBRIS; i++) {
		if (m_debris[i] != nullptr) {
			m_debris[i]->Update();
			if (m_debris[i]->m_isDead) {
				m_debris[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_SHOCKWAVE; i++) {
		if (m_shockWaves[i] != nullptr) {
			m_shockWaves[i]->Update();
			if (m_shockWaves[i]->m_isDead) {
				m_shockWaves[i] = nullptr;
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------
void Game::HandleEntitiesCollide() {
	for (int i = 0; i < MAX_BEETLES; i++) {
		for(int j = i+1; j < MAX_BEETLES; j++) {
			if (m_beetleEnemy[i] != nullptr && m_beetleEnemy[j] != nullptr) {
				PushDiscsOutOfEachOther2D(m_beetleEnemy[i]->m_position, ENEMY_BEETLE_PHYSICS_RADIUS,
					m_beetleEnemy[j]->m_position, ENEMY_BEETLE_PHYSICS_RADIUS);
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------
void Game::HandleFiringInput() {
	if (g_app->m_isFiring && m_player != nullptr) {
		if (!m_player->m_isDead && m_player->m_fireTimer == m_player->m_fireInterval) {
			for (int i = 0; i < m_player->m_fireBranch; i++) {
				int freeBulletIndex = -1;
				for (int j = 0; j < MAX_BULLETS; j++) {
					if (m_bullets[j] == nullptr) {
						freeBulletIndex = j;
						break;
					}
				}

				if (freeBulletIndex > -1) {
					m_bullets[freeBulletIndex] = new Bullet(this, m_player->m_position + m_player->GetForwardVector(),
						 Vec2::MakeFromPolarDegrees(m_player->m_orientationDegrees + 
													m_player->m_fireAngle/2.f - 
													(i+1)*(m_player->m_fireAngle/(m_player->m_fireBranch+1)
													+ m_randomGenerator->RollRandomFloatInRange(-m_player->m_randomFireAngle, m_player->m_randomFireAngle))));

					SoundID shootSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/ShootBullet.mp3");
					g_engine->m_audio->StartSound(shootSound, false, 1.0f, 0.f, m_randomGenerator->RollRandomFloatInRange(0.8f, 1.1f));
				}
				else {
					ERROR_RECOVERABLE("Run out bullets!")
				}
			}
			m_player->m_fireTimer = 0.f;
		}

		g_app->m_isFiring = false;
	}
	else if (g_app->m_isFiring) {
		g_app->m_isFiring = false;
	}
}

//---------------------------------------------------------------------------------------------------
void Game::HandleRespawnAsteroidInput() {
	if (g_app->m_isAsteroidRespawn) {
		int freeAsteroidIndex = -1;
		for (int i = 0; i < MAX_ASTEROIDS; i++) {
			if (m_asteroids[i] == nullptr) {
				freeAsteroidIndex = i;
				break;
			}
		}

		if (freeAsteroidIndex > -1) {
			m_asteroids[freeAsteroidIndex] = new Asteroid(this);
		}
		else {
			ERROR_RECOVERABLE("Run out asteroids!")
		}

		g_app->m_isAsteroidRespawn = false;
	}
}

//---------------------------------------------------------------------------------------------------
void Game::HandleRespawnPlayerInput() {
	if (g_app->m_isPlayerRespawn) {
		if (m_player != nullptr) {
			if (m_player->m_isDead && m_player->m_health>0) {
				m_player->m_position = Vec2(WORLD_CENTER_X, WORLD_CENTER_Y);
				m_player->m_velocity = Vec2(0, 0);
				m_player->m_acceleration = 0;
				m_player->m_orientationDegrees = 0.f;
				m_player->m_isDead = false;
				m_player->m_health--;
				m_player->m_healthBarVal = m_player->m_healthBarMax;
				m_player->m_shieldBarVal = m_player->m_shieldBarMax;
				m_player->m_invincibleTimer = m_player->m_invincibleDuration;
				SoundID respawnSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/Respawn.wav");
				g_engine->m_audio->StartSound(respawnSound, false, 1.0f, 0.f, 1.f);
			}
		}

		g_app->m_isPlayerRespawn = false;
	}
	else if (g_app->m_isPlayerRespawn) {
		g_app->m_isPlayerRespawn = false;
	}
}

//---------------------------------------------------------------------------------------------------
void Game::UpdateCameras() {
	DecayCameraShake();

	m_worldCamera->LookAt2D(m_player->m_position, Vec2(-VIEW_CENTER_X, -VIEW_CENTER_Y), Vec2(VIEW_CENTER_X, VIEW_CENTER_Y));
	m_worldCamera->ConstrainOrthoView(Vec2(0, 0), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
	m_worldCamera->Translate2D(Vec2(m_randomGenerator->RollRandomFloatZeroToOne() * m_curCameraShakeAmp,
		m_randomGenerator->RollRandomFloatZeroToOne() * m_curCameraShakeAmp));
}

//---------------------------------------------------------------------------------------------------
void Game::RenderPoolEntitys() const{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr) {
			m_asteroids[i]->Render();
		}
	}
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr) {
			m_bullets[i]->Render();
		}
	}
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetleEnemy[i] != nullptr) {
			m_beetleEnemy[i]->Render();
		}
	}
	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_waspEnemy[i] != nullptr) {
			m_waspEnemy[i]->Render();
		}
	}
	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] != nullptr) {
			m_debris[i]->Render();
		}
	}
	for (int i = 0; i < MAX_SHOCKWAVE; i++)
	{
		if (m_shockWaves[i] != nullptr) {
			m_shockWaves[i]->Render();
		}
	}
}

//---------------------------------------------------------------------------------------------------
void Game::RenderDebugThings() const {
	if (m_curGameState == GAME_PLAYING_MODE) {
		float drawThickness = .2f;
		if (m_player != nullptr)
		{
			DrawRing(m_player->m_position, m_player->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
			DrawRing(m_player->m_position, m_player->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
			DrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector() * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
			DrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector().GetRotatedBy90Degrees() * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
			DrawLine(m_player->m_position, m_player->m_position + m_player->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
		}

		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (m_asteroids[i] != nullptr) {
				DrawLine(m_asteroids[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DrawRing(m_asteroids[i]->m_position, m_asteroids[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DrawRing(m_asteroids[i]->m_position, m_asteroids[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + Vec2(CosDegrees(m_asteroids[i]->m_orientationDegrees), SinDegrees(m_asteroids[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + Vec2(-SinDegrees(m_asteroids[i]->m_orientationDegrees), CosDegrees(m_asteroids[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + m_asteroids[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}

		for (int i = 0; i < MAX_BULLETS; i++) {
			if (m_bullets[i] != nullptr) {
				DrawLine(m_bullets[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DrawRing(m_bullets[i]->m_position, m_bullets[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DrawRing(m_bullets[i]->m_position, m_bullets[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + Vec2(CosDegrees(m_bullets[i]->m_orientationDegrees), SinDegrees(m_bullets[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + Vec2(-SinDegrees(m_bullets[i]->m_orientationDegrees), CosDegrees(m_bullets[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + m_bullets[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}

		for (int i = 0; i < MAX_BEETLES; i++) {
			if (m_beetleEnemy[i] != nullptr) {
				DrawLine(m_beetleEnemy[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DrawRing(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DrawRing(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DrawRing(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_nearbyRadius, Rgba8(255, 255, 0, 255), drawThickness);
				DrawLine(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_position + Vec2(CosDegrees(m_beetleEnemy[i]->m_orientationDegrees), SinDegrees(m_beetleEnemy[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DrawLine(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_position + Vec2(-SinDegrees(m_beetleEnemy[i]->m_orientationDegrees), CosDegrees(m_beetleEnemy[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DrawLine(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_position + m_beetleEnemy[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}

		for (int i = 0; i < MAX_WASPS; i++) {
			if (m_waspEnemy[i] != nullptr) {
				DrawLine(m_waspEnemy[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DrawRing(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DrawRing(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DrawRing(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_nearbyRadius, Rgba8(255, 255, 0, 255), drawThickness);
				DrawLine(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_position + Vec2(CosDegrees(m_waspEnemy[i]->m_orientationDegrees), SinDegrees(m_waspEnemy[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DrawLine(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_position + Vec2(-SinDegrees(m_waspEnemy[i]->m_orientationDegrees), CosDegrees(m_waspEnemy[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DrawLine(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_position + m_waspEnemy[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}

		for (int i = 0; i < MAX_DEBRIS; i++) {
			if (m_debris[i] != nullptr) {
				DrawLine(m_debris[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DrawRing(m_debris[i]->m_position, m_debris[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DrawRing(m_debris[i]->m_position, m_debris[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DrawLine(m_debris[i]->m_position, m_debris[i]->m_position + Vec2(CosDegrees(m_debris[i]->m_orientationDegrees), SinDegrees(m_debris[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DrawLine(m_debris[i]->m_position, m_debris[i]->m_position + Vec2(-SinDegrees(m_debris[i]->m_orientationDegrees), CosDegrees(m_debris[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DrawLine(m_debris[i]->m_position, m_debris[i]->m_position + m_debris[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}
	}
	else if (m_curGameState == GAME_ATTRACT_MODE) {
		for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i++) {
			if (m_attractModeBoids[i].isActive) {
				DrawRing(m_attractModeBoids[i].position, m_attractModeBoids[i].bounderyRadius, Rgba8(255, 0, 0, 255), 1.f);
				DrawRing(m_attractModeBoids[i].position, m_attractModeBoids[i].nearbyRadius, Rgba8(0, 255, 0, 255), 1.f);
				DrawLine(m_attractModeBoids[i].position, m_attractModeBoids[i].position+m_attractModeBoids[i].velocity,
					Rgba8(0,0,(unsigned char)(255.f * (float)m_attractModeBoids[i].neighborCount/10.f), 255), 1.f);
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------
void Game::LoadNextWave(int levelIndex) {
	SoundID waveSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/WaveStart.mp3");
	g_engine->m_audio->StartSound(waveSound, false, 2.0f, 0.f, 1.f);

	for (int i = 0; i < m_levels[levelIndex].asteroidNum; i++) {
		int freeAsteroidIndex = -1;
		for (int j = 0; j < MAX_ASTEROIDS; j++) {
			if (m_asteroids[j] == nullptr) {
				freeAsteroidIndex = j;
			}
		}
		if (freeAsteroidIndex > -1) {
			m_asteroids[freeAsteroidIndex] = new Asteroid(this);
		}
	}
	for (int i = 0; i < m_levels[levelIndex].beetleEnemy; i++) {
		m_beetleEnemy[i] = new BeetleEnemy(this);
	}
	for (int i = 0; i < m_levels[levelIndex].waspEnemy; i++) {
		m_waspEnemy[i] = new WaspEnemy(this);
	}
}

//--------------------------------------------------------------------------------------------------
void Game::CheckWaveProgress() {
	m_isRemainEnemy = true;
	for (int i = 0; i < MAX_BEETLES; i++) {
		if (m_beetleEnemy[i] != nullptr || !m_isRemainEnemy) {
			m_isRemainEnemy = false;
			break;
		}
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_waspEnemy[i] != nullptr || !m_isRemainEnemy) {
			m_isRemainEnemy = false;
			break;
		}
	}

	if (m_isRemainEnemy) {
		m_curLevelIndex++;
		m_waveAnimationTimeCount = 0;
		if (m_curLevelIndex < sizeof(m_levels) / sizeof(GameLevel)) {
			LoadNextWave(m_curLevelIndex);
		}
		else {
			DelayQuit(9.f);
		}
	}
}

//--------------------------------------------------------------------------------------------------
void Game::RenderUI() const {
	RenderUIHealthBar();
	if (m_player->m_shieldBarMax > 0) {
		RenderUIShieldBar();
	}
	RenderUIHealth();
	RenderUIMiniMap();
	RenderUIWaves();
	RenderUIExpBar();

	if (m_curGameState == GAME_PLAYER_UPGRADE_MODE) {
		RenderUIUpgrade();
	}
}

//--------------------------------------------------------------------------------------------------
void Game::RenderUIHealthBar() const {
	DrawAABB(Vec2(12.5f, SCREEN_SIZE_Y - 7.5f - 25.f), Vec2(7.5f + 600.f, SCREEN_SIZE_Y - 12.5f),
				  Rgba8(180,180,180,255), Rgba8(224,224,224,255), Rgba8(224,224,224,255));
	
	float healthGridSize = 590.f / m_player->m_healthBarMax;
	for (int i = 0; i < m_player->m_healthBarMax; i++) {
		DrawAABB(Vec2(15.f + i * healthGridSize, SCREEN_SIZE_Y - 5.f - 25.f), Vec2(15.f + (i + 1) * healthGridSize, SCREEN_SIZE_Y - 15.f),
			Rgba8(64, 64, 64, 255), Rgba8(128, 128, 128, 255), Rgba8(64, 64, 64, 255));
	}
	for (int i = 0; i < m_player->m_healthBarVal; i++) {
		DrawAABB(Vec2(15.f + i*healthGridSize , SCREEN_SIZE_Y - 5.f - 25.f), Vec2(15.f + (i+1) * healthGridSize, SCREEN_SIZE_Y - 15.f),
			Rgba8(255, 32, 32, 255), Rgba8(255, 128, 128, 255), Rgba8(255, 128, 128, 255));
	}
}

//--------------------------------------------------------------------------------------------------
void Game::RenderUIShieldBar() const {
	DrawAABB(Vec2(12.5f, SCREEN_SIZE_Y - 7.5f - 50.f), Vec2(7.5f + 400.f, SCREEN_SIZE_Y - 37.5f),
		Rgba8(180, 180, 180, 255), Rgba8(224, 224, 224, 255), Rgba8(224, 224, 224, 255));

	float shieldGridSize = 390.f / m_player->m_shieldBarMax;
	for (int i = 0; i < m_player->m_shieldBarMax; i++) {
		DrawAABB(Vec2(15.f + i * shieldGridSize, SCREEN_SIZE_Y - 5.f - 50.f), Vec2(15.f + (i + 1) * shieldGridSize, SCREEN_SIZE_Y - 40.f),
			Rgba8(64, 64, 64, 255), Rgba8(128, 128, 128, 255), Rgba8(64, 64, 64, 255));
	}
	for (int i = 0; i < m_player->m_shieldBarVal; i++) {
		DrawAABB(Vec2(15.f + i * shieldGridSize, SCREEN_SIZE_Y - 5.f - 50.f), Vec2(15.f + (i + 1) * shieldGridSize, SCREEN_SIZE_Y - 40.f),
			Rgba8(102, 153, 204, 255), Rgba8(102, 153, 204, 128), Rgba8(102, 153, 204, 128));
	}
}

//--------------------------------------------------------------------------------------------------
void Game::RenderUIHealth() const{
	Vertex screenPlayerHealthMesh[PlayerShip::m_vertexNum];
	for (int i = 0; i < m_player->m_health; i++) {
		PlayerShip::GetLocalMesh(PlayerShip::m_vertexNum, screenPlayerHealthMesh);
		TransformVertexArrayXY3D(15, screenPlayerHealthMesh, 4.f, 90.f,
			Vec2(15.f + 4.f * PLAYER_SHIP_COSMETIC_RADIUS * (2 * i + 1),
				(m_player->m_shieldBarMax > 0 ? -60.f : -35.f)
				+ SCREEN_SIZE_Y - 4.f * PLAYER_SHIP_COSMETIC_RADIUS));
		g_engine->m_renderer->BindTexture(nullptr);
		g_engine->m_renderer->DrawVertexArray(15, screenPlayerHealthMesh);
	}
}

//--------------------------------------------------------------------------------------------------
void Game::RenderUIMiniMap() const {
	DrawAABB(m_miniMapCenter + Vec2(- WORLD_CENTER_X, -WORLD_CENTER_Y)*m_miniMapReduceRatio, 
				  m_miniMapCenter + Vec2(WORLD_CENTER_X, WORLD_CENTER_Y)*m_miniMapReduceRatio,
				  m_miniMapGridLineColor, m_miniMapGridLineColor, Rgba8(0,0,0,0));

	for (int i = 0; i <= m_miniMapGridNum; i++) {
		Vec2 xBias = Vec2(-WORLD_CENTER_X + (float)i/(float)m_miniMapGridNum * WORLD_SIZE_X, WORLD_CENTER_Y) * m_miniMapReduceRatio;
		Vec2 yBias = Vec2(-WORLD_CENTER_X, WORLD_CENTER_Y - (float)i / (float)m_miniMapGridNum * WORLD_SIZE_Y) * m_miniMapReduceRatio;
		DrawLine(m_miniMapCenter + xBias, m_miniMapCenter + xBias + Vec2(0, -WORLD_SIZE_Y * m_miniMapReduceRatio), m_miniMapGridLineColor, m_miniMapGridLineThickness);
		DrawLine(m_miniMapCenter + yBias, m_miniMapCenter + yBias + Vec2(WORLD_SIZE_X*m_miniMapReduceRatio, 0), m_miniMapGridLineColor, m_miniMapGridLineThickness);
	}

	DrawLine(m_miniMapOrigin + m_worldCamera->GetOrthoBottomLeft() * m_miniMapReduceRatio, 
				  m_miniMapOrigin + m_worldCamera->GetOrthoTopLeft() * m_miniMapReduceRatio, m_miniMapViewFieldColor, 2 * m_miniMapGridLineThickness);
	DrawLine(m_miniMapOrigin + m_worldCamera->GetOrthoTopLeft() * m_miniMapReduceRatio,
				  m_miniMapOrigin + m_worldCamera->GetOrthoTopRight() * m_miniMapReduceRatio, m_miniMapViewFieldColor, 2 * m_miniMapGridLineThickness);
	DrawLine(m_miniMapOrigin + m_worldCamera->GetOrthoTopRight() * m_miniMapReduceRatio,
				  m_miniMapOrigin + m_worldCamera->GetOrthoBottomRight() * m_miniMapReduceRatio, m_miniMapViewFieldColor, 2 * m_miniMapGridLineThickness);
	DrawLine(m_miniMapOrigin + m_worldCamera->GetOrthoBottomRight() * m_miniMapReduceRatio,
				  m_miniMapOrigin + m_worldCamera->GetOrthoBottomLeft() * m_miniMapReduceRatio, m_miniMapViewFieldColor, 2 * m_miniMapGridLineThickness);

	Vec2 playerMiniMapPos = m_miniMapCenter + m_miniMapReduceRatio * (m_player->m_position - Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));
	DrawLine(playerMiniMapPos, playerMiniMapPos + m_player->m_velocity.GetNormalized() * m_miniMapTokenSize * 3.f,
		m_miniMapPlayerVelocityColor, m_miniMapGridLineThickness);
	DrawDisc(playerMiniMapPos, m_miniMapTokenSize, m_miniMapPlayerOuterColor, m_miniMapPlayerInnerColor);

	for (int i = 0; i < MAX_ASTEROIDS; i++) {
		if (m_asteroids[i] != nullptr) {
			Vec2 miniMapPos = m_miniMapCenter + m_miniMapReduceRatio * (m_asteroids[i]->m_position - Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));
			DrawDisc(miniMapPos, m_miniMapTokenSize, m_miniMapAsteroidOuterColor, m_miniMapAsteroidInnerColor);
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++) {
		if (m_beetleEnemy[i] != nullptr) {
			Vec2 miniMapPos = m_miniMapCenter + m_miniMapReduceRatio * (m_beetleEnemy[i]->m_position - Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));
			DrawDisc(miniMapPos, m_miniMapTokenSize, m_miniMapBeetleOuterColor, m_miniMapBeetleInnerColor);
		}
	}

	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_waspEnemy[i] != nullptr) {
			Vec2 miniMapPos = m_miniMapCenter + m_miniMapReduceRatio * (m_waspEnemy[i]->m_position - Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));
			DrawDisc(miniMapPos, m_miniMapTokenSize, m_miniMapWaspOuterColor, m_miniMapWaspInnerColor);
		}
	}

	DrawLine(m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(-m_miniMapFrameThickness/2.f, -m_miniMapFrameThickness / 2.f),
		m_miniMapCenter + Vec2(-WORLD_CENTER_X, WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(-m_miniMapFrameThickness / 2.f, m_miniMapFrameThickness / 2.f), 
		m_miniMapFrameColor, m_miniMapFrameThickness);
	DrawLine(m_miniMapCenter + Vec2(-WORLD_CENTER_X, WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(-m_miniMapFrameThickness / 2.f, m_miniMapFrameThickness / 2.f),
		m_miniMapCenter + Vec2(WORLD_CENTER_X, WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(m_miniMapFrameThickness / 2.f, m_miniMapFrameThickness / 2.f), 
		m_miniMapFrameColor, m_miniMapFrameThickness);
	DrawLine(m_miniMapCenter + Vec2(WORLD_CENTER_X, WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(m_miniMapFrameThickness / 2.f, m_miniMapFrameThickness / 2.f),
		m_miniMapCenter + Vec2(WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(m_miniMapFrameThickness / 2.f, -m_miniMapFrameThickness / 2.f), 
		m_miniMapFrameColor, m_miniMapFrameThickness);
	DrawLine(m_miniMapCenter + Vec2(WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(m_miniMapFrameThickness / 2.f, -m_miniMapFrameThickness / 2.f),
		m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(-m_miniMapFrameThickness / 2.f, -m_miniMapFrameThickness / 2.f), 
		m_miniMapFrameColor, m_miniMapFrameThickness);
	
	DrawAABB(m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -50),
		m_miniMapCenter + Vec2(WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -15),
		m_miniMapGridLineColor, m_miniMapGridLineColor, Rgba8(0, 0, 0, 0));
	std::vector<Vertex> textVertex;
	AddVertsForTextTriangles2D(textVertex, Stringf("SPEED:%f", m_player->m_curSpeed),
		m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(2.5f, -46.f),
		20.5f,
		Rgba8(0, 255, 0, 64), 0.5f);
	AddVertsForTextTriangles2D(textVertex, Stringf("SPEED:%f", m_player->m_curSpeed),
		m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(5.f, -45.f),
		20.f,
		Rgba8(0, 255, 0, 224), 0.5f);
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray((int)textVertex.size(), textVertex.data());
	DrawLine(m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -50) + Vec2(-m_miniMapFrameThickness / 2.f, -m_miniMapFrameThickness / 2.f),
		m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -15) + Vec2(-m_miniMapFrameThickness / 2.f, m_miniMapFrameThickness / 2.f),
		m_miniMapFrameColor, m_miniMapFrameThickness);
	DrawLine(m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -15) + Vec2(-m_miniMapFrameThickness / 2.f, m_miniMapFrameThickness / 2.f),
		m_miniMapCenter + Vec2(WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -15) + Vec2(m_miniMapFrameThickness / 2.f, m_miniMapFrameThickness / 2.f),
		m_miniMapFrameColor, m_miniMapFrameThickness);
	DrawLine(m_miniMapCenter + Vec2(WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -15) + Vec2(m_miniMapFrameThickness / 2.f, m_miniMapFrameThickness / 2.f),
		m_miniMapCenter + Vec2(WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -50) + Vec2(m_miniMapFrameThickness / 2.f, -m_miniMapFrameThickness / 2.f),
		m_miniMapFrameColor, m_miniMapFrameThickness);
	DrawLine(m_miniMapCenter + Vec2(WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -50) + Vec2(m_miniMapFrameThickness / 2.f, -m_miniMapFrameThickness / 2.f),
		m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio + Vec2(0, -50) + Vec2(-m_miniMapFrameThickness / 2.f, -m_miniMapFrameThickness / 2.f),
		m_miniMapFrameColor, m_miniMapFrameThickness);
}

//--------------------------------------------------------------------------------------------------
void Game::RenderUIWaves() const {
	float waveFraction = GetClamped(m_waveAnimationTimeCount / m_waveAnimationTotalTime, 0.f, 1.f);
	std::vector<Vertex> textVertex;
	AddVertsForTextTriangles2D(textVertex, Stringf("WAVE-%d", m_curLevelIndex+1),
		Vec2(0, 0),
		50.f,
		Rgba8(255, 255, 255, (unsigned char) (255.f * SinDegrees(waveFraction*180.f))), 0.5f);
	TransformVertexArrayXY3D((int)textVertex.size(), textVertex.data(), 1.f, 0.f,
		Vec2(SCREEN_CENTER_X - 100.f, SCREEN_CENTER_Y + 200.f));
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray((int)textVertex.size(), textVertex.data());
}

//--------------------------------------------------------------------------------------------------
void Game::RenderUIExpBar() const {
	DrawAABB(Vec2(15.f, 5.f), Vec2(SCREEN_SIZE_X - 15.f, 20.f),
		Rgba8(180, 180, 180, 255), Rgba8(224, 224, 224, 255), Rgba8(224, 224, 224, 255));

	float expGridSize = (SCREEN_SIZE_X - 35.f) / m_player->m_nextLevelExpVal;
	for (int i = 0; i < m_player->m_nextLevelExpVal; i++) {
		DrawAABB(Vec2(17.5f + i * expGridSize, 7.5f), Vec2(17.f + (i + 1) * expGridSize, 17.5f),
			Rgba8(64, 64, 64, 255), Rgba8(128, 128, 128, 255), Rgba8(64, 64, 64, 255));
	}
	for (int i = 0; i < m_player->m_expBarVal; i++) {
		DrawAABB(Vec2(17.5f + i * expGridSize, 7.5f), Vec2(17.5f + (i + 1) * expGridSize, 17.5f),
			Rgba8(64, 128, 64, 255), Rgba8(96, 200, 96, 128), Rgba8(96, 200, 96, 255));
	}
}

//--------------------------------------------------------------------------------------------------
void Game::RenderUIUpgrade() const {
	DrawAABB(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y),
		Rgba8(102, 204, 153, 64), Rgba8(102, 204, 153, 64), Rgba8(102, 204, 153, 16));

	std::vector<Vertex> textVertex;
	AddVertsForTextTriangles2D(textVertex, "LEVEL UP! UPGRADE YOUR SHIP", Vec2(SCREEN_CENTER_X - 450.f, SCREEN_SIZE_Y - 72.5f), 50.5f, Rgba8(0, 128, 0, 255));
	AddVertsForTextTriangles2D(textVertex, "LEVEL UP! UPGRADE YOUR SHIP", Vec2(SCREEN_CENTER_X - 445.f, SCREEN_SIZE_Y - 70.f), 50.f, Rgba8(0, 200, 0, 255));

	AddVertsForTextTriangles2D(textVertex, Stringf("SHIP MAX HEALTH: %f", m_player->m_healthBarMax), Vec2(15.f, SCREEN_SIZE_Y - 100.f), 10.f, Rgba8(0, 200, 0, 255));
	AddVertsForTextTriangles2D(textVertex, Stringf("SHIP MAX SHIELD: %f", m_player->m_shieldBarMax), Vec2(15.f, SCREEN_SIZE_Y - 115.f), 10.f, Rgba8(0, 200, 0, 255));
	AddVertsForTextTriangles2D(textVertex, Stringf("SHIELD RECOVER SPEED: %f grid/s", m_player->m_shieldRecoverSpeed), Vec2(15.f, SCREEN_SIZE_Y - 130.f), 10.f, Rgba8(0, 200, 0, 255));
	AddVertsForTextTriangles2D(textVertex, Stringf("SHIELD EXPLOSION RANGE: %f m", m_player->m_shieldExplosionRange), Vec2(15.f, SCREEN_SIZE_Y - 145.f), 10.f, Rgba8(0, 200, 0, 255));
	AddVertsForTextTriangles2D(textVertex, Stringf("FIRE INTERVAL: %f s", m_player->m_fireInterval), Vec2(15.f, SCREEN_SIZE_Y - 160.f), 10.f, Rgba8(0, 200, 0, 255));
	AddVertsForTextTriangles2D(textVertex, Stringf("FIRE BRANCH: %f", m_player->m_fireBranch), Vec2(15.f, SCREEN_SIZE_Y - 175.f), 10.f, Rgba8(0, 200, 0, 255));
	AddVertsForTextTriangles2D(textVertex, Stringf("BULLET EXPLOSION RANGE: %f m", m_player->m_bulletExplosionRange), Vec2(15.f, SCREEN_SIZE_Y - 190.f), 10.f, Rgba8(0, 200, 0, 255));
	AddVertsForTextTriangles2D(textVertex, Stringf("BULLET TRACK DURATION: %f s", m_player->m_bulletTrackDuration), Vec2(15.f, SCREEN_SIZE_Y - 205.f), 10.f, Rgba8(0, 200, 0, 255));

	AddVertsForTextTriangles2D(textVertex, "KEYBOARD: SELECT LEFT(S) SELECT RIGHT(F) CONFIRM(ENTER)", Vec2(15.f, 40.f), 10.f, Rgba8(0, 200, 0, 255));
	AddVertsForTextTriangles2D(textVertex, "CONTROLLER: SELECT LEFT(DEPAD-L) SELECT RIGHT(DEPAD-R) CONFIRM(X)", Vec2(15.f, 25.f), 10.f, Rgba8(0, 200, 0, 255));
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray((int)textVertex.size(), textVertex.data());

	std::vector<Vertex> shineTextVertex;
	AddVertsForTextTriangles2D(shineTextVertex, "LEVEL UP! UPGRADE YOUR SHIP", Vec2(SCREEN_CENTER_X - 445.f, SCREEN_SIZE_Y - 70.f), 50.f, Rgba8(0, 200, 0, 255));
	TransformVertexArrayShine((int)shineTextVertex.size(), shineTextVertex.data(), 1.f, 0.f,Vec2(0,0), 1.2f * (float)m_gameClock->GetTotalSeconds());
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray((int)shineTextVertex.size(), shineTextVertex.data());

	RenderUIUpgradeItem(Vec2(SCREEN_CENTER_X - 350.f, SCREEN_CENTER_Y), m_playerUpgradeItems[m_upgradeChooseHealth], 0);
	RenderUIUpgradeItem(Vec2(SCREEN_CENTER_X, SCREEN_CENTER_Y), m_playerUpgradeItems[m_upgradeChooseShield], 1);
	RenderUIUpgradeItem(Vec2(SCREEN_CENTER_X + 350.f, SCREEN_CENTER_Y), m_playerUpgradeItems[m_upgradeChooseBullet], 2);
}

void Game::RenderUIUpgradeItem(Vec2 center, PlayerUpgradeItem content, int index) const {
	DrawAABB(center + Vec2(-m_upgradeItemWidth/2.f, -m_upgradeItemHeight/2.f), 
		center + Vec2(m_upgradeItemWidth / 2.f, m_upgradeItemHeight / 2.f),
		m_upgradeGridLineColor, m_upgradeGridLineColor, Rgba8(0, 0, 0, 0));

	for (int i = 0; i <= m_upgradeGridNum; i++) {
		Vec2 xBias = Vec2(-m_upgradeItemWidth / 2.f + (float)i / (float)m_upgradeGridNum * m_upgradeItemWidth, m_upgradeItemHeight/2.f);
		Vec2 yBias = Vec2(-m_upgradeItemWidth / 2.f, m_upgradeItemHeight /2.f - (float)i / (float)m_upgradeGridNum * m_upgradeItemHeight);
		DrawLine(center + xBias, center + xBias + Vec2(0, -m_upgradeItemHeight), m_upgradeGridLineColor, m_upgradeGridLineThickness);
		DrawLine(center + yBias, center + yBias + Vec2(m_upgradeItemWidth, 0), m_upgradeGridLineColor, m_upgradeGridLineThickness);
	}

	std::vector<Vertex> textVertex;
	if (content.hpUpgrade != NONE_HEALTH_UPGRADE) {
		AddVertsForTextTriangles2D(textVertex, "HELTH UPGRADE", center + Vec2( - 85.f, m_upgradeItemHeight/2.f - 30.f), 20.f, Rgba8(0, 255, 0, 255));
	}
	else if (content.shieldUpgrade != NONE_SHIELD_UPGRADE) {
		AddVertsForTextTriangles2D(textVertex, "SHELD UPGRADE", center + Vec2(-85.f, m_upgradeItemHeight / 2.f - 30.f), 20.f, Rgba8(0, 255, 0, 255));
	}
	else if (content.bulletUpgrade != NONE_BULLET_UPGRADE) {
		AddVertsForTextTriangles2D(textVertex, "BULLET UPGRADE", center + Vec2(-90.f, m_upgradeItemHeight / 2.f - 30.f), 20.f, Rgba8(0, 255, 0, 255));
	}
	AddVertsForTextTriangles2D(textVertex, content.upgradeDescribe, center + Vec2(-75.f, 0.f), 15.f, Rgba8(0, 255, 0, 255));
	
	if (m_upgradeChoose == index) {
		AddVertsForTextTriangles2D(textVertex, Stringf("[Upgrade%d]", index+1), center + Vec2(-70.f, -m_upgradeItemHeight / 2.f - 30.f), 20.f, Rgba8(0, 255, 0, 255));
	}
	else {
		AddVertsForTextTriangles2D(textVertex, Stringf("Upgrade%d", index+1), center + Vec2(-55.f, -m_upgradeItemHeight / 2.f - 30.f), 20.f, Rgba8(0, 128, 0, 255));
	}
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray((int)textVertex.size(), textVertex.data());
}

//--------------------------------------------------------------------------------------------------
void Game::DelayQuit(float delayTime) {
	if (!m_delayQuitFlag) {
		if (m_isRemainEnemy) {
			SoundID quitSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/GameWin.mp3");
			g_engine->m_audio->StartSound(quitSound, false, 2.f, 0.f, 1.f);
		}

		m_delayQuitWaitedTime = 0.f;
		m_delayQuitDuration = delayTime;
		m_delayQuitFlag = true;
	}
}

//--------------------------------------------------------------------------------------------------
void Game::AddCameraShake(float amp) {
	m_curCameraShakeAmp = GetClamped(m_curCameraShakeAmp + amp, 0.f, m_maxCameraShakeAmp);
}

//--------------------------------------------------------------------------------------------------
void Game::DecayCameraShake() {
	m_curCameraShakeAmp = GetClamped(m_curCameraShakeAmp - (float)m_gameClock->GetDeltaSeconds() * m_cameraShakeDecaySpeed, 0.f, m_maxCameraShakeAmp);
}

//--------------------------------------------------------------------------------------------------
void Game::SetNextGameState(GameState nextState) {
	m_nextGameState = nextState;
}

//--------------------------------------------------------------------------------------------------
GameState const Game::GetCurGameState() const{
	return m_curGameState;
}

//--------------------------------------------------------------------------------------------------
void Game::UpdateToNextUpgradeChoose() {
	SoundID selectSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/ChooseUpgrade.mp3");
	g_engine->m_audio->StartSound(selectSound, false, 1.1f, 0.f, m_randomGenerator->RollRandomFloatInRange(0.9f, 1.1f));
	m_upgradeChoose = (m_upgradeChoose + 1) % m_upgradeChooseMax;
}

//--------------------------------------------------------------------------------------------------
void Game::UpdateToPreviousUpgradeChoose() {
	SoundID selectSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/ChooseUpgrade.mp3");
	g_engine->m_audio->StartSound(selectSound, false, 1.1f, 0.f, m_randomGenerator->RollRandomFloatInRange(0.9f, 1.1f));
	m_upgradeChoose = m_upgradeChoose - 1 > -1 ? m_upgradeChoose - 1 : m_upgradeChooseMax - 1;
}

//--------------------------------------------------------------------------------------------------
PlayerUpgradeItem const Game::GetChoseUpgrade() const {
	if (m_upgradeChoose == 0) {
		return m_playerUpgradeItems[m_upgradeChooseHealth];
	}

	if (m_upgradeChoose == 1) {
		return m_playerUpgradeItems[m_upgradeChooseShield];
	}
	
	return m_playerUpgradeItems[m_upgradeChooseBullet];
}

//--------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const {
	g_engine->m_renderer->BeginCamera(*m_viewCamera);
	float titleFraction = m_titleAnimationTimeCount / m_titleAnimationTotalTime;
	float boidsFraction = m_boidsAnimationTimeCount / m_boidsAnimationTotalTime;
	float playerFraction = m_boidsPlayerAnimationTimeCount / m_boidsPlayerAnimationTotalTime;

	//-----------------------------------------------------------------------------------------------
	Vertex worldMesh[WaspEnemy::m_vertexNum];
	for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i ++) {
		if (m_attractModeBoids[i].isActive) {
			if (m_attractModeBoids[i].boidsType == 0) {
				PlayerShip::GetLocalMesh(PlayerShip::m_vertexNum, worldMesh);
				float flameLength = -2.f - 2.f * (m_attractModeBoids[i].velocity.GetLength() / m_boidPlayerMaxSpeed) * (m_randomGenerator->RollRandomFloatZeroToOne() * 0.5f + 1.f);
				worldMesh[15] = Vertex(Vec3(-2.f, 1.f, 0.f), Rgba8(255, 200, 0, 255), Vec2(0.f, 0.f));
				worldMesh[16] = Vertex(Vec3(-2.f, -1.f, 0.f), Rgba8(255, 200, 0, 255), Vec2(0.f, 0.f));
				worldMesh[17] = Vertex(Vec3(flameLength, 0.f, 0.f), Rgba8(255, 0, 0, 0), Vec2(0.f, 0.f));
				TransformVertexArrayXY3D(PlayerShip::m_vertexNum, worldMesh,
					4.f, m_attractModeBoids[i].velocity.GetOrientationDegrees(),
					m_attractModeBoids[i].position);
				g_engine->m_renderer->BindTexture(nullptr);
				g_engine->m_renderer->DrawVertexArray(PlayerShip::m_vertexNum, worldMesh);

				DrawDisc(m_attractModeBoids[i].position, 
							  m_attractModeBoids[i].nearbyRadius, 
							  Rgba8(0, 255, 0, (unsigned char)Interpolate(64.f, 0.f, playerFraction)), Rgba8(0, 255, 0, 0));
				DrawDisc(m_attractModeBoids[i].position, 
							  Interpolate(0, m_attractModeBoids[i].nearbyRadius, playerFraction), 
							  Rgba8(0, 255, 0, 64), Rgba8(0, 255, 0, 0));
				DrawDisc(m_attractModeBoids[i].position, 20.f, Rgba8(102, 153, 204, 255), Rgba8(102, 153, 204, 0));
				for (int j = 0; j < m_attractModeBoids[i].neighborCount; j++) {
					Vec2 toPlayer = m_attractModeBoids[i].position - m_boidPlayerNeighbors[j].position;
					float distance = toPlayer.GetLength();
					if (distance > 1.5f * m_attractModeBoids[i].nearbyRadius) {
						break;
					}
					toPlayer /= distance;
					DrawLine(m_attractModeBoids[i].position - toPlayer * 20.f,
						          m_boidPlayerNeighbors[j].position + toPlayer * 20.f, Rgba8(255, 0, 0, (unsigned char)Interpolate(128.f, 32.f, playerFraction)), 1.f);
					DrawDisc(m_boidPlayerNeighbors[j].position,20.f, Rgba8(255, 0, 0, (unsigned char)Interpolate(128.f, 32.f, playerFraction)), Rgba8(255, 0, 0, 0));
				}
			}
			else if (m_attractModeBoids[i].boidsType == 1) {
				BeetleEnemy::GetLocalMesh(BeetleEnemy::m_vertexNum, worldMesh);
				for (int j = BeetleEnemy::m_vertexNum - 12; j < BeetleEnemy::m_vertexNum - 7; j++) {
					worldMesh[j] = Interpolate(worldMesh[j], worldMesh[j + 6], SinDegrees(boidsFraction * 180.f));
				}
				TransformVertexArrayXY3D(BeetleEnemy::m_vertexNum - 6, worldMesh,
					4.f, m_attractModeBoids[i].velocity.GetOrientationDegrees(),
					m_attractModeBoids[i].position);
				g_engine->m_renderer->BindTexture(nullptr);
				g_engine->m_renderer->DrawVertexArray(BeetleEnemy::m_vertexNum - 6, worldMesh);
			}
			else if (m_attractModeBoids[i].boidsType == 2) {
				WaspEnemy::GetLocalMesh(WaspEnemy::m_vertexNum, worldMesh);
				for (int j = WaspEnemy::m_vertexNum - 24; j < WaspEnemy::m_vertexNum - 12; j++) {
					worldMesh[j] = Interpolate(worldMesh[j], worldMesh[j + 12], SinDegrees(boidsFraction * 180.f));
				}
				TransformVertexArrayXY3D(WaspEnemy::m_vertexNum - 12, worldMesh,
					4.f, m_attractModeBoids[i].velocity.GetOrientationDegrees(),
					m_attractModeBoids[i].position);
				g_engine->m_renderer->BindTexture(nullptr);
				g_engine->m_renderer->DrawVertexArray(WaspEnemy::m_vertexNum - 12, worldMesh);
			}
		}
	}

	//-----------------------------------------------------------------------------------------------
	std::vector<Vertex> textVertex;
	AddVertsForTextTriangles2D(textVertex, "GOLD",
							Vec2(SCREEN_CENTER_X + 292.5f, SCREEN_SIZE_Y - 229.f),
							155.f * Interpolate(m_titleScaleMin, m_titleScaleMax, SinDegrees(titleFraction * 180.f)),
							Rgba8(128, 96, 0, 255), 0.5f);
	AddVertsForTextTriangles2D(textVertex, "GOLD",
							Vec2(SCREEN_CENTER_X + 300.f, SCREEN_SIZE_Y - 225.f),
							150.f * Interpolate(m_titleScaleMin, m_titleScaleMax, SinDegrees(titleFraction * 180.f)),
							Rgba8(255, 192, 0, 255), 0.5f);
	TransformVertexArrayXY3D((int)textVertex.size(), textVertex.data(), 1.f, 0.f,
							Vec2(0, Interpolate(-m_titleUpDownMax, m_titleUpDownMax, SinDegrees(titleFraction * 180.f))));
	AddVertsForTextTriangles2D(textVertex, "STARTSHIP", Vec2(SCREEN_CENTER_X - 690.f, SCREEN_SIZE_Y - 229.f), 155.f, Rgba8(128, 128, 128, 255));
	AddVertsForTextTriangles2D(textVertex, "STARTSHIP", Vec2(SCREEN_CENTER_X - 675.f, SCREEN_SIZE_Y - 225.f), 150.f, Rgba8(200, 200, 200, 255));

	AddVertsForTextTriangles2D(textVertex, "PRESS KEYBOARD (SPACE) OR CONTROLLER (A) START GAME", 
							Vec2(SCREEN_CENTER_X - 280.f,  30.f), 15.f, 
							Rgba8(255, 255, 255, (unsigned char)(SinDegrees(titleFraction * 180.f) *200.f + 55.f)));
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray((int)textVertex.size(), textVertex.data());

	std::vector<Vertex> shineTextVertex;
	AddVertsForTextTriangles2D(shineTextVertex, "GOLD",
							Vec2(SCREEN_CENTER_X + 300.f, SCREEN_SIZE_Y - 225.f),
							150.f * Interpolate(m_titleScaleMin, m_titleScaleMax, SinDegrees(titleFraction * 180.f)),
							Rgba8(255, 255, 255, 0), 0.5f);

	AddVertsForTextTriangles2D(shineTextVertex, "STARTSHIP", 
							Vec2(SCREEN_CENTER_X - 675.f, SCREEN_SIZE_Y - 225.f), 150.f, Rgba8(255, 255, 255, 0));

	TransformVertexArrayShine((int)shineTextVertex.size(), shineTextVertex.data(), 1.f, 0.f,
							Vec2(0, Interpolate(-m_titleUpDownMax, m_titleUpDownMax, SinDegrees(titleFraction * 180.f))), 1.2f* (float)m_gameClock->GetTotalSeconds());
	g_engine->m_renderer->BindTexture(nullptr);
	g_engine->m_renderer->DrawVertexArray((int)shineTextVertex.size(), shineTextVertex.data());

	g_engine->m_renderer->EndCamera(*m_viewCamera);
}

//--------------------------------------------------------------------------------------------------
void Game::GenerateStarsMesh(int starCount, Vertex* starsMesh, Vec2 buttomLeft, Vec2 topRight, float starSizeMin, float starSizeMax, Rgba8 starColor1, Rgba8 starColor2) {
	for (int i = 0; i < starCount * 24; i += 24) {
		Vec2 randomPos = Vec2(m_randomGenerator->RollRandomFloatInRange(buttomLeft.x, topRight.x),
							  m_randomGenerator->RollRandomFloatInRange(buttomLeft.x, topRight.x));
		float randomSize = m_randomGenerator->RollRandomFloatInRange(starSizeMin, starSizeMax);
		Rgba8 randomColor = Rgba8(
			(unsigned char)m_randomGenerator->RollRandomFloatInRange(starColor1.r, starColor2.r),
			(unsigned char)m_randomGenerator->RollRandomFloatInRange(starColor1.g, starColor2.g),
			(unsigned char)m_randomGenerator->RollRandomFloatInRange(starColor1.b, starColor2.b),
			(unsigned char)m_randomGenerator->RollRandomFloatInRange(starColor1.a, starColor2.a)
		);

		//Star Mesh
		{
			starsMesh[i] = Vertex(
				Vec3(randomPos.x, randomPos.y + randomSize, 0.f),
				randomColor,
				Vec2(0, 0)
			);
			starsMesh[i + 1] = Vertex(
				Vec3(randomPos.x + randomSize / 2.f, randomPos.y + randomSize / 2.f, 0.f),
				Rgba8(0, 0, 0, 0),
				Vec2(0, 0)
			);
			starsMesh[i + 2] = Vertex(
				Vec3(randomPos.x, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);

			starsMesh[i + 3] = Vertex(
				Vec3(randomPos.x + randomSize / 2.f, randomPos.y + randomSize / 2.f, 0.f),
				Rgba8(0, 0, 0, 0),
				Vec2(0, 0)
			);
			starsMesh[i + 4] = Vertex(
				Vec3(randomPos.x + randomSize, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);
			starsMesh[i + 5] = Vertex(
				Vec3(randomPos.x, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);

			starsMesh[i + 6] = Vertex(
				Vec3(randomPos.x + randomSize, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);
			starsMesh[i + 7] = Vertex(
				Vec3(randomPos.x + randomSize / 2.f, randomPos.y - randomSize / 2.f, 0.f),
				Rgba8(0, 0, 0, 0),
				Vec2(0, 0)
			);
			starsMesh[i + 8] = Vertex(
				Vec3(randomPos.x, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);

			starsMesh[i + 9] = Vertex(
				Vec3(randomPos.x + randomSize / 2.f, randomPos.y - randomSize / 2.f, 0.f),
				Rgba8(0, 0, 0, 0),
				Vec2(0, 0)
			);
			starsMesh[i + 10] = Vertex(
				Vec3(randomPos.x, randomPos.y - randomSize, 0.f),
				randomColor,
				Vec2(0, 0)
			);
			starsMesh[i + 11] = Vertex(
				Vec3(randomPos.x, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);

			starsMesh[i + 12] = Vertex(
				Vec3(randomPos.x, randomPos.y - randomSize, 0.f),
				randomColor,
				Vec2(0, 0)
			);
			starsMesh[i + 13] = Vertex(
				Vec3(randomPos.x - randomSize / 2.f, randomPos.y - randomSize / 2.f, 0.f),
				Rgba8(0, 0, 0, 0),
				Vec2(0, 0)
			);
			starsMesh[i + 14] = Vertex(
				Vec3(randomPos.x, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);

			starsMesh[i + 15] = Vertex(
				Vec3(randomPos.x - randomSize / 2.f, randomPos.y - randomSize / 2.f, 0.f),
				Rgba8(0, 0, 0, 0),
				Vec2(0, 0)
			);
			starsMesh[i + 16] = Vertex(
				Vec3(randomPos.x - randomSize, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);
			starsMesh[i + 17] = Vertex(
				Vec3(randomPos.x, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);

			starsMesh[i + 18] = Vertex(
				Vec3(randomPos.x - randomSize, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);
			starsMesh[i + 19] = Vertex(
				Vec3(randomPos.x - randomSize / 2.f, randomPos.y + randomSize / 2.f, 0.f),
				Rgba8(0, 0, 0, 0),
				Vec2(0, 0)
			);
			starsMesh[i + 20] = Vertex(
				Vec3(randomPos.x, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);

			starsMesh[i + 21] = Vertex(
				Vec3(randomPos.x - randomSize / 2.f, randomPos.y + randomSize / 2.f, 0.f),
				Rgba8(0, 0, 0, 0),
				Vec2(0, 0)
			);
			starsMesh[i + 22] = Vertex(
				Vec3(randomPos.x, randomPos.y + randomSize, 0.f),
				randomColor,
				Vec2(0, 0)
			);
			starsMesh[i + 23] = Vertex(
				Vec3(randomPos.x, randomPos.y, 0.f),
				randomColor,
				Vec2(0, 0)
			);
		}
	}
}

//--------------------------------------------------------------------------------------------------
void Game::RenderStars() const {
	if (m_curGameState == GAME_ATTRACT_MODE) {
		g_engine->m_renderer->BindTexture(nullptr);
		g_engine->m_renderer->DrawVertexArray(MAX_ATTRACTMODE_STAR * 24, m_attractModeStarsMesh);
	}
	else if (m_curGameState == GAME_PLAYING_MODE) {
		TransformVertexArrayXY3D(MAX_FAR_STAR * 24, m_attractModeFarStarsMesh, 1.f, 0.f, -m_player->m_velocity * 0.0125f * (float)m_gameClock->GetDeltaSeconds());
		g_engine->m_renderer->BindTexture(nullptr);
		g_engine->m_renderer->DrawVertexArray(MAX_FAR_STAR * 24, m_attractModeFarStarsMesh);

		TransformVertexArrayXY3D(MAX_NEAR_STAR * 24, m_attractModeNearStarsMesh, 1.f, 0.f, -m_player->m_velocity * 0.025f * (float)m_gameClock->GetDeltaSeconds());
		g_engine->m_renderer->BindTexture(nullptr);
		g_engine->m_renderer->DrawVertexArray(MAX_NEAR_STAR * 24, m_attractModeNearStarsMesh);
	}
	else {
		g_engine->m_renderer->BindTexture(nullptr);
		g_engine->m_renderer->DrawVertexArray(MAX_FAR_STAR * 24, m_attractModeFarStarsMesh);
		g_engine->m_renderer->DrawVertexArray(MAX_NEAR_STAR * 24, m_attractModeNearStarsMesh);
	}
}

void Game::RenderWorldBoundary() const {
	DrawAABB(Vec2(-50, -50), Vec2(WORLD_SIZE_X+50.f, WORLD_SIZE_Y+50.f), Rgba8(255, 0, 255, 32), Rgba8(255, 0, 255, 32), Rgba8(0, 0, 0, 0));
	DrawAABB(Vec2(-50, -50), Vec2(WORLD_SIZE_X+50.f, WORLD_SIZE_Y+50.f), Rgba8(100, 0, 255, 16), Rgba8(100, 0, 255, 16), Rgba8(0, 0, 0, 0));
}

//--------------------------------------------------------------------------------------------------
void Game::UpdateAllAttractModeBoids() {
	for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i++) {
		if (m_attractModeBoids[i].isActive) {
			UpdateOneAttractModeBoid(&m_attractModeBoids[i]);
		}
	}
	for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i++) {
		if (m_attractModeBoids[i].isActive) {
			m_attractModeBoids[i].position = m_attractModeBoids[i].nextPosition;
			m_attractModeBoids[i].velocity = m_attractModeBoids[i].nextVelocity;
		}
	}
}

//--------------------------------------------------------------------------------------------------
void Game::UpdateOneAttractModeBoid(AttractModeBoidsEntity* boid) {
	boid->neighborCount = 0;
	for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i++) {
		if (&m_attractModeBoids[i] != boid && m_attractModeBoids[i].isActive) {
			float distance = GetDistance2D(boid->position, m_attractModeBoids[i].position);
			if (distance < boid->nearbyRadius) {
				m_boidsNeighbors[boid->neighborCount] = m_attractModeBoids[i];
				if (boid->boidsType == 0) {
					m_boidPlayerNeighbors[boid->neighborCount] = m_attractModeBoids[i];
				}
				boid->neighborCount++;
			}
		}
	}

	Vec2 separation(0.f, 0.f);
	Vec2 alignment(0.f, 0.f);
	Vec2 cohesion(0.f, 0.f);

	if (boid->neighborCount > 0) {
		for (int i = 0; i < boid->neighborCount; i++) {
			Vec2 away = boid->position - m_boidsNeighbors[i].position;
			float sqrDistance = away.GetLengthSquared();
			if (sqrDistance < boid->bounderyRadius * boid->bounderyRadius) {
				separation += away / sqrDistance;
			}
		}
		if (boid->boidsType !=0) {
			separation *= m_boidsSeprationWeight;
		}
		else {
			separation *= m_boidPlayerSeprationWeight;
		}

		for (int i = 0; i < boid->neighborCount; i++) {
			alignment += m_boidsNeighbors[i].velocity;
		}
		alignment /= (float)boid->neighborCount;
		alignment *= m_boidsAlignmentWeight;

		for (int i = 0; i < boid->neighborCount; i++) {
			cohesion += m_boidsNeighbors[i].position;
		}
		cohesion /= (float)boid->neighborCount;
		cohesion = (cohesion - boid->position) * m_boidsCohesionWeight;
	}
	else {
		Vec2 screenCenter(SCREEN_CENTER_X, SCREEN_CENTER_Y);
		Vec2 toCenter = screenCenter - boid->position;
		toCenter.Normalize();
		Vec2 centerForce = toCenter * m_boidCenterPullWeight; 

		Vec2 toPlayer = m_attractModeBoids[0].position - boid->position;
		toPlayer.Normalize();
		Vec2 playerForce = toPlayer * m_boidPlayerPullWeight;

		if (boid->boidsType == 0) {
			boid->nextVelocity = boid->velocity + centerForce * (float)m_gameClock->GetDeltaSeconds();
		}
		else {
			boid->nextVelocity = boid->velocity + playerForce * (float)m_gameClock->GetDeltaSeconds();
		}
		boid->nextVelocity = boid->nextVelocity.GetClamped(m_boidBeetleMaxSpeed);

		boid->nextPosition = boid->position + boid->nextVelocity * (float)m_gameClock->GetDeltaSeconds();
		boid->nextPosition.x = GetClamped(boid->nextPosition.x, 0.f, SCREEN_SIZE_X);
		boid->nextPosition.y = GetClamped(boid->nextPosition.y, 0.f, SCREEN_SIZE_Y);
		return;
	}


	if (boid->position.x <= 0) {
		boid->position.x = SCREEN_SIZE_X;
	}
	else if (boid->position.x >= SCREEN_SIZE_X) {
		boid->position.x = 0;
	}
	if (boid->position.y <= 0) {
		boid->position.y = SCREEN_SIZE_Y;
	}
	else if (boid->position.y >= SCREEN_SIZE_Y) {
		boid->position.y = 0;
	}

	Vec2 screenCenter(SCREEN_CENTER_X, SCREEN_CENTER_Y);
	Vec2 toCenter = screenCenter - boid->position;
	toCenter.Normalize();
	Vec2 centerForce = toCenter * m_boidCenterPullWeight;

	Vec2 toPlayer = m_attractModeBoids[0].position - boid->position;
	toPlayer.Normalize();
	Vec2 playerForce = toPlayer * m_boidPlayerPullWeight;

	if (boid->boidsType == 1) {
		boid->nextVelocity = boid->velocity
			+ (separation + alignment + cohesion + playerForce + centerForce) * (float)m_gameClock->GetDeltaSeconds();

		boid->nextVelocity = boid->nextVelocity.GetClamped(m_boidBeetleMaxSpeed);
	}
	else if (boid->boidsType == 2) {
		boid->nextVelocity = boid->velocity
			+ (separation + alignment + cohesion + playerForce*2 + centerForce) * (float)m_gameClock->GetDeltaSeconds();

		boid->nextVelocity = boid->nextVelocity.GetClamped(m_boidWaspMaxSpeed);
	}
	else {
		boid->nextVelocity = boid->velocity
			+ (separation + centerForce) * (float)m_gameClock->GetDeltaSeconds();

		boid->nextVelocity = boid->nextVelocity.GetClamped(m_boidPlayerMaxSpeed);
	}
	boid->nextPosition = boid->position + boid->nextVelocity * (float)m_gameClock->GetDeltaSeconds();

	boid->nextPosition.x = GetClamped(boid->nextPosition.x, 0.f, SCREEN_SIZE_X);
	boid->nextPosition.y = GetClamped(boid->nextPosition.y, 0.f, SCREEN_SIZE_Y);
}

//--------------------------------------------------------------------------------------------------
void Game::KillAllEntities() {
	for (int i = 0; i < MAX_BEETLES; i++) {
		if (m_beetleEnemy[i] != nullptr) {
			m_beetleEnemy[i]->Die();
		}
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_waspEnemy[i] != nullptr) {
			m_waspEnemy[i]->Die();
		}
	}
	for (int i = 0; i < MAX_ASTEROIDS; i++) {
		if (m_asteroids[i] != nullptr) {
			m_asteroids[i]->Die();
		}
	}
}

//-------------------------------------------------------------------------------------------------
void Game::RollUpgrades() {
	m_upgradeChooseHealth = 0;
	m_upgradeChooseShield = m_randomGenerator->RollRandomIntInRange(1, 3);
	m_upgradeChooseBullet = m_randomGenerator->RollRandomIntInRange(4, 7);
}