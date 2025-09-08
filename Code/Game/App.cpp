#include "App.hpp"
#include "Game.hpp"
#include "GameCommon.hpp"
#include "PlayerShip.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine//Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"

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
	if (m_keyDownThisFrame['T']) {
		m_isSlowDown = true;
	}

	if (m_keyDownThisFrame['P'] && pauseTrigger) {
		m_isPause = !m_isPause;
		pauseTrigger = false;
	}

	if (m_keyDownThisFrame[32] && firingTrigger) {
		m_isFiring = true;
		firingTrigger = false;
	}

	if (m_keyDownThisFrame[112] && debugDrawTrigger) {
		m_isDebugDraw = !m_isDebugDraw;
		debugDrawTrigger = false;
	}

	if (m_keyDownThisFrame['O']) {
		m_singleStep = true;
	}

	if (m_keyDownThisFrame['Q']) {
		m_isQuitting = true;
	}

	if (m_keyDownThisFrame['E'] && !m_keyUpThisFrame['E']) {
		m_game->m_player->m_acceleration = PLAYER_SHIP_ACCELERATION;
	}

	if (m_keyDownThisFrame['S'] && m_keyDownThisFrame['F']) {
		m_game->m_player->m_rotationSpeed = 0;
	}
	else if (m_keyDownThisFrame['F']) {
		m_game->m_player->m_rotationSpeed = -PLAYER_SHIP_TURN_SPEED;
	}
	else if (m_keyDownThisFrame['S']) {
		m_game->m_player->m_rotationSpeed = PLAYER_SHIP_TURN_SPEED;
	}


	if (m_keyUpThisFrame['T']) {
		m_isSlowDown = false;
	}

	if (m_keyUpThisFrame['P'] && !pauseTrigger) {
		pauseTrigger = true;
	}

	if (m_keyUpThisFrame[32] && !firingTrigger && !m_isFiring) {
		firingTrigger = true;
	}

	if (m_keyUpThisFrame[112] && !debugDrawTrigger) {
		debugDrawTrigger = true;
	}

	if (m_keyUpThisFrame['E']) {
		m_game->m_player->m_acceleration = 0;
	}

	if (m_keyUpThisFrame['S'] || m_keyUpThisFrame['F']) {
		m_game->m_player->m_rotationSpeed = 0;
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
		pauseTrigger = true;
	}

	//-------------------------------------------------------------------------------------------
	g_engine->BeginFrame();
		g_engine->m_renderer->BeginCamera(*m_camera);
			m_game->Update(deltaSeconds);
			m_game->Render();
		g_engine->m_renderer->EndCamera(*m_camera);
	g_engine->EndFrame();

	//-------------------------------------------------------------------------------------------
	for (int i = 0; i < 256; i++) {
		m_keyDownThisFrame[i] = false;
		m_keyUpThisFrame[i] = false;
	}
}

//-----------------------------------------------------------------------------------------------
bool App::IsQuitting() {
	return m_isQuitting;
}

//-----------------------------------------------------------------------------------------------
void App::OnKeyDown(unsigned char keyCode)
{
	m_keyDownThisFrame[keyCode] = true;
}

//-----------------------------------------------------------------------------------------------
void App::OnKeyUp(unsigned char keyCode)
{
	m_keyUpThisFrame[keyCode] = true;
}