#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/BeetleEnemy.hpp"
#include "Game/WaspEnemy.hpp"

#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine//Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"

App* g_app = nullptr;

//-----------------------------------------------------------------------------------------------
App::App()
{
	EngineConfig config;
	config.m_windowConfig.m_clientAspect = 2.f;
	config.m_windowConfig.m_windowTitle = "Starship Gold";
	new Engine(config);

	m_game = new Game();

	m_lastFrameTime = GetCurrentTimeSeconds();

	g_engine->m_audio->CreateOrGetSound("Data/Audio/ShootBullet.mp3");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/DieExplode.wav");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/HitWall.wav");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/Respawn.wav");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/EnemyGetHurt.wav");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/EnemyDie.wav");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/WaveStart.mp3");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/PlayerLevelUp.mp3");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/AsteroidBreak.mp3");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/ShockWaveSpawn.mp3");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/ChooseUpgrade.mp3");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/ConfirmUpgrade.mp3");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/GameWin.mp3");
	g_engine->m_audio->CreateOrGetSound("Data/Audio/GameLose.mp3");

	SoundID attractSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/AttractModeBGM.mp3");
	m_attractSoundPlaybackID = g_engine->m_audio->StartSound(attractSound, true, 1.0f, 0.f, 0.f);

	SoundID gameSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/GameModeBGM.mp3");
	m_gameSoundPlaybackID = g_engine->m_audio->StartSound(gameSound, true, 1.0f, 0.f, 0.f);

	SoundID accelerateSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/Acceleration.wav");
	m_accelerateSoundPlaybackID = g_engine->m_audio->StartSound(accelerateSound, true, 1.0f, 0.f, 0.f);
}

//-----------------------------------------------------------------------------------------------
App::~App()
{
	delete m_game;
	m_game = nullptr;
	delete g_engine;
	g_engine = nullptr;
}

//-----------------------------------------------------------------------------------------------
void App::RunMainLoop() {
	while (!IsQuitting())
	{
		RunFrame();
	}
}

//-----------------------------------------------------------------------------------------------
void App::RunFrame()
{
	//-------------------------------------------------------------------------------------------
	g_engine->BeginFrame();		
		HandlePlayerInput();

		if (m_isShutdown) {
			m_isShutdown = false;
			return;
		}

		Update();
		Render();
	g_engine->EndFrame();

}

//-----------------------------------------------------------------------------------------------
void App::Update() {
	//-------------------------------------------------------------------------------------------
	float deltaSeconds = static_cast<float>(GetCurrentTimeSeconds()) - m_lastFrameTime;

	float originalDeltaSeconds = deltaSeconds;
	if (m_isSlowDown) {
		deltaSeconds *= 0.1f;
		originalDeltaSeconds = deltaSeconds;
	}
	if (m_isPause) {
		deltaSeconds = 0.f;
	}
	if (m_isRunSingleStep) {
		m_isRunSingleStep = false;
		deltaSeconds = originalDeltaSeconds;

		m_isPause = true;
	}

	//-------------------------------------------------------------------------------------------
	m_game->Update(deltaSeconds);

	//-------------------------------------------------------------------------------------------
	m_lastFrameTime = static_cast<float>(GetCurrentTimeSeconds());
}

//-----------------------------------------------------------------------------------------------
void App::Render() {
	m_game->Render();
}

//-----------------------------------------------------------------------------------------------
bool App::IsQuitting() {
	return m_isQuitting;
}

//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	delete g_app;
	g_app = new App();
}

//-----------------------------------------------------------------------------------------------
void App::HandlePlayerInput(){

	//Keyboard control
	//-------------------------------------------------------------------------------------------
	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F8)) {
		Shutdown();
		m_isShutdown = true;
		return;
	}

	if (g_engine->m_input->IsKeyDown('T') || g_engine->m_input->WasKeyJustPressed('T')) {
		m_isSlowDown = true;
	}

	if (g_engine->m_input->WasKeyJustPressed('P')) {
		m_isPause = !m_isPause;
	}

	if (g_engine->m_input->WasKeyJustPressed('O')) {
		m_isRunSingleStep = true;
	}

	if (g_engine->m_input->IsKeyDown(KEYCODE_SPACE)) {
		if (m_game->GetCurGameState() == GAME_PLAYING_MODE) {
			m_isFiring = true;
		}
		else if(m_game->GetCurGameState() == GAME_ATTRACT_MODE){
			delete m_game;
			m_game = new Game();
			m_game->SetNextGameState(GAME_PLAYING_MODE);
		}
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_ENTER)) {
		if (m_game->GetCurGameState() == GAME_PLAYER_UPGRADE_MODE) {
			SoundID confirmSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/ConfirmUpgrade.mp3");
			g_engine->m_audio->StartSound(confirmSound, false, 2.f, 0.f, 1.f);

			if (m_game->m_player->m_upgradeTimes > 0) {
				m_game->m_player->GainUpgrade(m_game->GetChoseUpgrade());
				m_game->SetNextGameState(GAME_PLAYER_UPGRADE_MODE);
			}
			else {
				m_game->SetNextGameState(GAME_PLAYING_MODE);
			}
		}
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F1)) {
		m_isDebugDraw = !m_isDebugDraw;
	}

	if (g_engine->m_input->WasKeyJustPressed('N')) {
		if (m_game->GetCurGameState() != GAME_ATTRACT_MODE) {
			m_isPlayerRespawn = true;
		}
		else {
			delete m_game;
			m_game = new Game();
			m_game->SetNextGameState(GAME_PLAYING_MODE);
		}
	}

	if (g_engine->m_input->WasKeyJustPressed('I')) {
		m_isAsteroidRespawn = true;
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_ESC)) {
		if (m_game->GetCurGameState() == GAME_ATTRACT_MODE) {
			m_isQuitting = true;
		}
		else {
			m_game->SetNextGameState(GAME_ATTRACT_MODE);
		}
	}

	if (g_engine->m_input->WasKeyJustPressed('E') || g_engine->m_input->IsKeyDown('E')) {
		m_game->m_player->m_acceleration = PLAYER_SHIP_ACCELERATION;
	}

	if ((g_engine->m_input->WasKeyJustPressed('F') || g_engine->m_input->IsKeyDown('F'))
		&& !(g_engine->m_input->WasKeyJustPressed('S') || g_engine->m_input->IsKeyDown('S'))) {
		if (m_game->GetCurGameState() == GAME_PLAYING_MODE) {
			m_game->m_player->m_rotationSpeed = -PLAYER_SHIP_TURN_SPEED;
		}
		else if (m_game->GetCurGameState() == GAME_PLAYER_UPGRADE_MODE && !g_engine->m_input->IsKeyDown('F')) {
			m_game->UpdateToNextUpgradeChoose();
		}
	}

	if ((g_engine->m_input->WasKeyJustPressed('S') || g_engine->m_input->IsKeyDown('S'))
		&& !(g_engine->m_input->WasKeyJustPressed('F') || g_engine->m_input->IsKeyDown('F'))) {
		if (m_game->GetCurGameState() == GAME_PLAYING_MODE) {
			m_game->m_player->m_rotationSpeed = PLAYER_SHIP_TURN_SPEED;
		}
		else if (m_game->GetCurGameState() == GAME_PLAYER_UPGRADE_MODE && !g_engine->m_input->IsKeyDown('S')) {
			m_game->UpdateToPreviousUpgradeChoose();
		}
	}

	if (g_engine->m_input->WasKeyJustPressed('K')) {
		m_game->KillAllEntities();
	}

	if (g_engine->m_input->WasKeyJustReleased('T')) {
		m_isSlowDown = false;
	}

	if (g_engine->m_input->WasKeyJustReleased('E') || !g_engine->m_input->IsKeyDown('E')) {
		m_game->m_player->m_acceleration = 0;
	}

	if ((g_engine->m_input->WasKeyJustReleased('S') || g_engine->m_input->WasKeyJustReleased('F')) 
		|| (g_engine->m_input->IsKeyDown('F') && g_engine->m_input->IsKeyDown('S'))) {
		m_game->m_player->m_rotationSpeed = 0;
	}

	//Controller control
	//-------------------------------------------------------------------------------------------
	if (g_engine->m_input->GetController(0).GetLeftStick().GetMagnitude()>0) {
		if (m_game->GetCurGameState() == GAME_PLAYING_MODE) {
			m_game->m_player->m_orientationDegrees = g_engine->m_input->GetController(0).GetLeftStick().GetOrientationDegrees();
			m_game->m_player->m_acceleration = PLAYER_SHIP_ACCELERATION * g_engine->m_input->GetController(0).GetLeftStick().GetMagnitude();
		}
	}

	if (g_engine->m_input->GetController(0).IsButtonDown(XboxButtonID::GAMEPAD_A)) {
		if (m_game->GetCurGameState() == GAME_PLAYING_MODE) {
			m_isFiring = true;
		}
		else if(m_game->GetCurGameState() == GAME_ATTRACT_MODE){
			delete m_game;
			m_game = new Game();
			m_game->SetNextGameState(GAME_PLAYING_MODE);
		}
	}

	if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::GAMEPAD_START)) {
		if (m_game->GetCurGameState() == GAME_PLAYING_MODE) {
			m_isPlayerRespawn = true;
		}
		else if (m_game->GetCurGameState() == GAME_ATTRACT_MODE) {
			delete m_game;
			m_game = new Game();
			m_game->SetNextGameState(GAME_PLAYING_MODE);
		}
	}

	if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::GAMEPAD_DPAD_RIGHT)) {
		if (m_game->GetCurGameState() == GAME_PLAYER_UPGRADE_MODE) {
			m_game->UpdateToNextUpgradeChoose();
		}
	}

	if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::GAMEPAD_DPAD_LEFT)) {
		if (m_game->GetCurGameState() == GAME_PLAYER_UPGRADE_MODE) {
			m_game->UpdateToPreviousUpgradeChoose();
		}
	}

	if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::GAMEPAD_X)) {
		if (m_game->GetCurGameState() == GAME_PLAYER_UPGRADE_MODE) {
			SoundID confirmSound = g_engine->m_audio->CreateOrGetSound("Data/Audio/ConfirmUpgrade.mp3");
			g_engine->m_audio->StartSound(confirmSound, false, 2.f, 0.f, 1.f);

			if (m_game->m_player->m_upgradeTimes > 0) {
				m_game->m_player->GainUpgrade(m_game->GetChoseUpgrade());
				m_game->SetNextGameState(GAME_PLAYER_UPGRADE_MODE);
			}
			else {
				m_game->SetNextGameState(GAME_PLAYING_MODE);
			}
		}
	}
}

