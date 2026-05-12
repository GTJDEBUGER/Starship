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
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"

App* g_app = nullptr;

//-----------------------------------------------------------------------------------------------
App::App()
{
	m_screenCamera = new Camera(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));

	EngineConfig config;
	config.m_windowConfig.m_clientAspect = 2.f;
	config.m_windowConfig.m_windowTitle = "Starship Gold";
	config.m_devConsoleConfig.m_camera = m_screenCamera;
	new Engine(config);

	m_screenCamera->SetViewPort(
		AABB2(
			Vec2(0.f, 0.f),
			Vec2((float)g_engine->m_window->GetClientDimensions().x, (float)g_engine->m_window->GetClientDimensions().y)
		)
	);

	m_game = new Game();

	g_engine->m_eventSystem->SubscribeEventCallbackFunction("quit", &HandleQuit, "Close whole game application.", true);
	g_engine->m_eventSystem->SubscribeEventCallbackFunction("setTimeScale", &HandleSetTimeScale, "Set game running at a new time scale. (Parameter required: scale=?)", true);
	g_engine->m_eventSystem->FireEvent("help");

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

	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "Starship GOLD");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "Input Tutorial:");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-F1 Draw debug rings and lines");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-F8 Force restart whole game");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-T Slow down whole game");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-O Run one step of the game");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-P Pause whole game");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-I Generate new asteroids");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-SPACEE Fire bullets or start game at attract mode");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-N Respawn player");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-E Accelerate players spaceship");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-S Rotate players spaceship counterclockwise or select left upgrade at player upgrade board");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-F Rotate players spaceship clockwise or select right upgrade at player upgrade board");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-ENTER Confirm upgrade at player upgrade board");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-ESC Switch from game mode to attract mode or shut down application in attract mode");
	g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, "\t-K Cheat button for kill all entities in game");
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
	Clock::TickSystemClock();
	m_game->m_gameClock->SetTimeScale(m_gameTimeScale);
	//-------------------------------------------------------------------------------------------
	m_game->Update();
}

//-----------------------------------------------------------------------------------------------
void App::Render() {
	m_game->Render();

	g_engine->m_devConsole->Render(AABB2(Vec2(0,0), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y)));
}

//-----------------------------------------------------------------------------------------------
bool App::IsQuitting() {
	return m_isQuitting;
}

//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	delete m_game;
	m_game = new Game();
}

//-----------------------------------------------------------------------------------------------
void App::HandlePlayerInput(){

	//Keyboard control
	//-------------------------------------------------------------------------------------------
	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_TILDE)) {
		g_engine->m_devConsole->ToggleOpen();
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F8)) {
		Shutdown();
		m_isShutdown = true;
		return;
	}

	if (g_engine->m_input->IsKeyDown('T') || g_engine->m_input->WasKeyJustPressed('T')) {
		m_game->m_gameClock->SetTimeScale(m_gameTimeScale * 0.1f);
	}

	if (g_engine->m_input->WasKeyJustPressed('P')) {
		m_game->m_gameClock->TogglePause();
	}

	if (g_engine->m_input->WasKeyJustPressed('O')) {
		m_game->m_gameClock->StepSingleFrame();
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
		m_game->m_gameClock->SetTimeScale(m_gameTimeScale);
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

//-----------------------------------------------------------------------------------------------
bool App::HandleQuit([[maybe_unused]] EventArgs& args) {
	g_app->m_isQuitting = true;

	return true;
}

//-----------------------------------------------------------------------------------------------
bool App::HandleSetTimeScale([[maybe_unused]] EventArgs& args) {
	float timeScale = args.GetValue("scale", -1.f);
	if (timeScale >= 0.f) {
		g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_MESSAGE, Stringf("Set game running at new time scale. (scale = %f)", timeScale));
		g_app->m_gameTimeScale = timeScale;
	}
	else {
		g_engine->m_devConsole->AddLine(DevConsoleLineType::INFO_ERROR, Stringf("Invalid time scale! (scale = %f)", timeScale));
	}
	return true;
}