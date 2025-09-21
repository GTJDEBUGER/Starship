#pragma once
#include "Game/GameCommon.hpp"

class PlayerShip;
class Asteroid;
class Bullet;
class BeetleEnemy;
class WaspEnemy;
class Debris;

//-----------------------------------------------------------------------------------------------
struct GameLevel {
	int asteroidNum = 0;
	int beetleEnemy = 0;
	int waspEnemy = 0;
};

//-----------------------------------------------------------------------------------------------
class Game {
public:
	Game();
	~Game();
	void Update(float deltaSeconds);
	void Render() const;
	void DelayQuit(float delayTime);

public:
	PlayerShip* m_player = nullptr;
	Asteroid* m_asteroids[MAX_ASTEROIDS];
	Bullet* m_bullets[MAX_BULLETS];
	BeetleEnemy* m_beetleEnemy[MAX_BEETLES];
	WaspEnemy* m_waspEnemy[MAX_WASPS];
	Debris* m_debris[MAX_DEBRIS];

private:
	void InitialObjectPools();
	void DeleteObjectPools();
	void UpdatePoolEntitys(float deltaSeconds);
	void HandleFiringInput();
	void HandleRespawnAsteroidInput();
	void HandleRespawnPlayerInput();
	void RenderPoolEntitys() const;
	void RenderDebugThings() const;
	void RenderUI() const;
	void SetUpLevel(int levelIndex);
	void CheckGotoNextLevel();

private:
	GameLevel levels[5] = { {4,1,0 }, {6,3,1}, {8, 5, 2}, {10, 7, 3}, {12, 9, 4} };
	int m_curLevelIndex = 0;
	float m_waitedTime = 0;
	float m_delayTime;
	bool m_quitFlag = false;
};