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
	if (m_player != nullptr) {
		m_player->Update(deltaSeconds);
		if(m_player->m_isDead) {
			delete m_player;
			m_player = new PlayerShip(this);
		}
	}

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] != nullptr) {
			m_asteroids[i]->Update(deltaSeconds);
			if (m_asteroids[i]->m_isDead) {
				delete m_asteroids[i];
				m_asteroids[i] = new Asteroid(this);
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
	if (g_app->m_isFiring) {
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
			// Error report
		}

		g_app->m_isFiring = false;
	}
}

//-----------------------------------------------------------------------------------------------
void Game::Render()
{
	m_player->Render();
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
		if (m_player != nullptr)
		{
			g_engine->m_renderer->DrawCircle(m_player->m_position, PLAYER_SHIP_COSMETIC_RADIUS, Rgba8(255, 0, 255, 255));
			g_engine->m_renderer->DrawCircle(m_player->m_position, PLAYER_SHIP_PHYSICS_RADIUS, Rgba8(0, 255, 255, 255));
			g_engine->m_renderer->DrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector() * 4.f, Rgba8(255, 0, 0, 255));
			g_engine->m_renderer->DrawLine(m_player->m_position, m_player->m_position + m_player->GetForwardVector().GetRotatedBy90Degrees() * 4.f, Rgba8(0, 255, 0, 255));
			g_engine->m_renderer->DrawLine(m_player->m_position, m_player->m_position + m_player->m_velocity, Rgba8(255, 255, 0, 255));
		}

		for(int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (m_asteroids[i] != nullptr) {
				g_engine->m_renderer->DrawCircle(m_asteroids[i]->m_position, ASTEROID_COSMETIC_RADIUS, Rgba8(255, 0, 255, 255));
				g_engine->m_renderer->DrawCircle(m_asteroids[i]->m_position, ASTEROID_PHYSICS_RADIUS, Rgba8(0, 255, 255, 255));
				g_engine->m_renderer->DrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + Vec2(CosDegrees(m_asteroids[i]->m_orientationDegrees), SinDegrees(m_asteroids[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255));
				g_engine->m_renderer->DrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + Vec2(-SinDegrees(m_asteroids[i]->m_orientationDegrees), CosDegrees(m_asteroids[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255));
				g_engine->m_renderer->DrawLine(m_asteroids[i]->m_position, m_asteroids[i]->m_position + m_asteroids[i]->m_velocity, Rgba8(255, 255, 0, 255));

				if (m_player != nullptr) {
					g_engine->m_renderer->DrawLine(m_asteroids[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255));
				}

			}
		}

		for (int i = 0; i < MAX_BULLETS; i++) {
			if (m_bullets[i] != nullptr) {
				g_engine->m_renderer->DrawCircle(m_bullets[i]->m_position, ASTEROID_COSMETIC_RADIUS, Rgba8(255, 0, 255, 255));
				g_engine->m_renderer->DrawCircle(m_bullets[i]->m_position, ASTEROID_PHYSICS_RADIUS, Rgba8(0, 255, 255, 255));
				g_engine->m_renderer->DrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + Vec2(CosDegrees(m_bullets[i]->m_orientationDegrees), SinDegrees(m_bullets[i]->m_orientationDegrees)) * 4.f, Rgba8(255, 0, 0, 255));
				g_engine->m_renderer->DrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + Vec2(-SinDegrees(m_bullets[i]->m_orientationDegrees), CosDegrees(m_bullets[i]->m_orientationDegrees)) * 4.f, Rgba8(0, 255, 0, 255));
				g_engine->m_renderer->DrawLine(m_bullets[i]->m_position, m_bullets[i]->m_position + m_bullets[i]->m_velocity, Rgba8(255, 255, 0, 255));

				if (m_player != nullptr) {
					g_engine->m_renderer->DrawLine(m_bullets[i]->m_position, m_player->m_position, Rgba8(50, 50, 50, 255));
				}
			}
		}
	}
}