#pragma once
#include "Game/GameCommon.hpp"

class PlayerShip;
class Asteroid;
class Bullet;
class BeetleEnemy;

//-----------------------------------------------------------------------------------------------
class Game {
public:
	Game();
	~Game();
	void Update(float deltaSeconds);
	void Render() const;

public:
	PlayerShip* m_player = nullptr;
	Asteroid* m_asteroids[MAX_ASTEROIDS];
	Bullet* m_bullets[MAX_BULLETS];
	BeetleEnemy* m_beetleEnemy[MAX_BEETLES];
};