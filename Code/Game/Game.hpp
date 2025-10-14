#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"

class PlayerShip;
class Asteroid;
class Bullet;
class BeetleEnemy;
class WaspEnemy;
class Debris;
class ShockWave;
class Camera;
class RandomNumberGenerator;

//-----------------------------------------------------------------------------------------------
enum GameState {
	GAME_ATTRACT_MODE,
	GAME_PLAYING_MODE,
	GAME_PLAYER_UPGRADE_MODE,
};

enum PlayerHpUpgrade
{
	NONE_HP_UPGRADE,
	HP_MAX
};

enum PlayerShieldUpgrade {
	NONE_SHIELD_UPGRADE,
	SHIELD_MAX,
	SHIELD_RECOVER,
	SHIELD_EXPLOSION_RANGE
};

enum PlayerBulletUpgrade {
	NONE_BULLET_UPGRADE,
	BULLET_INTERVAL,
	BULLET_BRANCH,
	BULLET_EXPLOSION_RANGE,
	BULLET_TRACK
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
	int neighborCount = 0;
	bool isActive = false;
	Vec2 worldPos;
	Vec2 velocity;
	float bounderyRadius = 30.f;
	float nearbyRadius = 40.f;

	Vec2 nextPosition;
	Vec2 nextVelocity;
};

struct PlayerUpgradeItem
{
	PlayerHpUpgrade hpUpgrade;
	PlayerShieldUpgrade shieldUpgrade;
	PlayerBulletUpgrade bulletUpgrade;

	const char* upgradeDescribe;
	float upgradeVal;
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
	void GetNextUpgradeChoose();
	void GetPreviousUpgradeChoose();
	PlayerUpgradeItem GetChoseUpgrade();

public:
	PlayerShip* m_player = nullptr;
	Asteroid* m_asteroids[MAX_ASTEROIDS];
	Bullet* m_bullets[MAX_BULLETS];
	BeetleEnemy* m_beetleEnemy[MAX_BEETLES];
	WaspEnemy* m_waspEnemy[MAX_WASPS];
	Debris* m_debris[MAX_DEBRIS];
	ShockWave* m_shockWaves[MAX_SHOCKWAVE];
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
	void UpdateCameras(float deltaSeconds);
	void RenderPoolEntitys() const;
	void RenderDebugThings() const;
	void RenderUI() const;
	void RenderUIHealthBar() const;
	void RenderUIShieldBar() const;
	void RenderUIHealth() const;
	void RenderUIMiniMap() const;
	void RenderUIWaves() const;
	void RenderUIExpBar() const;
	void RenderUIUpgrade() const;
	void RenderUIUpgradeItem(Vec2 center, PlayerUpgradeItem content, int index) const;
	void RenderAttractMode() const;
	void GenerateStarsMesh(int starCount, Vertex* startsMesh, 
							Vec2 buttomLeft, Vec2 topRight, float starSizeMin, float starSizeMax,
							Rgba8 starColor1, Rgba8 starColor2);
	void RenderStars() const;
	void RenderWorldBoundary() const;
	void SetUpLevel(int levelIndex);
	void CheckGotoNextLevel();
	void DecayCameraShake(float deltaSeconds);
	void UpdateAllAttractModeBoids(float deltaSeconds);
	void UpdateOneAttractModeBoid(AttractModeBoidsEntity* boid, float deltaSeconds);

private:
	GameState m_curGameState = GAME_ATTRACT_MODE;
	GameState m_nextGameState = GAME_ATTRACT_MODE;
	GameLevel levels[5] = { {20, 200 ,0 }, {40,300,100}, {80, 600, 200}, {160, 800, 400}, {320, 1000, 1000} };
	int m_curLevelIndex = 0;

	float m_gameRunTime = 0.f;
	float m_deltaSeconds = 0.f;

	float m_titleAnimationTotalTime = 2.f;
	float m_titleAnimationTimeCount = 0.f;
	float m_titleScaleMax = 1.05f;
	float m_titleScaleMin = 0.95f;
	float m_titleUpDownMax = 0.5f;

	float m_waveAnimationTotalTime = 2.f;
	float m_waveAnimationTimeCount = 0.f;

	bool m_gameModeStartFlag = true;
	int m_miniMapGridNum = 16;
	float m_miniMapReduceRatio = 0.2f;
	Vec2 m_miniMapCenter = Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) + Vec2(-110.f, -110.f);
	Vec2 m_miniMapOrigin = m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio;
	float m_miniMapFrameThickness = 5.f;
	Rgba8 m_miniMapFrameColor = Rgba8(200, 200, 200, 255);
	float m_miniMapGridLineThickness = 1.f;
	Rgba8 m_miniMapGridLineColor = Rgba8(0, 255, 0, 64);
	Rgba8 m_miniMapViewFieldColor = Rgba8(255, 0, 0, 64);
	Rgba8 m_miniMapPlayerOuterColor = Rgba8(102, 153, 204, 128);
	Rgba8 m_miniMapPlayerInnerColor = Rgba8(102, 153, 204, 255);
	Rgba8 m_miniMapAsteroidOuterColor = Rgba8(100, 100, 100, 128);
	Rgba8 m_miniMapAsteroidInnerColor = Rgba8(100, 100, 100, 255);
	Rgba8 m_miniMapBeetleOuterColor = Rgba8(161, 67, 5, 128);
	Rgba8 m_miniMapBeetleInnerColor = Rgba8(161, 67, 5, 255);
	Rgba8 m_miniMapWaspOuterColor = Rgba8(255, 222, 0, 128);
	Rgba8 m_miniMapWaspInnerColor = Rgba8(255, 222, 0, 255);
	float m_miniMapTokenSize = 2.f;
	Rgba8 m_miniMapPlayerVelocityColor = Rgba8(255,255,0,128);

	bool m_playerUpgradeStartFlag = true;
	int m_upgradeChoose = 0;
	int m_upgradeChooseMax = 3;
	PlayerUpgradeItem m_playerUpgradeItems[8] = {
		{
			HP_MAX,NONE_SHIELD_UPGRADE,NONE_BULLET_UPGRADE,
			"Max Health+",
			25.f
		},
		{
			NONE_HP_UPGRADE,SHIELD_MAX,NONE_BULLET_UPGRADE,
			"Max Shield+",
			20.f
		},
		{
			NONE_HP_UPGRADE,SHIELD_RECOVER,NONE_BULLET_UPGRADE,
			"Shield Recover+",
			1.f
		},
		{
			NONE_HP_UPGRADE,SHIELD_EXPLOSION_RANGE,NONE_BULLET_UPGRADE,
			"Shield Explosion+",
			5.5f
		},
		{
			NONE_HP_UPGRADE,NONE_SHIELD_UPGRADE, BULLET_INTERVAL,
			"Fire Interval-",
			0.05f
		},
		{
			NONE_HP_UPGRADE,NONE_SHIELD_UPGRADE, BULLET_BRANCH,
			"Fire Branch+",
			1.f
		},
		{
			NONE_HP_UPGRADE,NONE_SHIELD_UPGRADE, BULLET_EXPLOSION_RANGE,
			"Bullet Explosion+",
			5.f
		},
		{
			NONE_HP_UPGRADE,NONE_SHIELD_UPGRADE, BULLET_TRACK,
			"Track Timer+",
			0.25f
		},
	};
	int m_upgradeChooseHealth = 0;
	int m_upgradeChooseShield = 0;
	int m_upgradeChooseBullet = 0;
	float m_upgradeItemWidth = 200.f;
	float m_upgradeItemHeight = 400.f;
	int m_upgradeGridNum = 16;
	float m_upgradeGridLineThickness = 1.f;
	Rgba8 m_upgradeGridLineColor = Rgba8(0, 255, 0, 64);

	bool m_attractModeStartFlag = true;
	AttractModeBoidsEntity m_attractModeBoids[MAX_ATTRACTMODE_BOIDS];
	AttractModeBoidsEntity m_boidsNeighbors[MAX_ATTRACTMODE_BOIDS];
	AttractModeBoidsEntity m_boidPlayerNeighbors[MAX_ATTRACTMODE_BOIDS];
	float m_boidPlayerSeprationWeight = 10000.f;
	float m_boidsSeprationWeight = 300.f;
	float m_boidsCohesionWeight = 2.f;
	float m_boidsAlignmentWeight = 0.8f;
	float m_boidBeetleMaxSpeed = 100.f;
	float m_boidWaspMaxSpeed = 300.f;
	float m_boidPlayerMaxSpeed = 500.f;
	float m_boidCenterPullWeight = 50.f;
	float m_boidPlayerPullWeight = 50.f;
	float m_boidsAnimationTimeCount = 0.f;
	float m_boidsAnimationTotalTime = 0.2f;
	float m_boidsPlayerAnimationTimeCount = 0.f;
	float m_boidsPlayerAnimationTotalTime = 1.f;

	Vertex* m_attractModeStarsMesh = nullptr;
	Vertex* m_nearStarsMesh = nullptr;
	Vertex* m_farStarsMesh = nullptr;
	float m_starFieldExpandDistance = 70.f;

	float m_curCameraShakeAmp = 0.f;
	float m_maxCameraShakeAmp = 10.f;
	float m_cameraShakeDecaySpeed = 5.f;
	float m_playerFireScreenShakeAmp = 0.5f;
	float m_playSoundSpeed = 1.f;
	float m_playSoundSwitchSpeed = 2.f;

	float m_waitedTime = 0;
	float m_delayTime = 1.f;
	bool m_quitFlag = false;
};