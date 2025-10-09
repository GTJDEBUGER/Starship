#include <iostream>

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Bullet.hpp"
#include "Game/BeetleEnemy.hpp"
#include "Game/WaspEnemy.hpp"
#include "Game/App.hpp"
#include "Game/Debris.hpp"

//-----------------------------------------------------------------------------------------------
Game::Game()
{
	InitialObjectPools();
	SetUpLevel(m_curLevelIndex);
	m_player = new PlayerShip(this);
	m_worldCamera = new Camera(Vec2(0.f, 0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
	m_screenCamera = new Camera(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	m_randomGenerator = new RandomNumberGenerator();

	m_attractModeBoids = new AttractModeBoidsEntity[MAX_ATTRACTMODE_BOIDS];

	m_attractModeBoids[0].boidsType = 0;
	m_attractModeBoids[0].isActive = true;
	m_attractModeBoids[0].worldPos = Vec2(SCREEN_CENTER_X,SCREEN_CENTER_Y);
	m_attractModeBoids[0].velocity = Vec2(m_randomGenerator->RollRandomFloatInRange(-1.f, 1.f),
		m_randomGenerator->RollRandomFloatInRange(-1.f, 1.f));
	m_attractModeBoids[0].velocity.Normalize();
	m_attractModeBoids[0].velocity *= m_boidsMaxSpeed * m_randomGenerator->RollRandomFloatInRange(25.f, 75.f);
	m_attractModeBoids[0].nearbyRadius = 500.f;

	for (int i = 1; i < MAX_ATTRACTMODE_BOIDS; i++) {
		m_attractModeBoids[i].boidsType = 1;
		m_attractModeBoids[i].isActive = true;
		m_attractModeBoids[i].worldPos = Vec2(m_randomGenerator->RollRandomFloatInRange(0.f, SCREEN_SIZE_X),
			m_randomGenerator->RollRandomFloatInRange(0.f, SCREEN_SIZE_Y));
		m_attractModeBoids[i].velocity = Vec2(m_randomGenerator->RollRandomFloatInRange(-1.f, 1.f),
			m_randomGenerator->RollRandomFloatInRange(-1.f, 1.f));
		m_attractModeBoids[i].velocity.Normalize();
		m_attractModeBoids[i].velocity *= m_boidsMaxSpeed * m_randomGenerator->RollRandomFloatInRange(25.f, 75.f);
	}
}

//-----------------------------------------------------------------------------------------------
Game::~Game()
{
	delete m_randomGenerator;
	m_randomGenerator = nullptr;

	delete m_worldCamera;
	m_worldCamera = nullptr;

	delete m_screenCamera;
	m_screenCamera = nullptr;

	delete m_player;
	m_player = nullptr;
	DeleteObjectPools();
}

//-----------------------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	if (m_nextGameState != m_curGameState) {
		m_curGameState = m_nextGameState;
	}
	//-----------------------------------------------------------------------------------------------

	if (m_curGameState == GAME_PLAYING) {
		//-----------------------------------------------------------------------------------------------
		if (m_quitFlag) {
			m_waitedTime += deltaSeconds;
			if (m_waitedTime > m_delayTime) {
				m_nextGameState = GAME_ATTRACT_MODE;
			}
		}
		//-----------------------------------------------------------------------------------------------
		if (m_player != nullptr && !m_player->m_isDead) {
			m_player->Update(deltaSeconds);
		}
		UpdateEntities(deltaSeconds);
		HandleEntitiesCollide();
		//-----------------------------------------------------------------------------------------------
		HandleFiringInput();
		//-----------------------------------------------------------------------------------------------
		HandleRespawnAsteroidInput();
		//-----------------------------------------------------------------------------------------------
		HandleRespawnPlayerInput();
		//-----------------------------------------------------------------------------------------------
		CheckGotoNextLevel();

		//-----------------------------------------------------------------------------------------------
		DecayCameraShake(deltaSeconds);
		m_worldCamera->ResetOrthoViewToBase();
		m_worldCamera->Translate2D(Vec2(m_randomGenerator->RollRandomFloatZeroToOne() * m_curCameraShakeAmp,
			m_randomGenerator->RollRandomFloatZeroToOne() * m_curCameraShakeAmp));
	}
	
	//-----------------------------------------------------------------------------------------------
	if(m_curGameState == GAME_ATTRACT_MODE) {
		UpdateAllAttractModeBoids(deltaSeconds);

		m_startButtonAnimationTimeCount += deltaSeconds;
		if (m_startButtonAnimationTimeCount > m_startButtonAnimationTotalTime) {
			m_startButtonAnimationTimeCount = 0;
		}

		m_titleAnimationTimeCount += deltaSeconds;
		if (m_titleAnimationTimeCount > m_titleAnimationTotalTime) {
			m_titleAnimationTimeCount = 0;
		}

		m_boidsAnimationTimeCount += deltaSeconds;
		if (m_boidsAnimationTimeCount > m_boidsAnimationTotalTime) {
			m_boidsAnimationTimeCount = 0;
		}
	}
}

//---------------------------------------------------------------------------------------------------
void Game::Render() const
{
	if (m_curGameState == GAME_PLAYING) {
		//-----------------------------------------------------------------------------------------------
		g_engine->m_renderer->BeginCamera(*m_worldCamera);

		RenderPoolEntitys();

		if (!m_player->m_isDead) {
			m_player->Render();
		}

		if (g_app->m_isDebugDraw) {
			RenderDebugThings();
		}

		g_engine->m_renderer->EndCamera(*m_worldCamera);

		//-----------------------------------------------------------------------------------------------

		g_engine->m_renderer->BeginCamera(*m_screenCamera);

		RenderUI();

		g_engine->m_renderer->EndCamera(*m_screenCamera);
	}

	if (m_curGameState == GAME_ATTRACT_MODE) {
		g_engine->m_renderer->BeginCamera(*m_screenCamera);

		if (g_app->m_isDebugDraw) {
			RenderDebugThings();
		}
		RenderAttractMode();

		g_engine->m_renderer->EndCamera(*m_screenCamera);
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
}

//---------------------------------------------------------------------------------------------------
void Game::UpdateEntities(float deltaSeconds) {
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr) {
			m_asteroids[i]->Update(deltaSeconds);
			if (m_asteroids[i]->m_isDead) {
				delete m_asteroids[i];
				m_asteroids[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] != nullptr) {
			m_bullets[i]->Update(deltaSeconds);
			if (m_bullets[i]->m_isDead) {
				delete m_bullets[i];
				m_bullets[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetleEnemy[i] != nullptr) {
			m_beetleEnemy[i]->Update(deltaSeconds);
			if (m_beetleEnemy[i]->m_isDead) {
				delete m_beetleEnemy[i];
				m_beetleEnemy[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_waspEnemy[i] != nullptr) {
			m_waspEnemy[i]->Update(deltaSeconds);
			if (m_waspEnemy[i]->m_isDead) {
				m_waspEnemy[i] = nullptr;
			}
		}
	}
	for (int i = 0; i < MAX_DEBRIS; i++) {
		if (m_debris[i] != nullptr) {
			m_debris[i]->Update(deltaSeconds);
			if (m_debris[i]->m_isDead) {
				m_debris[i] = nullptr;
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

	for (int i = 0; i < MAX_ASTEROIDS; i++) {
		for (int j = i + 1; j < MAX_ASTEROIDS; j++) {
			if (m_asteroids[i] != nullptr && m_asteroids[j] != nullptr) {
				if (PushDiscsOutOfEachOther2D(m_asteroids[i]->m_position, ASTEROID_PHYSICS_RADIUS,
					               m_asteroids[j]->m_position, ASTEROID_PHYSICS_RADIUS)) {
					if (DotProduct2D(m_asteroids[i]->m_velocity, m_asteroids[j]->m_velocity) < 0.f) {
						m_asteroids[i]->m_velocity.Reflect(m_asteroids[i]->m_position - m_asteroids[j]->m_position);
						m_asteroids[i]->m_velocity.Normalize();
						m_asteroids[j]->m_velocity.Reflect(m_asteroids[j]->m_position - m_asteroids[i]->m_position);
						m_asteroids[j]->m_velocity.Normalize();
					}
					else {
						float asteroidISpeed = m_asteroids[i]->m_velocity.GetLength();
						float asteroidJSpeed = m_asteroids[j]->m_velocity.GetLength();
						
						m_asteroids[i]->m_velocity = m_asteroids[i]->m_velocity / asteroidISpeed * asteroidJSpeed;
						m_asteroids[j]->m_velocity = m_asteroids[j]->m_velocity / asteroidJSpeed * asteroidISpeed;
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------
void Game::HandleFiringInput() {
	if (g_app->m_isFiring && m_player != nullptr) {
		if (!m_player->m_isDead) {
			int freeBulletIndex = -1;
			for (int i = 0; i < MAX_BULLETS; i++) {
				if (m_bullets[i] == nullptr) {
					freeBulletIndex = i;
					break;
				}
			}

			if (freeBulletIndex > -1) {
				m_bullets[freeBulletIndex] = new Bullet(this, m_player->m_position + m_player->GetForwardVector(),
					m_player->GetForwardVector());

				SoundID shootSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/ShootBullet.mp3");
				g_engine->m_audio->StartSound(shootSound,false,1.0f,0.f, m_randomGenerator->RollRandomFloatInRange(0.8f, 1.1f));
			}
			else {
				ERROR_RECOVERABLE("Run out bullets!")
			}
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
}

//---------------------------------------------------------------------------------------------------
void Game::RenderDebugThings() const {
	if (m_curGameState == GAME_PLAYING) {
		float drawThickness = .2f;
		if (m_player != nullptr)
		{
			DebugDrawRing(m_player->m_position, m_player->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
			DebugDrawRing(m_player->m_position, m_player->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
			DebugDrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector() * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
			DebugDrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector().GetRotatedBy90Degrees() * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
			DebugDrawLine(m_player->m_position, m_player->m_position + m_player->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
		}

		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (m_asteroids[i] != nullptr) {
				DebugDrawLine(m_asteroids[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DebugDrawRing(m_asteroids[i]->m_position, m_asteroids[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DebugDrawRing(m_asteroids[i]->m_position, m_asteroids[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DebugDrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + Vec2(CosDegrees(m_asteroids[i]->m_orientationDegrees), SinDegrees(m_asteroids[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DebugDrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + Vec2(-SinDegrees(m_asteroids[i]->m_orientationDegrees), CosDegrees(m_asteroids[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DebugDrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + m_asteroids[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}

		for (int i = 0; i < MAX_BULLETS; i++) {
			if (m_bullets[i] != nullptr) {
				DebugDrawLine(m_bullets[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DebugDrawRing(m_bullets[i]->m_position, m_bullets[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DebugDrawRing(m_bullets[i]->m_position, m_bullets[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DebugDrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + Vec2(CosDegrees(m_bullets[i]->m_orientationDegrees), SinDegrees(m_bullets[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DebugDrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + Vec2(-SinDegrees(m_bullets[i]->m_orientationDegrees), CosDegrees(m_bullets[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DebugDrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + m_bullets[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}

		for (int i = 0; i < MAX_BEETLES; i++) {
			if (m_beetleEnemy[i] != nullptr) {
				DebugDrawLine(m_beetleEnemy[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DebugDrawRing(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DebugDrawRing(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DebugDrawLine(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_position + Vec2(CosDegrees(m_beetleEnemy[i]->m_orientationDegrees), SinDegrees(m_beetleEnemy[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DebugDrawLine(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_position + Vec2(-SinDegrees(m_beetleEnemy[i]->m_orientationDegrees), CosDegrees(m_beetleEnemy[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DebugDrawLine(m_beetleEnemy[i]->m_position, m_beetleEnemy[i]->m_position + m_beetleEnemy[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}

		for (int i = 0; i < MAX_WASPS; i++) {
			if (m_waspEnemy[i] != nullptr) {
				DebugDrawLine(m_waspEnemy[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DebugDrawRing(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DebugDrawRing(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DebugDrawLine(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_position + Vec2(CosDegrees(m_waspEnemy[i]->m_orientationDegrees), SinDegrees(m_waspEnemy[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DebugDrawLine(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_position + Vec2(-SinDegrees(m_waspEnemy[i]->m_orientationDegrees), CosDegrees(m_waspEnemy[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DebugDrawLine(m_waspEnemy[i]->m_position, m_waspEnemy[i]->m_position + m_waspEnemy[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}

		for (int i = 0; i < MAX_DEBRIS; i++) {
			if (m_debris[i] != nullptr) {
				DebugDrawLine(m_debris[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255), drawThickness);

				DebugDrawRing(m_debris[i]->m_position, m_debris[i]->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
				DebugDrawRing(m_debris[i]->m_position, m_debris[i]->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
				DebugDrawLine(m_debris[i]->m_position, m_debris[i]->m_position + Vec2(CosDegrees(m_debris[i]->m_orientationDegrees), SinDegrees(m_debris[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
				DebugDrawLine(m_debris[i]->m_position, m_debris[i]->m_position + Vec2(-SinDegrees(m_debris[i]->m_orientationDegrees), CosDegrees(m_debris[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
				DebugDrawLine(m_debris[i]->m_position, m_debris[i]->m_position + m_debris[i]->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);
			}
		}
	}
	else if (m_curGameState == GAME_ATTRACT_MODE) {
		for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i++) {
			if (m_attractModeBoids[i].isActive) {
				DebugDrawRing(m_attractModeBoids[i].worldPos, m_attractModeBoids[i].bounderyRadius, Rgba8(255, 0, 0, 255), 1.f);
				DebugDrawRing(m_attractModeBoids[i].worldPos, m_attractModeBoids[i].nearbyRadius, Rgba8(0, 255, 0, 255), 1.f);
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------
void Game::SetUpLevel(int levelIndex) {
	for (int i = 0; i < levels[levelIndex].asteroidNum; i++) {
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
	for (int i = 0; i < levels[levelIndex].beetleEnemy; i++) {
		m_beetleEnemy[i] = new BeetleEnemy(this);
	}
	for (int i = 0; i < levels[levelIndex].waspEnemy; i++) {
		m_waspEnemy[i] = new WaspEnemy(this);
	}
}

//--------------------------------------------------------------------------------------------------
void Game::CheckGotoNextLevel() {
	bool flag = true;
	for (int i = 0; i < MAX_BEETLES; i++) {
		if (m_beetleEnemy[i] != nullptr || !flag) {
			flag = false;
			break;
		}
	}
	for (int i = 0; i < MAX_WASPS; i++) {
		if (m_waspEnemy[i] != nullptr || !flag) {
			flag = false;
			break;
		}
	}

	if (flag) {
		m_curLevelIndex++;
		if (m_curLevelIndex < sizeof(levels) / sizeof(GameLevel)) {
			SetUpLevel(m_curLevelIndex);
		}
		else {
			DelayQuit(2.f);
		}
	}
}

//--------------------------------------------------------------------------------------------------
void Game::RenderUI() const {
	Vertex worldMesh[PlayerShip::m_vertexNum];
	float screenScale = SCREEN_SIZE_X / WORLD_SIZE_X;
	for (int i = 0; i < m_player->m_health; i++) {
		PlayerShip::GetLocalMesh(PlayerShip::m_vertexNum, worldMesh);
		TransformVertexArrayXY3D(15, worldMesh, screenScale, 90.f, 
			Vec2(screenScale * PLAYER_SHIP_COSMETIC_RADIUS * (2*i+1), 
				 SCREEN_SIZE_Y - screenScale * PLAYER_SHIP_COSMETIC_RADIUS));
		g_engine->m_renderer->DrawVertexArray(15, worldMesh);
	}
}

//--------------------------------------------------------------------------------------------------
void Game::DelayQuit(float delayTime) {
	if (!m_quitFlag) {
		m_delayTime = delayTime;
		m_quitFlag = true;
	}
}

//--------------------------------------------------------------------------------------------------
void Game::AddCameraShake(float amp) {
	m_curCameraShakeAmp = GetClamped(m_curCameraShakeAmp + amp, 0.f, m_maxCameraShakeAmp);
}

//--------------------------------------------------------------------------------------------------
void Game::DecayCameraShake(float deltaSeconds) {
	m_curCameraShakeAmp = GetClamped(m_curCameraShakeAmp - deltaSeconds * m_cameraShakeDecaySpeed, 0.f, m_maxCameraShakeAmp);
}

//--------------------------------------------------------------------------------------------------
void Game::SetNextGameState(GameState nextState) {
	m_nextGameState = nextState;
}

//--------------------------------------------------------------------------------------------------
GameState Game::GetCurGameState() {
	return m_curGameState;
}

//--------------------------------------------------------------------------------------------------
void Game::RenderAttractMode() const {
	g_engine->m_renderer->BeginCamera(*m_screenCamera);
	float titleFraction = m_titleAnimationTimeCount / m_titleAnimationTotalTime;
	float boidsFraction = m_boidsAnimationTimeCount / m_boidsAnimationTotalTime;

	//-----------------------------------------------------------------------------------------------
	Vertex worldMesh[WaspEnemy::m_vertexNum];
	for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i ++) {
		if (m_attractModeBoids[i].isActive) {
			if (m_attractModeBoids[i].boidsType == 0) {
				PlayerShip::GetLocalMesh(PlayerShip::m_vertexNum, worldMesh);
				TransformVertexArrayXY3D(PlayerShip::m_vertexNum, worldMesh,
					4.f, m_attractModeBoids[i].velocity.GetOrientationDegrees(),
					m_attractModeBoids[i].worldPos);
				g_engine->m_renderer->DrawVertexArray(PlayerShip::m_vertexNum, worldMesh);
			}
			else if (m_attractModeBoids[i].boidsType == 1) {
				BeetleEnemy::GetLocalMesh(BeetleEnemy::m_vertexNum, worldMesh);
				for (int j = BeetleEnemy::m_vertexNum - 12; j < BeetleEnemy::m_vertexNum - 7; j++) {
					worldMesh[j] = Interpolate(worldMesh[j], worldMesh[j + 6], SinDegrees(boidsFraction * 180.f));
				}
				TransformVertexArrayXY3D(BeetleEnemy::m_vertexNum - 6, worldMesh,
					4.f, m_attractModeBoids[i].velocity.GetOrientationDegrees(),
					m_attractModeBoids[i].worldPos);
				g_engine->m_renderer->DrawVertexArray(BeetleEnemy::m_vertexNum - 6, worldMesh);
			}
		}
	}

	//-----------------------------------------------------------------------------------------------
	std::vector<Vertex> textVertex;
	AddVertsForTextTriangles2D(textVertex, "GOLD",
		Vec2(SCREEN_CENTER_X + 300.f, SCREEN_SIZE_Y - 225.f),
		150.f * Interpolate(m_titleScaleMin, m_titleScaleMax, SinDegrees(titleFraction * 180.f)),
		Rgba8(255, 255, 0, 255), 0.5f);
	TransformVertexArrayXY3D((int)textVertex.size(), textVertex.data(), 1.f, 0.f,
		Vec2(0, Interpolate(-m_titleUpDownMax, m_titleUpDownMax, SinDegrees(titleFraction * 180.f)))
	);
	AddVertsForTextTriangles2D(textVertex, "STARTSHIP", Vec2(SCREEN_CENTER_X - 675.f, SCREEN_SIZE_Y - 225.f), 150.f, Rgba8(255, 255, 255, 255));
	g_engine->m_renderer->DrawVertexArray((int)textVertex.size(), textVertex.data());

	g_engine->m_renderer->EndCamera(*m_screenCamera);
}

//--------------------------------------------------------------------------------------------------
void Game::UpdateAllAttractModeBoids(float deltaSeconds) {
	for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i++) {
		if(m_attractModeBoids[i].isActive)
			UpdateOneAttractModeBoid(&m_attractModeBoids[i], deltaSeconds);
	}
	for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i++) {
		if (m_attractModeBoids[i].isActive) {
			m_attractModeBoids[i].worldPos = m_attractModeBoids[i].nextPosition;
			m_attractModeBoids[i].velocity = m_attractModeBoids[i].nextVelocity;
		}
	}
}

//--------------------------------------------------------------------------------------------------
void Game::UpdateOneAttractModeBoid(AttractModeBoidsEntity* boid, float deltaSeconds) {
	int neighborCount = 0;
	for (int i = 0; i < MAX_ATTRACTMODE_BOIDS; i++) {
		if (&m_attractModeBoids[i] != boid && m_attractModeBoids[i].isActive) {
			float distance = GetDistance2D(boid->worldPos, m_attractModeBoids[i].worldPos);
			if (distance < boid->nearbyRadius) {
				m_boidsNeighbors[neighborCount] = &m_attractModeBoids[i];
				neighborCount++;
			}
		}
	}


	Vec2 separation(0.f, 0.f);
	Vec2 alignment(0.f, 0.f);
	Vec2 cohesion(0.f, 0.f);

	if (neighborCount > 0) {
		// Separation
		for (int i = 0; i < neighborCount; i++) {
			Vec2 away = boid->worldPos - m_boidsNeighbors[i]->worldPos;
			float sqrDistance = away.GetLengthSquared();
			if (sqrDistance < boid->bounderyRadius * boid->bounderyRadius) {
				separation += away / sqrDistance;
			}
		}
		separation *= m_boidsSeprationWeight;

		// Alignment
		for (int i = 0; i < neighborCount; i++) {
			alignment += m_boidsNeighbors[i]->velocity;
		}
		alignment /= (float)neighborCount;
		alignment *= m_boidsAlignmentWeight;

		// Cohesion
		for (int i = 0; i < neighborCount; i++) {
			cohesion += m_boidsNeighbors[i]->worldPos;
		}
		cohesion /= (float)neighborCount;
		cohesion = (cohesion - boid->worldPos) * m_boidsCohesionWeight;
	}
	else {
		Vec2 screenCenter(SCREEN_CENTER_X, SCREEN_CENTER_Y);
		Vec2 toCenter = screenCenter - boid->worldPos;
		toCenter.Normalize();

		Vec2 centerForce = toCenter * m_centerPullWeight; 

		boid->nextVelocity = boid->velocity + centerForce * deltaSeconds;
		boid->nextVelocity = boid->nextVelocity.GetClamped(m_boidsMaxSpeed);

		boid->nextPosition = boid->worldPos + boid->nextVelocity * deltaSeconds;
		boid->nextPosition.x = GetClamped(boid->nextPosition.x, 0.f, SCREEN_SIZE_X);
		boid->nextPosition.y = GetClamped(boid->nextPosition.y, 0.f, SCREEN_SIZE_Y);
		return;
	}


	Vec2 boundaryForce(0.f, 0.f);

	if (boid->worldPos.x < m_boundaryMargin) {
		boundaryForce.x += (m_boundaryMargin - boid->worldPos.x);
	}
	else if (boid->worldPos.x > SCREEN_SIZE_X - m_boundaryMargin) {
		boundaryForce.x -= (boid->worldPos.x - (SCREEN_SIZE_X - m_boundaryMargin));
	}
	if (boid->worldPos.y < m_boundaryMargin) {
		boundaryForce.y += (m_boundaryMargin - boid->worldPos.y);
	}
	else if (boid->worldPos.y > SCREEN_SIZE_Y - m_boundaryMargin) {
		boundaryForce.y -= (boid->worldPos.y - (SCREEN_SIZE_Y - m_boundaryMargin));
	}
	boundaryForce *= m_boundaryWeight;

	Vec2 screenCenter(SCREEN_CENTER_X, SCREEN_CENTER_Y);
	Vec2 toCenter = screenCenter - boid->worldPos;
	toCenter.Normalize();
	Vec2 centerForce = toCenter * m_centerPullWeight;

	boid->nextVelocity = boid->velocity
		+ (separation + alignment + cohesion + boundaryForce + centerForce) * deltaSeconds;

	boid->nextVelocity = boid->nextVelocity.GetClamped(m_boidsMaxSpeed);
	boid->nextPosition = boid->worldPos + boid->nextVelocity * deltaSeconds;

	boid->nextPosition.x = GetClamped(boid->nextPosition.x, 0.f, SCREEN_SIZE_X);
	boid->nextPosition.y = GetClamped(boid->nextPosition.y, 0.f, SCREEN_SIZE_Y);
}