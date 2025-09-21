#include <direct.h> 

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

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
}

//-----------------------------------------------------------------------------------------------
Game::~Game()
{
	delete m_player;
	m_player = nullptr;
	DeleteObjectPools();
}

//-----------------------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	//-----------------------------------------------------------------------------------------------
	if (m_quitFlag) {
		m_waitedTime += deltaSeconds;
		if (m_waitedTime > m_delayTime) {
			g_app->m_isAttractMode = true;
		}
	}
	//-----------------------------------------------------------------------------------------------
	if (m_player != nullptr && !m_player->m_isDead) {
		m_player->Update(deltaSeconds);
	}
	UpdatePoolEntitys(deltaSeconds);
	//-----------------------------------------------------------------------------------------------
	HandleFiringInput();
	//-----------------------------------------------------------------------------------------------
	HandleRespawnAsteroidInput();
	//-----------------------------------------------------------------------------------------------
	HandleRespawnPlayerInput();
	//-----------------------------------------------------------------------------------------------
	CheckGotoNextLevel();
}

//---------------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_engine->m_renderer->BeginCamera(*g_app->m_gameCamera);

	RenderPoolEntitys();

	if (!m_player->m_isDead) {
		m_player->Render();
	}

	if (g_app->m_isDebugDraw) {
		RenderDebugThings();
	}

	RenderUI();

	g_engine->m_renderer->EndCamera(*g_app->m_gameCamera);
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
void Game::UpdatePoolEntitys(float deltaSeconds) {
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
	float drawThickness = .2f;

	DebugDrawRing(m_player->m_position, m_player->m_cosmeticRadius, Rgba8(255, 0, 255, 255), drawThickness);
	DebugDrawRing(m_player->m_position, m_player->m_physicsRadius, Rgba8(0, 255, 255, 255), drawThickness);
	DebugDrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector() * 4.f, Rgba8(255, 0, 0, 255), drawThickness);
	DebugDrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector().GetRotatedBy90Degrees() * 4.f, Rgba8(0, 255, 0, 255), drawThickness);
	DebugDrawLine(m_player->m_position, m_player->m_position + m_player->m_velocity, Rgba8(255, 255, 0, 255), drawThickness);

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

//---------------------------------------------------------------------------------------------------
void Game::SetUpLevel(int levelIndex) {
	for (int i = 0; i < levels[levelIndex].asteroidNum; i++) {
		m_asteroids[i] = new Asteroid(this);
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
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr) {
			flag = false;
			break;
		}
	}
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
	for (int i = 0; i < m_player->m_health; i++) {
		PlayerShip::GetLocalMesh(PlayerShip::m_vertexNum, worldMesh);
		TransformVertexArrayXY3D(15, worldMesh, 1.f, 90.f, Vec2(PLAYER_SHIP_COSMETIC_RADIUS * (2*i+1), WORLD_SIZE_Y - PLAYER_SHIP_COSMETIC_RADIUS));
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