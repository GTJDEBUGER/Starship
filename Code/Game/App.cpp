#include "App.hpp"
#include "Game.hpp"
#include "GameCommon.hpp"
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
	float deltaSeconds = 1.f / 60.f;

	// Handle user input
	float originalDeltaSeconds = deltaSeconds;
	if (g_isSlowDown) {
		deltaSeconds *= 0.1f;
		originalDeltaSeconds = deltaSeconds;
	}
	if (g_isPause) {
		deltaSeconds = 0.f;
	}
	if (g_singleStep) {
		g_singleStep = false;
		deltaSeconds = originalDeltaSeconds;

		g_isPause = true;
		pauseTrigger = true;
	}

	g_engine->BeginFrame();
		g_engine->m_renderer->BeginCamera(*m_camera);
			m_game->Update(deltaSeconds);
			m_game->Render();
		g_engine->m_renderer->EndCamera(*m_camera);
	g_engine->EndFrame();

	for (int i = 0; i < 256; i++) {
		m_keyDownLastFrame[i] = m_keyDownThisFrame[i];
	}
}

//-----------------------------------------------------------------------------------------------
bool App::IsQuitting() {
	return g_isQuitting;
}

//-----------------------------------------------------------------------------------------------
bool App::IsKeyDown(unsigned char keyCode) const
{
	return m_keyDownThisFrame[keyCode];
}

//-----------------------------------------------------------------------------------------------
bool App::WasKeyJustPressed(unsigned char keyCode) const
{
	return m_keyDownThisFrame[keyCode] && !m_keyDownLastFrame[keyCode];
}