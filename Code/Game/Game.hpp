#pragma once
#include "Game/GameCommon.hpp"

class PlayerShip;
class Asteroid;
class Bullet;
class BeetleEnemy;
class WaspEnemy;
class Debris;
class Camera;
class RandomNumberGenerator;

//-----------------------------------------------------------------------------------------------
enum GameState {
	GAME_ATTRACT_MODE,
	GAME_PLAYING
};

//-----------------------------------------------------------------------------------------------
struct GameLevel {
	int asteroidNum = 0;
	int beetleEnemy = 0;
	int waspEnemy = 0;
};

struct AttractModeBoidsEntity
{
	int boidsType = 0; //0 for player, 1 for beetle, 2 for wasp
	bool isActive = false;
	Vec2 worldPos;
	Vec2 velocity;
	float bounderyRadius = 40.f;
	float nearbyRadius = 20.f;

	Vec2 nextPosition;
	Vec2 nextVelocity;
};

//-----------------------------------------------------------------------------------------------
class Game {
public:
	Game();
	~Game();
	void Update(float deltaSeconds);
	void Render() const;
	void DelayQuit(float delayTime);
	void AddCameraShake(float amp);
	void SetNextGameState(GameState nextState);
	GameState GetCurGameState();

public:
	PlayerShip* m_player = nullptr;
	Asteroid* m_asteroids[MAX_ASTEROIDS];
	Bullet* m_bullets[MAX_BULLETS];
	BeetleEnemy* m_beetleEnemy[MAX_BEETLES];
	WaspEnemy* m_waspEnemy[MAX_WASPS];
	Debris* m_debris[MAX_DEBRIS];
	Camera* m_worldCamera = nullptr;
	Camera* m_screenCamera = nullptr;
	RandomNumberGenerator* m_randomGenerator = nullptr;

private:
	void InitialObjectPools();
	void DeleteObjectPools();
	void UpdateEntities(float deltaSeconds);
	void HandleEntitiesCollide();
	void HandleFiringInput();
	void HandleRespawnAsteroidInput();
	void HandleRespawnPlayerInput();
	void RenderPoolEntitys() const;
	void RenderDebugThings() const;
	void RenderUI() const;
	void RenderAttractMode() const;
	void SetUpLevel(int levelIndex);
	void CheckGotoNextLevel();
	void DecayCameraShake(float deltaSeconds);
	void UpdateAllAttractModeBoids(float deltaSeconds);
	void UpdateOneAttractModeBoid(AttractModeBoidsEntity* boid, float deltaSeconds);

private:
	GameState m_curGameState = GAME_ATTRACT_MODE;
	GameState m_nextGameState = GAME_ATTRACT_MODE;
	GameLevel levels[5] = { {4,1,0 }, {6,3,1}, {8, 5, 2}, {10, 7, 3}, {12, 9, 4} };
	int m_curLevelIndex = 0;
	
	float m_startButtonAnimationTotalTime = 1.f;
	float m_startButtonAnimationTimeCount = 0.f;

	float m_titleAnimationTotalTime = 2.f;
	float m_titleAnimationTimeCount = 0.f;
	float m_titleScaleMax = 1.05f;
	float m_titleScaleMin = 0.95f;
	float m_titleUpDownMax = 0.5f;

	AttractModeBoidsEntity* m_attractModeBoids = nullptr;
	AttractModeBoidsEntity* m_boidsNeighbors[MAX_ATTRACTMODE_BOIDS];
	float m_playerSeprationWeight = 1000.f;
	float m_boidsSeprationWeight = 15.f;
	float m_boidsCohesionWeight = 0.8f;
	float m_boidsAlignmentWeight = 0.6f;
	float m_boidsMaxSpeed = 50.f;
	float m_boundaryMargin = 20.f;
	float m_boundaryWeight = 100.f;
	float m_centerPullWeight = 5.f;
	float m_playerPullWeight = 5.f;
	float m_boidsAnimationTimeCount = 0.f;
	float m_boidsAnimationTotalTime = 0.2f;
	
	float m_waitedTime = 0;
	float m_delayTime = 1.f;
	bool m_quitFlag = false;
	float m_curCameraShakeAmp = 0.f;
	float m_maxCameraShakeAmp = 10.f;
	float m_cameraShakeDecaySpeed = 5.f;
	float m_playerFireScreenShakeAmp = 0.5f;
};