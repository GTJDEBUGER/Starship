#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"

class PlayerShip;
class Asteroid;
class Bullet;
class BeetleEnemy;
class WaspEnemy;
class Debris;
class ShockWave;
class Camera;
class RandomNumberGenerator;
class Clock;

//-----------------------------------------------------------------------------------------------
enum GameState {
	GAME_MODE_NONE = -1,
	GAME_ATTRACT_MODE,
	GAME_PLAYING_MODE,
	GAME_PLAYER_UPGRADE_MODE,
};

enum PlayerHealthUpgrade
{
	NONE_HEALTH_UPGRADE = -1,
	HEALTH_MAX_UPGRADE
};

enum PlayerShieldUpgrade {
	NONE_SHIELD_UPGRADE = -1,
	SHIELD_MAX_UPGRADE,
	SHIELD_RECOVER_UPGREADE,
	SHIELD_EXPLOSION_RANGE_UPGRADE
};

enum PlayerBulletUpgrade {
	NONE_BULLET_UPGRADE = -1,
	BULLET_INTERVAL_UPGRADE,
	BULLET_BRANCH_UPGRADE,
	BULLET_EXPLOSION_RANGE_UPGRADE,
	BULLET_TRACK_UPGRADE
};

//-----------------------------------------------------------------------------------------------
struct GameLevel {
	int asteroidNum = 0;
	int beetleEnemy = 0;
	int waspEnemy   = 0;
};

struct AttractModeBoidsEntity
{
	bool  isActive         = false;
	int   boidsType        = 0; //0 for player, 1 for beetle, 2 for wasp
	int   neighborCount    = 0;
	float bounderyRadius   = 30.f;
	float nearbyRadius     = 40.f;
	Vec2  position;
	Vec2  velocity;
	Vec2  nextPosition;
	Vec2  nextVelocity;
};

struct PlayerUpgradeItem
{
	PlayerHealthUpgrade hpUpgrade;
	PlayerShieldUpgrade shieldUpgrade;
	PlayerBulletUpgrade bulletUpgrade;
	const char*         upgradeDescribe;
	float               upgradeVal;
};

//-----------------------------------------------------------------------------------------------
class Game {
public:
	Game();
	~Game();
	void                    Update();
	void	                Render() const;
	void	                DelayQuit(float delayTime);
	void	                AddCameraShake(float amp);
	void	                SetNextGameState(GameState nextState);
	void	                UpdateToNextUpgradeChoose();
	void	                UpdateToPreviousUpgradeChoose();
	GameState const         GetCurGameState() const;
	PlayerUpgradeItem const GetChoseUpgrade() const;
	void                    KillAllEntities();
	void                    RollUpgrades();

public:
	PlayerShip*             m_player                     = nullptr;
	Camera*                 m_worldCamera                = nullptr;
	Camera*                 m_viewCamera                 = nullptr;
	RandomNumberGenerator*  m_randomGenerator            = nullptr;
	Asteroid*               m_asteroids[MAX_ASTEROIDS];
	Bullet*                 m_bullets[MAX_BULLETS];
	BeetleEnemy*            m_beetleEnemy[MAX_BEETLES];
	WaspEnemy*              m_waspEnemy[MAX_WASPS];
	Debris*                 m_debris[MAX_DEBRIS];
	ShockWave*              m_shockWaves[MAX_SHOCKWAVE];
	Clock*                  m_gameClock                  = nullptr;

private:
	void                    GenerateStarsMesh(int starCount, Vertex* startsMesh,
		                     Vec2 buttomLeft, Vec2 topRight, float starSizeMin, float starSizeMax,
		                     Rgba8 starColor1, Rgba8 starColor2);
	void                    InitialObjectPools();
	void                    DeleteObjectPools();
	void                    HandleEntitiesCollide();
	void                    HandleFiringInput();
	void                    HandleRespawnAsteroidInput();
	void                    HandleRespawnPlayerInput();
	void                    LoadNextWave(int levelIndex);
	void                    CheckWaveProgress();
	void                    DecayCameraShake();
	void                    UpdateEntities();
	void                    UpdateCameras();
	void                    UpdateAllAttractModeBoids();
	void                    UpdateOneAttractModeBoid(AttractModeBoidsEntity* boid);
	void                    RenderPoolEntitys() const;
	void                    RenderDebugThings() const;
	void                    RenderUI() const;
	void                    RenderUIHealthBar() const;
	void                    RenderUIShieldBar() const;
	void                    RenderUIHealth() const;
	void                    RenderUIMiniMap() const;
	void                    RenderUIWaves() const;
	void                    RenderUIExpBar() const;
	void                    RenderUIUpgrade() const;
	void                    RenderUIUpgradeItem(Vec2 center, PlayerUpgradeItem content, int index) const;
	void                    RenderAttractMode() const;
	void                    RenderStars() const;
	void                    RenderWorldBoundary() const;

private:
	GameState               m_curGameState                  = GAME_ATTRACT_MODE;
	GameState               m_nextGameState                 = GAME_ATTRACT_MODE;
	int                     m_curLevelIndex                 = 0;
	GameLevel               m_levels[5]                     = { 
				             		   	                      {20, 200, 0 },   //Wave 1
				             		            	          {40, 300, 100},  //Wave 2
				             		            	          {80, 600, 200},  //Wave 3
				             		            	          {160, 800, 400}, //Wave 4
				             		            	          {320, 1000, 1000}//Wave 5 
				             		                   }   ;
						    
	bool                    m_gameModeStartFlag             = true;		               						   		   
	float                   m_waveAnimationTotalTime        = 2.f;
	float                   m_waveAnimationTimeCount        = 0.f;		             						   	   
	int                     m_miniMapGridNum                = 16;
	float                   m_miniMapReduceRatio            = 0.2f;
	Vec2                    m_miniMapCenter                 = Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) + Vec2(-110.f, -110.f);
	Vec2                    m_miniMapOrigin                 = m_miniMapCenter + Vec2(-WORLD_CENTER_X, -WORLD_CENTER_Y) * m_miniMapReduceRatio;
	float                   m_miniMapFrameThickness         = 5.f;
	Rgba8                   m_miniMapFrameColor             = Rgba8(200, 200, 200, 255);
	float                   m_miniMapGridLineThickness      = 1.f;
	Rgba8                   m_miniMapGridLineColor          = Rgba8(0, 255, 0, 64);
	Rgba8                   m_miniMapViewFieldColor         = Rgba8(255, 0, 0, 64);
	Rgba8                   m_miniMapPlayerOuterColor       = Rgba8(102, 153, 204, 128);
	Rgba8                   m_miniMapPlayerInnerColor       = Rgba8(102, 153, 204, 255);
	Rgba8                   m_miniMapAsteroidOuterColor     = Rgba8(100, 100, 100, 128);
	Rgba8                   m_miniMapAsteroidInnerColor     = Rgba8(100, 100, 100, 255);
	Rgba8                   m_miniMapBeetleOuterColor       = Rgba8(161, 67, 5, 128);
	Rgba8                   m_miniMapBeetleInnerColor       = Rgba8(161, 67, 5, 255);
	Rgba8                   m_miniMapWaspOuterColor         = Rgba8(255, 222, 0, 128);
	Rgba8                   m_miniMapWaspInnerColor         = Rgba8(255, 222, 0, 255);
	float                   m_miniMapTokenSize              = 2.f;
	Rgba8                   m_miniMapPlayerVelocityColor    = Rgba8(255,255,0,128);
			                								   
	bool                    m_playerUpgradeStartFlag        = true;
	int                     m_upgradeChoose                 = 0;
	int                     m_upgradeChooseMax              = 3;
	PlayerUpgradeItem       m_playerUpgradeItems[8]         = {
		{
			HEALTH_MAX_UPGRADE,NONE_SHIELD_UPGRADE,NONE_BULLET_UPGRADE,
			"Max Health+",
			25.f
		},
		{
			NONE_HEALTH_UPGRADE,SHIELD_MAX_UPGRADE,NONE_BULLET_UPGRADE,
			"Max Shield+",
			20.f
		},
		{
			NONE_HEALTH_UPGRADE,SHIELD_RECOVER_UPGREADE,NONE_BULLET_UPGRADE,
			"Shield Recover+",
			1.f
		},
		{
			NONE_HEALTH_UPGRADE,SHIELD_EXPLOSION_RANGE_UPGRADE,NONE_BULLET_UPGRADE,
			"Shield Explosion+",
			5.5f
		},
		{
			NONE_HEALTH_UPGRADE,NONE_SHIELD_UPGRADE, BULLET_INTERVAL_UPGRADE,
			"Fire Interval-",
			0.05f
		},
		{
			NONE_HEALTH_UPGRADE,NONE_SHIELD_UPGRADE, BULLET_BRANCH_UPGRADE,
			"Fire Branch+",
			1.f
		},
		{
			NONE_HEALTH_UPGRADE,NONE_SHIELD_UPGRADE, BULLET_EXPLOSION_RANGE_UPGRADE,
			"Bullet Explosion+",
			5.f
		},
		{
			NONE_HEALTH_UPGRADE,NONE_SHIELD_UPGRADE, BULLET_TRACK_UPGRADE,
			"Track Timer+",
			0.25f
		},
	};
	int                     m_upgradeChooseHealth           = 0;
	int                     m_upgradeChooseShield           = 0;
	int                     m_upgradeChooseBullet           = 0;
	float                   m_upgradeItemWidth              = 200.f;
	float                   m_upgradeItemHeight             = 400.f;
	int                     m_upgradeGridNum                = 16;
	float                   m_upgradeGridLineThickness      = 1.f;
	Rgba8                   m_upgradeGridLineColor          = Rgba8(0, 255, 0, 64);
					        								   
	bool                    m_attractModeStartFlag          = true;
	AttractModeBoidsEntity  m_attractModeBoids[MAX_ATTRACTMODE_BOIDS];
	AttractModeBoidsEntity  m_boidsNeighbors[MAX_ATTRACTMODE_BOIDS];
	AttractModeBoidsEntity  m_boidPlayerNeighbors[MAX_ATTRACTMODE_BOIDS];
	float                   m_boidPlayerSeprationWeight     = 10000.f;
	float                   m_boidsSeprationWeight          = 300.f;
	float                   m_boidsCohesionWeight           = 2.f;
	float                   m_boidsAlignmentWeight          = 0.8f;
	float                   m_boidBeetleMaxSpeed            = 100.f;
	float                   m_boidWaspMaxSpeed              = 300.f;
	float                   m_boidPlayerMaxSpeed            = 500.f;
	float                   m_boidCenterPullWeight          = 50.f;
	float                   m_boidPlayerPullWeight          = 50.f;
	float                   m_boidsAnimationTimeCount       = 0.f;
	float                   m_boidsAnimationTotalTime       = 0.2f;
	float                   m_boidsPlayerAnimationTimeCount = 0.f;
	float                   m_boidsPlayerAnimationTotalTime = 1.f;	             						   	   
	float                   m_titleAnimationTotalTime       = 2.f;
	float                   m_titleAnimationTimeCount       = 0.f;
	float                   m_titleScaleMax                 = 1.05f;
	float                   m_titleScaleMin                 = 0.95f;
	float                   m_titleUpDownMax                = 0.5f;
	Vertex*                 m_attractModeStarsMesh          = nullptr;
	Vertex*                 m_attractModeNearStarsMesh      = nullptr;
	Vertex*                 m_attractModeFarStarsMesh       = nullptr;
	float                   m_attractModeStarExpandDistance = 70.f;
						    
	float                   m_curCameraShakeAmp             = 0.f;
	float                   m_maxCameraShakeAmp             = 10.f;
	float                   m_cameraShakeDecaySpeed         = 5.f;
	float                   m_playModeSoundSpeed            = 1.f;
	float                   m_playModeSoundDecaySpeed       = 2.f;
						    
	bool                    m_isRemainEnemy                 = true;
	float                   m_delayQuitWaitedTime           = 0;
	float                   m_delayQuitDuration             = 1.f;
	bool                    m_delayQuitFlag                 = false;
};