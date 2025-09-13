#include "Engine/Core/Engine.hpp"
#include "Engine//Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerShip.hpp"

App* g_app = nullptr;

//-----------------------------------------------------------------------------------------------
App::App()
{
	g_engine = new Engine();
	m_game = new Game();
	m_camera = new Camera(Vec2(0.f,0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
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
void App::RunFrame()
{
	//-------------------------------------------------------------------------------------------
	HandlePlayerInput();
	if (m_isShutdown) {
		m_isShutdown = false;
		return;
	}

	//-------------------------------------------------------------------------------------------
	float deltaSeconds = 1.f / 60.f;

	float originalDeltaSeconds = deltaSeconds;
	if (m_isSlowDown) {
		deltaSeconds *= 0.1f;
		originalDeltaSeconds = deltaSeconds;
	}
	if (m_isPause) {
		deltaSeconds = 0.f;
	}
	if (m_singleStep) {
		m_singleStep = false;
		deltaSeconds = originalDeltaSeconds;

		m_isPause = true;
	}

	//-------------------------------------------------------------------------------------------
	g_engine->BeginFrame();
		g_engine->m_renderer->BeginCamera(*m_camera);
			m_game->Update(deltaSeconds);
			m_game->Render();
		g_engine->m_renderer->EndCamera(*m_camera);
	g_engine->EndFrame();

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
void App::OnKeyDown(unsigned char keyCode)
{
	m_keyDownThisFrame[keyCode] = true;
}

//-----------------------------------------------------------------------------------------------
void App::OnKeyUp(unsigned char keyCode)
{
	m_keyDownThisFrame[keyCode] = false;
}

//-----------------------------------------------------------------------------------------------
void App::HandlePlayerInput(){
	if (IsKeyDownThisFrame(119)) { //F8
		Shutdown();
		m_isShutdown = true;
		return;
	}

	if (IsKeyHeldThisFrame('T') || IsKeyDownThisFrame('T')) {
		m_isSlowDown = true;
	}

	if (IsKeyDownThisFrame('P')) {
		m_isPause = !m_isPause;
	}

	if (IsKeyDownThisFrame('O')) {
		m_singleStep = true;
	}

	if (IsKeyDownThisFrame(32)) {
		m_isFiring = true;
	}

	if (IsKeyDownThisFrame(112)) {
		m_isDebugDraw = !m_isDebugDraw;
	}

	if (IsKeyDownThisFrame('N')) {
		m_isPlayerRespawn = true;
	}

	if (IsKeyDownThisFrame('I')) {
		m_isAsteroidRespawn = true;
	}

	if (IsKeyDownThisFrame('Q')) {
		m_isQuitting = true;
	}

	if (IsKeyHeldThisFrame('E') || IsKeyDownThisFrame('E')) {
		m_game->m_player->m_acceleration = PLAYER_SHIP_ACCELERATION;
	}

	if ((IsKeyHeldThisFrame('F') || IsKeyDownThisFrame('F')) && !(IsKeyHeldThisFrame('S') || IsKeyDownThisFrame('S'))) {
		m_game->m_player->m_rotationSpeed = -PLAYER_SHIP_TURN_SPEED;
	}

	if ((IsKeyHeldThisFrame('S') || IsKeyDownThisFrame('S')) && !(IsKeyHeldThisFrame('F') || IsKeyDownThisFrame('F'))) {
		m_game->m_player->m_rotationSpeed = PLAYER_SHIP_TURN_SPEED;
	}


	if (IsKeyUpThisFrame('T')) {
		m_isSlowDown = false;
	}

	if (IsKeyUpThisFrame('E')) {
		m_game->m_player->m_acceleration = 0;
	}

	if ((IsKeyUpThisFrame('S') || IsKeyUpThisFrame('F')) || (IsKeyHeldThisFrame('F') && IsKeyHeldThisFrame('S'))) {
		m_game->m_player->m_rotationSpeed = 0;
	}

	//------------------------------------------------------------------------------------------
	for (int i = 0; i < 256; i++) {
		m_keyDownLastFrame[i] = m_keyDownThisFrame[i];
	}
}

//-----------------------------------------------------------------------------------------------
bool App::IsKeyDownThisFrame(unsigned char keyCode) {
	return m_keyDownThisFrame[keyCode] && !m_keyDownLastFrame[keyCode];
}

//-----------------------------------------------------------------------------------------------
bool App::IsKeyHeldThisFrame(unsigned char keyCode) {
	return m_keyDownThisFrame[keyCode] && m_keyDownLastFrame[keyCode];
}

//-----------------------------------------------------------------------------------------------
bool App::IsKeyUpThisFrame(unsigned char keyCode) {
	return !m_keyDownThisFrame[keyCode] && m_keyDownLastFrame[keyCode];
}