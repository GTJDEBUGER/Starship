#include "Game.hpp"
#include "GameCommon.hpp"
#include "PlayerShip.hpp"
#include "Asteroid.hpp"
#include "Bullet.hpp"

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
	m_player->Update(deltaSeconds);
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
}