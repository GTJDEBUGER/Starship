#include "Game.hpp"
#include "GameCommon.hpp"
#include "PlayerShip.hpp"
#include "Asteroid.hpp"
#include "Bullet.hpp"
#include "App.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
Game::Game()
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		m_asteroids[i] = nullptr;
	}
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		m_bullets[i] = nullptr;
	}

	for (int i = 0; i < 6; i++) {
		m_asteroids[i] = new Asteroid(this);
	}
	m_player = new PlayerShip(this);
}

//-----------------------------------------------------------------------------------------------
Game::~Game()
{
	delete m_player;
	m_player = nullptr;
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
}

//-----------------------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	if (m_player != nullptr && !m_player->m_isDead) {
		m_player->Update(deltaSeconds);
	}

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

	//-----------------------------------------------------------------------------------------------
	if (g_app->m_isFiring && !m_player->m_isDead) {
		int freeBulletIndex = -1;
		for (int i = 0; i < MAX_BULLETS; i++) {
			if (m_bullets[i] == nullptr) {
				freeBulletIndex = i;
				break;
			}
		}

		if (freeBulletIndex > -1) {
			m_bullets[freeBulletIndex] = new Bullet(this, m_player->m_position + m_player->GetForwardVector() *2, 
														  m_player->GetForwardVector());
		}
		else {
			if (IsDebuggerAvailable()) {
				SystemDialogue_Okay("Run out bullets!", "Maximum of 20 Bullets alive at once.", MsgSeverityLevel::FATAL);
			}
		}

		g_app->m_isFiring = false;
	}
	else if(g_app->m_isFiring) {
		g_app->m_isFiring = false;
	}

	//-----------------------------------------------------------------------------------------------
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
			if (IsDebuggerAvailable()) {
				SystemDialogue_Okay("Run out asteroids!", "Maximum of 12 asteroids alive at once.", MsgSeverityLevel::FATAL);
			}
		}

		g_app->m_isAsteroidRespawn = false;
	}

	//-----------------------------------------------------------------------------------------------
	if (g_app->m_isPlayerRespawn && m_player->m_isDead) {
		m_player->m_position = Vec2(WORLD_CENTER_X, WORLD_CENTER_Y);
		m_player->m_velocity = Vec2(0, 0);
		m_player->m_orientationDegrees = 0.f;
		m_player->m_isDead = false;

		g_app->m_isPlayerRespawn = false;
	}
	else if(g_app->m_isPlayerRespawn) {
		g_app->m_isPlayerRespawn = false;
	}
}

//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	if (!m_player->m_isDead) {
		m_player->Render();
	}

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

	// Debug Draw
	if (g_app->m_isDebugDraw) {
		if (m_player != nullptr && !m_player->m_isDead)
		{
			DebugDrawRing(m_player->m_position, PLAYER_SHIP_COSMETIC_RADIUS, Rgba8(255, 0, 255, 255));
			DebugDrawRing(m_player->m_position, PLAYER_SHIP_PHYSICS_RADIUS, Rgba8(0, 255, 255, 255));
			DebugDrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector() * 4.f, Rgba8(255, 0, 0, 255));
			DebugDrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector().GetRotatedBy90Degrees() * 4.f, Rgba8(0, 255, 0, 255));
			DebugDrawLine(m_player->m_position, m_player->m_position + m_player->m_velocity, Rgba8(255, 255, 0, 255));
		}

		for(int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (m_asteroids[i] != nullptr) {
				DebugDrawRing(m_asteroids[i]->m_position, ASTEROID_COSMETIC_RADIUS, Rgba8(255, 0, 255, 255));
				DebugDrawRing(m_asteroids[i]->m_position, ASTEROID_PHYSICS_RADIUS, Rgba8(0, 255, 255, 255));
				DebugDrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + Vec2(CosDegrees(m_asteroids[i]->m_orientationDegrees), SinDegrees(m_asteroids[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255));
				DebugDrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + Vec2(-SinDegrees(m_asteroids[i]->m_orientationDegrees), CosDegrees(m_asteroids[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255));
				DebugDrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + m_asteroids[i]->m_velocity, Rgba8(255, 255, 0, 255));

				if (m_player != nullptr && !m_player->m_isDead) {
					DebugDrawLine(m_asteroids[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255));
				}

			}
		}

		for (int i = 0; i < MAX_BULLETS; i++) {
			if (m_bullets[i] != nullptr) {
				DebugDrawRing(m_bullets[i]->m_position, ASTEROID_COSMETIC_RADIUS, Rgba8(255, 0, 255, 255));
				DebugDrawRing(m_bullets[i]->m_position, ASTEROID_PHYSICS_RADIUS, Rgba8(0, 255, 255, 255));
				DebugDrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + Vec2(CosDegrees(m_bullets[i]->m_orientationDegrees), SinDegrees(m_bullets[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255));
				DebugDrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + Vec2(-SinDegrees(m_bullets[i]->m_orientationDegrees), CosDegrees(m_bullets[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255));
				DebugDrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + m_bullets[i]->m_velocity, Rgba8(255, 255, 0, 255));

				if (m_player != nullptr && !m_player->m_isDead) {
					DebugDrawLine(m_bullets[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255));
				}
			}
		}
	}
}