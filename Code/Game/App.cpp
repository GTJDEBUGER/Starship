#include "Engine/Core/Engine.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine//Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/BeetleEnemy.hpp"
#include "Game/WaspEnemy.hpp"

App* g_app = nullptr;

//-----------------------------------------------------------------------------------------------
App::App()
{
	g_engine = new Engine();
	m_game = new Game();
	m_gameCamera = new Camera(Vec2(0.f,0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
	m_UICamera = new Camera(Vec2(0.f, 0.f), Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));
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
	g_engine->BeginFrame();	
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
	if (m_singleStep) {
		m_singleStep = false;
		deltaSeconds = originalDeltaSeconds;

		m_isPause = true;
	}

	//-------------------------------------------------------------------------------------------
	if (!m_isAttractMode) {
		m_game->Update(deltaSeconds);
	}
	else {
		m_startButtonAnimationTimeCount += deltaSeconds;
		if (m_startButtonAnimationTimeCount > m_startButtonAnimationTotalTime) {
			m_startButtonAnimationTimeCount = 0;
		}
	}

	//-------------------------------------------------------------------------------------------
	m_lastFrameTime = static_cast<float>(GetCurrentTimeSeconds());
}

//-----------------------------------------------------------------------------------------------
void App::Render() {
	if (!m_isAttractMode) {
		m_game->Render();
	}
	else {
		RenderAttractMode();
	}
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
		m_singleStep = true;
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_SPACE)) {
		if (!m_isAttractMode) {
			m_isFiring = true;
		}
		else {
			m_isAttractMode = false;
			delete m_game;
			m_game = nullptr;
			m_game = new Game();
		}
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_F1)) {
		m_isDebugDraw = !m_isDebugDraw;
	}

	if (g_engine->m_input->WasKeyJustPressed('N')) {
		if (!m_isAttractMode) {
			m_isPlayerRespawn = true;
		}
		else {
			m_isAttractMode = false;
			delete m_game;
			m_game = nullptr;
			m_game = new Game();
		}
	}

	if (g_engine->m_input->WasKeyJustPressed('I')) {
		m_isAsteroidRespawn = true;
	}

	if (g_engine->m_input->WasKeyJustPressed(KEYCODE_ESC)) {
		if (m_isAttractMode) {
			m_isQuitting = true;
		}
		else {
			m_isAttractMode = true;
		}
	}

	if (g_engine->m_input->WasKeyJustPressed('E') || g_engine->m_input->IsKeyDown('E')) {
		m_game->m_player->m_acceleration = PLAYER_SHIP_ACCELERATION;
	}

	if ((g_engine->m_input->WasKeyJustPressed('F') || g_engine->m_input->IsKeyDown('F'))
		&& !(g_engine->m_input->WasKeyJustPressed('S') || g_engine->m_input->IsKeyDown('S'))) {
		m_game->m_player->m_rotationSpeed = -PLAYER_SHIP_TURN_SPEED;
	}

	if ((g_engine->m_input->WasKeyJustPressed('S') || g_engine->m_input->IsKeyDown('S'))
		&& !(g_engine->m_input->WasKeyJustPressed('F') || g_engine->m_input->IsKeyDown('F'))) {
		m_game->m_player->m_rotationSpeed = PLAYER_SHIP_TURN_SPEED;
	}


	if (g_engine->m_input->WasKeyJustReleased('T')) {
		m_isSlowDown = false;
	}

	if (g_engine->m_input->WasKeyJustReleased('E')) {
		m_game->m_player->m_acceleration = 0;
	}

	if ((g_engine->m_input->WasKeyJustReleased('S') || g_engine->m_input->WasKeyJustReleased('F')) 
		|| (g_engine->m_input->IsKeyDown('F') && g_engine->m_input->IsKeyDown('S'))) {
		m_game->m_player->m_rotationSpeed = 0;
	}

	//Controller control
	//-------------------------------------------------------------------------------------------
	if (g_engine->m_input->GetController(0).GetLeftStick().GetMagnitude()>0) {
		m_game->m_player->m_orientationDegrees = g_engine->m_input->GetController(0).GetLeftStick().GetOrientationDegrees();
		m_game->m_player->m_acceleration = PLAYER_SHIP_ACCELERATION * g_engine->m_input->GetController(0).GetLeftStick().GetMagnitude();
	}
	else {
		m_game->m_player->m_acceleration = 0;
	}

	if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::GAMEPAD_A)) {
		if (!m_isAttractMode) {
			m_isFiring = true;
		}
		else {
			m_isAttractMode = false;
			delete m_game;
			m_game = nullptr;
			m_game = new Game();
		}
	}

	if (g_engine->m_input->GetController(0).WasButtonJustPressed(XboxButtonID::GAMEPAD_START)) {
		if (!m_isAttractMode) {
			m_isPlayerRespawn = true;
		}
		else {
			m_isAttractMode = false;
			delete m_game;
			m_game = nullptr;
			m_game = new Game();
		}
	}
}

//-----------------------------------------------------------------------------------------------
void App::RenderAttractMode() const {
	g_engine->m_renderer->BeginCamera(*g_app->m_UICamera);

	Vertex worldMesh[WaspEnemy::m_vertexNum];

	PlayerShip::GetLocalMesh(PlayerShip::m_vertexNum, worldMesh);
	TransformVertexArrayXY3D(PlayerShip::m_vertexNum, worldMesh, 5.f, 0.f, Vec2(WORLD_CENTER_X - 20*PLAYER_SHIP_COSMETIC_RADIUS , WORLD_CENTER_Y));
	g_engine->m_renderer->DrawVertexArray(15, worldMesh);

	BeetleEnemy::GetLocalMesh(BeetleEnemy::m_vertexNum, worldMesh);
	TransformVertexArrayXY3D(BeetleEnemy::m_vertexNum-6, worldMesh, 5.f, 200.f, 
		Vec2(WORLD_CENTER_X + 20*PLAYER_SHIP_COSMETIC_RADIUS, WORLD_CENTER_Y + 10 * PLAYER_SHIP_COSMETIC_RADIUS));
	g_engine->m_renderer->DrawVertexArray(BeetleEnemy::m_vertexNum - 6, worldMesh);

	WaspEnemy::GetLocalMesh(WaspEnemy::m_vertexNum, worldMesh);
	TransformVertexArrayXY3D(WaspEnemy::m_vertexNum - 12, worldMesh, 5.f, 160.f,
		Vec2(WORLD_CENTER_X + 20 * PLAYER_SHIP_COSMETIC_RADIUS, WORLD_CENTER_Y - 10 * PLAYER_SHIP_COSMETIC_RADIUS));
	g_engine->m_renderer->DrawVertexArray(WaspEnemy::m_vertexNum - 12, worldMesh);

	Vertex startButtonMesh[3];
	float fraction = m_startButtonAnimationTimeCount / m_startButtonAnimationTotalTime;
	if (fraction <= 0.5f) {
		startButtonMesh[0] = Vertex(Vec3(-1.f, 1.7f, 0.f), 
									Rgba8(0, static_cast<unsigned char>(Interpolate(255.f, 64.f, fraction * 2)), 0, 255), 
									Vec2(0, 0));
		startButtonMesh[1] = Vertex(Vec3(-1.f, -1.7f, 0.f),
									Rgba8(0, static_cast<unsigned char>(Interpolate(255.f, 64.f, fraction * 2)), 0, 255),
									Vec2(0, 0)); 
		startButtonMesh[2] = Vertex(Vec3(2.f, 0.f, 0.f),
									Rgba8(0, static_cast<unsigned char>(Interpolate(255.f, 64.f, fraction * 2)), 0, 255),
									Vec2(0, 0));
	}
	else
	{
		startButtonMesh[0] = Vertex(Vec3(-1.f, 1.7f, 0.f),
									Rgba8(0, static_cast<unsigned char>(Interpolate(64.f, 255.f, (fraction-0.5f) * 2)), 0, 255),
									Vec2(0, 0)); 
		startButtonMesh[1] = Vertex(Vec3(-1.f, -1.7f, 0.f),
									Rgba8(0, static_cast<unsigned char>(Interpolate(64.f, 255.f, (fraction - 0.5f) * 2)), 0, 255),
									Vec2(0, 0));
		startButtonMesh[2] = Vertex(Vec3(2.f, 0.f, 0.f),
									Rgba8(0, static_cast<unsigned char>(Interpolate(64.f, 255.f, (fraction - 0.5f) * 2)), 0, 255),
									Vec2(0, 0));
	}
	TransformVertexArrayXY3D(3, startButtonMesh, 5.f, 0.f, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));
	g_engine->m_renderer->DrawVertexArray(3, startButtonMesh);

	g_engine->m_renderer->EndCamera(*g_app->m_UICamera);
}